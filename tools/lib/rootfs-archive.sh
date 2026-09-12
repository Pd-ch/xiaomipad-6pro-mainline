#!/bin/sh
# SPDX-License-Identifier: MIT
# Local archive operations only; no device access or partition changes.
set -eu

die() { printf 'rootfs-archive: %s\n' "$*" >&2; exit 1; }
[ "$#" = 3 ] || die 'usage: rootfs-archive.sh pack|extract ROOT ARCHIVE.tar.gz'
action=$1
root=$(realpath -m -- "$2")
archive=$(realpath -m -- "$3")
[ "$(id -u)" = 0 ] || die 'root is required to preserve numeric ownership and capabilities'
tar --version | grep -q 'GNU tar' || die 'GNU tar is required; BusyBox tar is not supported'
[ "$root" != / ] || die 'the host root is not a valid target'

case $action in
pack)
	[ -d "$root" ] || die 'root directory is missing'
	case $archive in "$root"/*) die 'archive must be outside the input tree' ;; esac
	[ ! -e "$archive" ] && [ ! -L "$archive" ] || die 'archive already exists'
	temporary=$(mktemp "${archive}.XXXXXX")
	trap 'rm -f -- "$temporary"' EXIT HUP INT TERM
	tar --create --use-compress-program='gzip -1' --file="$temporary" --directory="$root" \
		--format=pax --numeric-owner --acls --xattrs --xattrs-include='*' \
		--sort=name --pax-option=delete=atime,delete=ctime .
	chmod 0644 "$temporary"
	ln -- "$temporary" "$archive"
	;;
extract)
	[ -f "$archive" ] || die 'archive is missing'
	[ ! -e "$root" ] && [ ! -L "$root" ] || die 'extraction requires a new directory'
	mkdir -- "$root"
	tar --extract --gzip --file="$archive" --directory="$root" \
		--numeric-owner --same-owner --same-permissions --acls --xattrs --xattrs-include='*'
	;;
*) die 'expected pack or extract' ;;
esac
