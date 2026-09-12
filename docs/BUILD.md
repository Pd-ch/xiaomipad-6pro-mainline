# Building

[中文](BUILD.zh-CN.md) | [Project overview](../README.md)

## Kernel

Use Linux with the project and kernel checkouts beside each other:

```text
workspace/
  xiaomipad-6pro-mainline/
  linux-sm8450-liuqin/
```

The kernel checkout must match the commit in `kernel/source.json`. The repository
is based on `sm8450-mainline/linux`; the device branch is `liuqin-6.17`.
The build script verifies the commit and generated configuration before compiling.

On Ubuntu 24.04, install the kernel build dependencies:

```sh
sudo apt-get update
sudo apt-get install build-essential bc bison flex libssl-dev libelf-dev \
  gcc-aarch64-linux-gnu binutils-aarch64-linux-gnu python3 git ccache kmod rsync
```

From the project checkout:

```sh
python3 tools/build-liuqin-kernel.py --configure-only
python3 tools/build-liuqin-kernel.py --jobs 12
```

The second command builds the Image, device tree and modules under `out/kernel`.
It does not access the tablet. Available outputs include:

| Path under out/kernel | Content |
|---|---|
| arch/arm64/boot/Image | ARM64 kernel |
| arch/arm64/boot/dts/qcom/sm8475-xiaomi-liuqin.dtb | Device tree |
| modules/lib/modules/ | Installed kernel modules |
| .config | Effective kernel configuration |
| vmlinux, System.map | Debugging and symbol information |
| build-info.json, SHA256SUMS | Source/toolchain identity and core output checksums |

Use `--source PATH` for a different kernel checkout and `--out PATH` for a
different output directory. Repeated builds reuse compatible outputs and ccache.
Changed build inputs require a new output directory. The default compiler is
`aarch64-linux-gnu-gcc`; `CROSS_COMPILE` selects another toolchain prefix.

A successful kernel build does not establish hardware functionality or produce
a complete installation image. Byte-for-byte comparisons require the same
complete toolchain and build inputs.

## Userspace and Images

The device integration sources include system services, audio configuration,
power-key support, sensor patches and the GNOME Settings patch. Sensor source
versions are recorded in `device/sensors/sources.manifest`.

GNOME Settings uses the Ubuntu source package and the patch recorded in
`device/gnome-control-center/source.json`. It requires Python 3.12, curl, patch,
dpkg-dev and an AArch64 binfmt interpreter on the host, plus the prepared Ubuntu
ARM64 root filesystem. Prepare its source with:

```sh
python3 tools/build-liuqin-settings.py --prepare-only
```

Build with `sudo python3 tools/build-liuqin-settings.py --jobs 8`. The build runs
inside an isolated mount namespace and does not modify the input root filesystem.
The device package builder reads its binary and source identity from
`out/gnome-control-center/`; there is no dependency on a historical Settings binary.

The speaker topology is built from the AudioReach source revision
`2af1f1ebb8d4fd03b5f53891467ddde2e208a8a0` in
[linux-msm/audioreach-topology](https://github.com/linux-msm/audioreach-topology).
With that checkout available, run:

```sh
AUDIOREACH_TOPOLOGY_DIR="$PWD/../audioreach-topology" \
  OUTPUT="$PWD/out/audio-topology/Xiaomi-Pad-6-Pro-tplg.bin" \
  sh tools/build-liuqin-audio-topology.sh
```

The native boot builder consumes the selected kernel's Image and DTB, an
assembled root file manifest, the topology and the prepared firmware inputs.
It does not require an older boot image or a recovery root filesystem.

| Input | Variable |
|---|---|
| Kernel source checkout and build output | KERNEL_SOURCE, KERNEL_OUT |
| Assembled root's native-root.hashes | NATIVE_ROOT_HASHES |
| Compiled speaker topology | AUDIO_TOPOLOGY |
| Prepared device firmware directories | FIRMWARE_POOL |
| HSP2 WLAN tuple | WLAN_HSP2_TUPLE |
| Extracted stock DTBO entries and base DTBs | STOCK_OVERLAY_DIR, STOCK_BASE_DIR |

After supplying those inputs, `sh tools/build-liuqin-native-boot.sh` writes
`out/native-boot/boot-liuqin-native.img`. The accompanying
`native-boot.identity` records input and output hashes without local paths.
An offline assembly result still requires device validation before distribution.

The complete image build is still being prepared for a clean checkout.
Versioned firmware inputs and userspace build dependencies must be available
before that workflow is supported. Do not substitute an older
boot image or a kernel from a different build.

## Continuous Integration

The workflow builds the pinned kernel using the same Python entry point as the
local build. Its artifacts are kernel build outputs, not installable Ubuntu
releases. Full-image release automation is not enabled.
