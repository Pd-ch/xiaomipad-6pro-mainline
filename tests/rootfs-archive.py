#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Run with unshare -Ur python3 tests/rootfs-archive.py (no device access)."""
import os
from pathlib import Path
import subprocess
import tempfile

helper = Path(__file__).resolve().parents[1] / "tools/lib/rootfs-archive.sh"
with tempfile.TemporaryDirectory(prefix="liuqin-archive-test-") as directory:
    work = Path(directory)
    source, target, archive = work / "source", work / "target", work / "root.tar.gz"
    source.mkdir()
    program = source / "program"
    program.write_bytes(b"archive fixture\n")
    program.chmod(0o751)
    os.setxattr(program, "user.archive-test", b"preserved")
    subprocess.run(["setcap", "cap_net_bind_service=ep", str(program)], check=True)
    (source / "link").symlink_to("program")
    os.link(program, source / "hardlink")
    for action, root in (("pack", source), ("extract", target)):
        subprocess.run(["sh", str(helper), action, str(root), str(archive)], check=True)
    restored = target / "program"
    assert restored.read_bytes() == program.read_bytes()
    assert restored.stat().st_mode == program.stat().st_mode
    assert (restored.stat().st_uid, restored.stat().st_gid) == (program.stat().st_uid, program.stat().st_gid)
    for name in ("user.archive-test", "security.capability"):
        assert os.getxattr(restored, name) == os.getxattr(program, name)
    assert (target / "link").readlink() == Path("program")
    assert restored.stat().st_ino == (target / "hardlink").stat().st_ino
    result = subprocess.run(["sh", str(helper), "extract", str(target), str(archive)],
                            capture_output=True)
    assert result.returncode != 0
print("PASS: content, ownership, modes, capabilities, user xattrs, links and existing-target refusal")
