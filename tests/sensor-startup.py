#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Check SLPI lifecycle policy without accessing a real remote processor."""
import configparser
from pathlib import Path
import subprocess
import tempfile

project = Path(__file__).resolve().parents[1]
overlay = project / 'device/sensors-overlay'
helper = overlay / 'usr/local/sbin/liuqin-slpi'
subprocess.run(['sh', '-n', str(helper)], check=True)
unit = configparser.ConfigParser(interpolation=None, strict=False)
unit.read(overlay / 'etc/systemd/system/liuqin-hexagonrpcd-sdsp.service')
assert 'liuqin-slpi.service' in unit['Unit']['Requires'].split()
assert 'liuqin-slpi.service' in unit['Unit']['After'].split()
assert 'qcom_q6v5_pas.slpi_auto_boot=0' in (
    project / 'device/native-bootargs.txt').read_text().split()

with tempfile.TemporaryDirectory() as directory:
    root = Path(directory)
    remote = root / 'remoteproc/remoteproc7'
    remote.mkdir(parents=True)
    (remote / 'name').write_text('slpi\n')
    registry = root / 'sensors/registry'
    registry.mkdir(parents=True)
    manifest = registry / 'SHA256SUMS'
    manifest.write_text('fixture\n')
    binaries = root / 'bin'
    binaries.mkdir()
    # Identity switching is separate from the lifecycle state-machine test.
    runuser = binaries / 'runuser'
    runuser.write_text('#!/bin/sh\n[ "$1 $2 $3" = "-u fastrpc --" ] || exit 2\n'
                       'shift 3\nexec "$@"\n')
    runuser.chmod(0o755)
    sandbox = ['bwrap', '--unshare-user', '--unshare-pid', '--unshare-net',
               '--ro-bind', '/', '/', '--tmpfs', '/sys', '--dir', '/sys/class',
               '--bind', str(root / 'remoteproc'), '/sys/class/remoteproc',
               '--tmpfs', '/var', '--dir', '/var/lib',
               '--bind', str(root / 'sensors'), '/var/lib/liuqin-sensors',
               '--tmpfs', '/tmp', '--ro-bind', str(binaries), '/tmp/bin',
               '--ro-bind', str(helper), '/tmp/liuqin-slpi',
               '--setenv', 'PATH', '/tmp/bin:/usr/bin:/bin']

    def invoke(action, state, expected, success=True):
        (remote / 'state').write_text(state + '\n')
        result = subprocess.run(sandbox + ['sh', '/tmp/liuqin-slpi', action],
                                capture_output=True, text=True)
        assert (result.returncode == 0) == success, result.stderr
        assert (remote / 'state').read_text().strip() == expected

    invoke('start', 'offline', 'start')
    invoke('start', 'running', 'running')
    invoke('start', 'crashed', 'crashed', success=False)
    manifest.unlink()
    invoke('start', 'offline', 'offline', success=False)
    invoke('stop', 'running', 'stop')
    invoke('stop', 'offline', 'offline')
    invoke('invalid', 'offline', 'offline', success=False)

print('SLPI lifecycle policy checks passed; hardware validation is separate.')
