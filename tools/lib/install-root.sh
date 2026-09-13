#!/bin/sh
# SPDX-License-Identifier: MIT
# Device-side installer, intended only for the dedicated read-only RAM image.
set -eu
die() { printf 'liuqin-install: %s\n' "$*" >&2; exit 1; }
[ "$#" = 5 ] || die 'usage: install-root.sh BOOT_ID ROOTFS_URL SHA256 BYTES ERASE-LIUQIN-USERDATA'
[ "$5" = ERASE-LIUQIN-USERDATA ] || die 'explicit data-erasure acknowledgement required'
[ "$(cat /proc/sys/kernel/random/boot_id)" = "$1" ] || die 'RAM boot identity changed'
[ "$(cat /etc/liuqin-installer 2>/dev/null)" = liuqin ] || die 'not the installer RAM image'
ln -sf /proc/self/fd/0 /dev/stdin
# The host checks Fastboot product/serial; boot_id binds this operation to it.
[ "$(cat /sys/class/block/sda/size)" = 493854720 ] || die 'unsupported storage capacity'
[ "$(cat /sys/class/block/sda35/start)" = 22065152 ] || die 'userdata start mismatch'
[ "$(cat /sys/class/block/sda35/size)" = 471789528 ] || die 'userdata size mismatch'
grep -q '^PARTNAME=userdata$' /sys/class/block/sda35/uevent || die 'not userdata'
device_number=$(cat /sys/class/block/sda35/dev)
awk -v device="$device_number" '$3 == device {found=1} END {exit !found}' /proc/self/mountinfo &&
	die 'userdata is mounted (including through a device alias)'
[ "$(/bin/busybox blockdev --getro /dev/sda35)" = 1 ] || die 'userdata is not initially read-only'
battery=
for supply in /sys/class/power_supply/*; do
	[ "$(cat "$supply/type" 2>/dev/null)" = Battery ] || continue
	battery=$(cat "$supply/capacity")
	break
done
case $battery in ''|*[!0-9]*) die 'battery level is unavailable' ;; esac
[ "$battery" -ge 30 ] || die 'charge the tablet to at least 30 percent before installation'
case $3 in *[!0-9a-f]*|'') die 'invalid rootfs hash' ;; esac
[ "${#3}" = 64 ] || die 'invalid rootfs hash length'
case $4 in ''|*[!0-9]*) die 'invalid archive size' ;; esac
available=$(awk '/^MemAvailable:/ {print $2}' /proc/meminfo)
[ "$4" -gt 0 ] && [ "$(( $4 + 536870912 ))" -lt "$((available * 1024))" ] ||
	die 'insufficient RAM to stage this archive safely'
archive=/mnt/install-download/rootfs.tar.gz
mounted=false
opened=false
persist_mounted=false
download_mounted=false
cleanup() {
	sync
	if [ "$mounted" = true ]; then
		umount /mnt/install || return 1
		mounted=false
	fi
	if [ "$opened" = true ]; then
		/bin/busybox blockdev --setro /dev/sda35
		/bin/busybox blockdev --setro /dev/sda
		opened=false
	fi
	if [ "$persist_mounted" = true ]; then
		umount /run/persist || return 1
		persist_mounted=false
	fi
	if [ "$download_mounted" = true ]; then
		umount /mnt/install-download || return 1
		download_mounted=false
	fi
}
trap cleanup EXIT
trap 'exit 130' HUP INT TERM
# A default /run tmpfs is too small for some complete desktop archives.
mkdir -p /mnt/install-download /run/persist /mnt/install
mount -t tmpfs -o "size=$(( $4 + 16777216 ))" tmpfs /mnt/install-download
download_mounted=true
/bin/busybox wget -O "$archive" "$2"
[ "$(stat -c %s "$archive")" = "$4" ] || die 'downloaded archive size mismatch'
printf '%s  %s\n' "$3" "$archive" | /bin/busybox sha256sum -c -
[ "$(cat /proc/sys/kernel/random/boot_id)" = "$1" ] || die 'RAM identity changed before formatting'
[ -b /dev/disk/by-partlabel/persist ] || die 'persist is missing'
mount -t ext4 -o ro,noload /dev/disk/by-partlabel/persist /run/persist
persist_mounted=true
for item in wlan/wlan_mac.bin bluetooth/.bt_nv.bin audio/crus_calr.bin; do
	[ -f "/run/persist/$item" ] || die 'factory data is incomplete'
done
[ -d /run/persist/sensors/registry/registry ] || die 'factory sensor registry is missing'
/bin/busybox blockdev --setrw /dev/sda
opened=true
/bin/busybox blockdev --setrw /dev/sda35
/usr/sbin/mkfs.ext4 -F -L LIUQIN_ROOT -m 0 /dev/sda35
mount -t ext4 /dev/sda35 /mnt/install
mounted=true
mkdir /mnt/install/native-root
/usr/bin/tar -xzf "$archive" -C /mnt/install/native-root \
	--numeric-owner --same-owner --same-permissions --acls --xattrs --xattrs-include='*' --warning=no-timestamp
PERSIST_SRC=/run/persist sh /usr/lib/liuqin/provision.sh /mnt/install/native-root
[ -n "$(/usr/sbin/getcap /mnt/install/native-root/usr/lib/snapd/snap-confine)" ] ||
	die 'snap-confine capability was not restored'
# Verify immutable boot-contract files after extraction and provisioning.
tail -n +2 /etc/liuqin-native-root.contract | while read -r expected path; do
	[ -n "$expected" ] || continue
	actual=$(/bin/busybox sha256sum "/mnt/install/native-root$path" | /bin/busybox cut -d' ' -f1)
	[ "$actual" = "$expected" ] || die "root contract mismatch: $path"
done
cleanup
trap - EXIT HUP INT TERM
printf 'liuqin-install: ROOT_INSTALLED\n'
