# Installation

[中文](FLASHING.zh-CN.md) | [Project overview](../README.md)

## Availability

The public installer and installation images are not released yet. Installation
commands will accompany a tested release bundle.

## Supported Device

Xiaomi Pad 6 Pro, codename `liuqin`, SM8475. Other Xiaomi Pad models are not
compatible. Supported storage variants will be listed with the release.

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
tested recovery instructions. Verify the entire bundle before installation.
Kernel build artifacts alone are not installation images.
