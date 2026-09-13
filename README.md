# Ubuntu for Xiaomi Pad 6 Pro

Run Ubuntu 26.04 on the Xiaomi Pad 6 Pro, with the GNOME desktop and a device-adapted kernel based on upstream Linux.

**WeChat: 熵减矩阵 · Xiaohongshu: yzddmr6**
Usage tips, AI discussions and community exchange, primarily in Chinese. [中文](README.zh-CN.md)

## ⚠️ Read Before Installing

This project is only for the **Xiaomi Pad 6 Pro (liuqin, SM8475)**. Other Xiaomi Pad models are not compatible.

- **Data loss**: unlocking the bootloader and performing an initial installation erase user data. Back up your files first.
- **Storage and layout**: installation testing currently targets one known **256 GB partition layout**, not every 256 GB device. Other capacities, modified partition layouts and slot-B installation are unverified and unsupported. Do not bypass the checks.
- **Validation stage**: the complete installer's initial installation, first boot and Android recovery still require device testing. The hardware status table is not an installation-bundle acceptance report.
- **Installation layout**: Ubuntu is installed as the sole operating system. Android dual boot is not provided.
- **Recovery preparation**: obtain the matching stock firmware and read the [data and recovery instructions](docs/FLASHING.md#data-and-recovery) before installing.
- **Hardware limitations**: some features are incomplete. Review the hardware support table below.

Flashing an incorrect boot image or partition can prevent the tablet from starting. Follow the installation guide and keep your backups.
Factory calibration and device addresses must come from the same tablet; never copy them between devices.

## 🚀 Getting Started

Public installation images are not available yet.

| I want to | Read |
|---|---|
| Install Ubuntu | [Installation guide](docs/FLASHING.md) |
| Build the kernel and device components | [Build guide](docs/BUILD.md) |
| Restore Android | [Data and recovery instructions](docs/FLASHING.md#data-and-recovery) |

Prebuilt installation bundles will accompany project releases, with matching boot and root filesystem images, installation tools and checksums.

## Hardware Support

The table describes the current port. Complete installation images still require release validation.

| Feature | Support | Details |
|---|---|---|
| Display | Working | 2880 x 1800, 120 Hz |
| Touchscreen | Working | Touch input and gestures |
| GPU | Working | Adreno 730, Freedreno; some applications need rendering workarounds |
| Wi-Fi | Working | 2.4 GHz and 5 GHz connections |
| Bluetooth | Partial | Controller initialization and scanning; pairing and reconnection need further testing |
| Speakers | Working | Four speakers; factory calibration is read from each tablet; audio tuning remains in progress |
| Magnetic keyboard | Working | Character keys, volume keys and reattachment |
| Automatic rotation | Partial | Rotation demonstrated; startup and login reliability still need final validation |
| Power and volume keys | Working | Screen control, power menu and volume adjustment |
| Charging | Partial | Basic charging; Xiaomi proprietary fast charging is not supported |
| Suspend / resume | Partial | Basic resume demonstrated; peripheral recovery and deep-sleep power use need further testing |
| H.264 hardware decoding | Working | Iris / V4L2 decoding; browser integration and other codecs are not validated |
| Automatic brightness | Not supported | Automatic brightness control is not implemented |
| Cameras | Not supported | No working camera integration |
| Microphone | Not validated | Recording has not been verified |
| Stylus | Not validated | Pen input has not been verified |

## Usage and Maintenance

Ubuntu packages are managed through APT. Update instructions for the project kernel and device components will accompany each installation release.
Do not mix boot images and system components from different releases.

Report problems through GitHub Issues with the device model, system version, reproduction steps and relevant logs.
Remove passwords, network credentials and personal information before sharing logs.
The accounts listed above are also available for usage discussions.

## Development and Contributions

| Repository | Contents |
|---|---|
| xiaomipad-6pro-mainline | Device configuration, userspace integration, build and installation tools, documentation |
| linux-sm8450-liuqin | Complete Linux kernel source and device adaptation commits |

[kernel/source.json](kernel/source.json) records the kernel revision used by the build.
The device branch is `liuqin-6.17`. Contributions to drivers, tools and documentation are welcome;
see [CONTRIBUTING.md](CONTRIBUTING.md).

## Acknowledgments and Licensing

This project builds on the kernel work of [sm8450-mainline](https://github.com/sm8450-mainline/linux),
along with Ubuntu, GNOME, Freedreno and the Linux Qualcomm community.

Original project code is MIT-licensed unless a file states otherwise. Linux and third-party components retain their own licenses;
firmware is subject to its respective owners' terms. See [LICENSE](LICENSE) and [NOTICE](NOTICE).
