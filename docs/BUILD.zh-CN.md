# 构建指南

[English](BUILD.md) ｜ [项目首页](../README.zh-CN.md)

## 内核

使用 Linux 主机，将项目和内核源码放在同级目录：

```text
workspace/
  xiaomipad-6pro-mainline/
  linux-sm8450-liuqin/
```

内核 checkout 必须对应 `kernel/source.json` 中的提交。内核仓库基于
`sm8450-mainline/linux`，设备分支为 `liuqin-6.17`。
构建脚本会在编译前核对源码提交和最终配置。

Ubuntu 24.04 主机依赖：

```sh
sudo apt-get update
sudo apt-get install build-essential bc bison flex libssl-dev libelf-dev \
  gcc-aarch64-linux-gnu binutils-aarch64-linux-gnu python3 git ccache kmod rsync
```

在项目目录执行：

```sh
python3 tools/build-liuqin-kernel.py --configure-only
python3 tools/build-liuqin-kernel.py --jobs 12
```

第二条命令编译 Image、设备树和模块，输出到 `out/kernel`，不会访问平板。

| out/kernel 下的路径 | 内容 |
|---|---|
| arch/arm64/boot/Image | ARM64 内核 |
| arch/arm64/boot/dts/qcom/sm8475-xiaomi-liuqin.dtb | 设备树 |
| modules/lib/modules/ | 安装后的内核模块 |
| .config | 最终内核配置 |
| vmlinux、System.map | 调试与符号信息 |
| build-info.json、SHA256SUMS | 源码/工具链身份与核心产物校验值 |

用 `--source 路径` 指定其他内核目录，用 `--out 路径` 指定其他输出目录。
重复构建复用兼容的输出和 ccache；构建输入改变时需使用新输出目录。
默认编译器为 `aarch64-linux-gnu-gcc`，可通过 `CROSS_COMPILE` 指定工具链前缀。

内核编译成功不代表真机功能验收通过，也不等于生成了完整安装镜像。
逐字节比较还需要固定完整工具链与全部构建输入。

## 用户态与镜像

设备适配源码包括系统服务、音频配置、电源键支持、传感器补丁和 GNOME 设置程序补丁。
传感器源码版本记录在 `device/sensors/sources.manifest`。

GNOME 设置程序的源码与补丁由 `device/gnome-control-center/source.json` 固定。
主机需提供 Python 3.12、curl、patch、dpkg-dev 和 AArch64 binfmt 解释器，并准备好
Ubuntu ARM64 根文件系统。先准备源码：

```sh
python3 tools/build-liuqin-settings.py --prepare-only
```

再执行 `sudo python3 tools/build-liuqin-settings.py --jobs 8`。构建使用独立挂载命名空间，
不修改输入根文件系统。设备包构建器从 `out/gnome-control-center/` 读取程序与源码身份，
不再依赖历史预编译的设置程序。

扬声器拓扑使用
[linux-msm/audioreach-topology](https://github.com/linux-msm/audioreach-topology)
的 `2af1f1ebb8d4fd03b5f53891467ddde2e208a8a0` 提交。准备好对应源码后执行：

```sh
AUDIOREACH_TOPOLOGY_DIR="$PWD/../audioreach-topology" \
  OUTPUT="$PWD/out/audio-topology/Xiaomi-Pad-6-Pro-tplg.bin" \
  sh tools/build-liuqin-audio-topology.sh
```

native boot 构建器直接使用指定内核的 Image、DTB，以及已装配根文件系统的文件清单、
音频拓扑和固件输入，不需要旧 boot 镜像或 recovery 根文件系统。

| 输入 | 环境变量 |
|---|---|
| 内核源码与构建输出 | KERNEL_SOURCE、KERNEL_OUT |
| 根文件系统的 native-root.hashes | NATIVE_ROOT_HASHES |
| 编译后的扬声器拓扑 | AUDIO_TOPOLOGY |
| 已准备的设备固件目录 | FIRMWARE_POOL |
| HSP2 WLAN 固件组合 | WLAN_HSP2_TUPLE |
| 原厂 DTBO 条目及基础 DTB | STOCK_OVERLAY_DIR、STOCK_BASE_DIR |

提供这些输入后，执行 `sh tools/build-liuqin-native-boot.sh`，输出为
`out/native-boot/boot-liuqin-native.img`。同目录的 `native-boot.identity`
记录输入与输出校验值，不包含本地路径。离线装配结果需经过真机验证后才能分发。

完整镜像从干净 checkout 构建的流程仍在准备，需补齐版本化固件输入、用户态构建依赖
后，才能提供受支持的一体化入口。不要用旧 boot 镜像或其他构建的内核代替。

## 持续集成

工作流使用与本地相同的 Python 入口编译固定版本内核，产物为内核构建文件，
不是可安装的 Ubuntu 发行包。完整镜像的发布自动化尚未启用。
