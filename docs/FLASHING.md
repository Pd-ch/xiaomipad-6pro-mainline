# Installation

[中文](FLASHING.zh-CN.md) | [Project overview](../README.md)

## Availability

Download all files from the same release and follow the [installation steps](INSTALL-TESTING.md).
The system archive may be split into several files; the guide includes the joining command.

## Supported Device

Xiaomi Pad 6 Pro, codename `liuqin`, SM8475. Other Xiaomi Pad models are not
compatible. Initial installation and first boot have been tested on one known
**256 GB partition layout**. Android recovery has not yet been independently validated.
128 GB, 512 GB, other capacities, modified layouts and slot-B installation are
unverified and unsupported. Equal capacity does not imply equal layout. Do not
change constants or bypass checks to force an installation.

The bootloader must be unlocked and slot A active. Device-side checks require
Linux sysfs values of 493854720 sectors for sda, start 22065152 and size 471789528
for sda35, and partition name userdata. These counts use 512-byte sectors, not
filesystem block sizes. Unknown or mismatching device, layout, slot or session
identity must stop installation. Checks reduce risk but do not guarantee recovery
or replace real installation testing.

## Data and Recovery

Ubuntu will use the Android userdata partition. Initial installation is
destructive and does not provide Android dual boot. Unlocking the bootloader
also erases user data.

Before installation:

1. Back up personal files outside the tablet.
2. Obtain the stock firmware matching the device and retain its recovery instructions.
3. Back up the original boot partitions and factory `persist` partition before
   overwriting any boot partition. A RAM boot and a persistent flash are different
   operations; backing up after flashing does not preserve the original image.

The installer must read factory calibration and addresses from the same tablet.
Never use another tablet's `persist` image or calibration data.

Returning to Android requires restoring the appropriate stock firmware and
preparing userdata for Android. Replacing only the boot image does not undo an
Ubuntu installation. Keep the bootloader unlocked while non-stock boot images
remain installed.

## Release Bundle

A supported release will provide matching boot and root filesystem images,
installation tools, checksums, source revisions, supported storage layouts and
recovery requirements. The installer verifies images before accessing the device.
Kernel build artifacts alone are not installation images.
