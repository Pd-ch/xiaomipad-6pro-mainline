# 安装测试

[English](INSTALL-TESTING.md)

安装器仍在开发中，尚无经过完整验收的安装版本。以下步骤仅供有人在场、具备恢复条件的测试。

## 准备

- Xiaomi Pad 6 Pro（liuqin），256 GB 存储布局；其他容量会被拒绝。
- Bootloader 已解锁，A 槽处于活动状态，平板进入 Fastboot，电量至少 30%。
- Linux 主机、Python 3.11 或更新版本、Android platform-tools，以及正常的 USB 网络支持。
- 个人文件已备份到平板以外；安装会清空整个 userdata，安装器不会备份个人文件。
- 已准备适配本机、满足防回滚要求的原厂 Fastboot ROM，并明确如何恢复 Android。

## 校验与执行

进入安装包目录，先校验本地文件；这两条命令不访问设备：

```sh
sha256sum -c SHA256SUMS
python3 install.py --bundle . --check
```

确认允许本次清空数据后，执行待验证候选的测试安装：

```sh
python3 install.py --bundle . --serial DEVICE_SERIAL \
  --backup /path/to/new-private-backup --erase-userdata --allow-unverified
```

安装器临时启动 installer.img，等待 USB 网络，备份并校验 boot_a、boot_b 与 persist；
随后下载并校验系统归档，格式化 userdata，安装系统并提取本机校准和地址。
根文件系统安装成功且卸载后，才写入 boot_a 并重启。不会修改分区表、写入 persist、
自动切换槽位或重新锁定 Bootloader。备份必须放在安装包目录以外，并保持私密。

USB 网络通常通过 DHCP 配置；必要时可用 `--host-address` 指定主机 USB 网卡地址。
安装 RAM 环境的救援 shell 没有身份认证，只能使用可信的直连 USB，不要接入共享网络。
失败后先保留报错与备份，确认已完成哪些步骤，不要直接反复重跑。

## 恢复 Android

恢复会清除 Ubuntu，需要使用匹配的原厂 Fastboot ROM 完成系统恢复和 userdata 初始化。
仅还原 boot_a 不等于恢复 Android。

使用原厂完整清刷流程，不用保留数据或重新上锁的变体；保留原厂防回滚检查。
不得恢复其他平板的 persist 或校准。在仍有非原厂镜像时保持 Bootloader 解锁。
原厂 ROM 从上游取得，不在本项目重复托管。

当前安装器与恢复路线仍须在最终候选上完成真机测试；编译和本地校验通过不能替代这一步。
