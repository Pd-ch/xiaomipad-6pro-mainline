# Installation Testing

The installer is under development. No supported installation release is
available yet. These steps are for an attended test with a recoverable tablet.

## Requirements

- Xiaomi Pad 6 Pro (liuqin), 256 GB layout. Other capacities are rejected.
- Unlocked bootloader, slot A active, and the device in Fastboot mode.
- Battery at least 30 percent charged.
- Linux host with Python 3.11 or newer, Android platform-tools and USB networking.
- Personal files backed up outside the tablet. Installation erases all userdata.
- A matching original Xiaomi Fastboot ROM and an Android recovery plan prepared
  before installation. The installer does not back up personal userdata.

Check the downloaded bundle first:

```sh
sha256sum -c SHA256SUMS
python3 install.py --bundle . --check
```

For an explicitly authorized, unverified candidate test:

```sh
python3 install.py --bundle . --serial DEVICE_SERIAL \
  --backup /path/to/new-private-backup --erase-userdata --allow-unverified
```

The installer boots `installer.img` in RAM, waits for its USB network, backs up
boot_a, boot_b and persist, verifies those backups, then installs the rootfs and
provisions this tablet's calibration and addresses. It formats userdata as ext4,
writes boot_a only after root installation succeeds, and reboots. It does not
switch slots, relock the bootloader, modify the partition table or write persist.
Keep the backup directory private. The USB rescue shell has no authentication:
use a direct, trusted USB connection, not a shared network.

USB networking normally obtains an address through DHCP. `--host-address` selects
the host's USB address when automatic route selection is unsuitable. If the
installer cannot establish its control channel it stops; do not blindly retry
after a partial installation. Preserve the error output and backup first.

## Recovery

Returning to Android erases the Ubuntu installation and requires a compatible
original Fastboot ROM, including its userdata initialization. Restoring boot_a
alone is not a complete Android recovery.

Use the original ROM's full clean-flash procedure, not its keep-data or relock
variant. Preserve the anti-rollback checks. Never restore another tablet's
persist or calibration. Keep the bootloader unlocked while non-stock images
remain. The original ROM is an upstream input, not duplicated in this repository.

The current installer and this recovery route still require end-to-end device
testing. A successful build or local checksum check does not establish recovery.
