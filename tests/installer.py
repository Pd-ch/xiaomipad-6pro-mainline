#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Offline tests: no Fastboot, tablet connection or block-device writes."""
import hashlib
import importlib.util
import json
from pathlib import Path
import re
import socket
import subprocess
import tempfile
import threading
from types import SimpleNamespace
from unittest.mock import patch

project = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('installer', project / 'tools/install-liuqin.py')
installer = importlib.util.module_from_spec(spec)
spec.loader.exec_module(installer)

with tempfile.TemporaryDirectory() as directory:
    root = Path(directory)
    files = {}
    for name in ('boot.img', 'installer.img', 'rootfs.tar.gz'):
        (root / name).write_bytes(name.encode())
        files[name] = hashlib.sha256(name.encode()).hexdigest()
    (root / 'bundle.json').write_text(json.dumps({'device': 'liuqin', 'files': files}))
    args = ['python3', str(project / 'tools/install-liuqin.py'), '--bundle', str(root), '--check']
    subprocess.run(args, check=True)
    subprocess.run(args + ['--enable-rescue'], check=True)
    (root / 'boot.img').write_bytes(b'corrupted')
    assert subprocess.run(args, capture_output=True).returncode != 0
    (root / 'boot.img').write_bytes(b'boot.img')
    (root / 'bundle.json').write_text(json.dumps({'device': 'liuqin', 'files': files,
                                               'status': 'OFFLINE_ASSEMBLED'}))
    for reported in ('0x100000', hex(471789528 * 512 + 512), 'unknown'):
        calls = []

        def fastboot(command, **kwargs):
            calls.append(command)
            assert command[:4] == ['fastboot', '-s', 'TEST_SERIAL', 'getvar']
            name = command[-1]
            values = {'product': 'liuqin', 'unlocked': 'yes', 'current-slot': 'a',
                      'partition-size:userdata': reported}
            return SimpleNamespace(stdout=name + ': ' + values[name] + '\n')

        with patch.object(installer.sys, 'argv', ['install.py', '--bundle', str(root),
                          '--serial', 'TEST_SERIAL', '--backup', str(root.parent / 'unused-backup'),
                          '--erase-userdata', '--allow-unverified']), \
             patch.object(installer.subprocess, 'run', side_effect=fastboot):
            try:
                installer.main()
            except (SystemExit, RuntimeError):
                pass
            else:
                raise AssertionError('unsupported or unknown userdata size was accepted')
        assert calls[-1][-1] == 'partition-size:userdata'
    print('PASS: wrong and unknown layouts rejected before RAM boot or partition writes')

# A local fake shell supplies a CRLF transcript containing the echoed command.
# Only complete marker lines may finish the transaction, not the echo itself.
listener = socket.socket()
listener.bind(('127.0.0.1', 0))
listener.listen(1)
address = listener.getsockname()

def shell():
    with listener.accept()[0] as connection:
        received = b''
        while not received.endswith(b'\n'):
            received += connection.recv(4096)
        token = re.search(rb'LIUQIN_[a-f0-9]+', received)[0]
        connection.sendall(received.replace(b'\n', b'\r\n'))
        connection.sendall(b'\r\n' + token + b'_START\r\nvalue\r\n' + token + b'_END 0\r\n')

thread = threading.Thread(target=shell)
thread.start()
connect = socket.create_connection
with patch.object(installer.socket, 'create_connection', side_effect=lambda *a, **k: connect(address, **k)):
    assert installer.command('unused', 'printf value') == b'value'
thread.join()
listener.close()
assert subprocess.run(['sh', str(project / 'tools/lib/install-root.sh')], capture_output=True).returncode != 0
invalid = subprocess.run(['sh', str(project / 'tools/lib/install-root.sh'),
                          'unused', 'unused', 'unused', 'unused',
                          'ERASE-LIUQIN-USERDATA', 'INVALID'], capture_output=True)
assert invalid.returncode != 0 and b'unsupported rescue option' in invalid.stderr
print('PASS: checksum rejection, local-only check, CRLF command framing and missing-authorization refusal')
