#!/bin/sh
#
# This script generates a "memstick image" (image that can be copied to a
# USB memory stick) from a directory tree.  Note that the script does not
# clean up after itself very well for error conditions on purpose so the
# problem can be diagnosed (full filesystem most likely but ...).
#
# Usage: make-memstick.sh <directory tree or manifest> <image filename>
#
# $FreeBSD$
#

set -e
# XXXOP - be more verbose
set -x

scriptdir=$(dirname $(realpath $0))
. ${scriptdir}/../../tools/boot/install-boot.sh

if [ "$(uname -s)" = "FreeBSD" ]; then
	PATH=/bin:/usr/bin:/sbin:/usr/sbin
	export PATH
fi

if [ $# -ne 2 ]; then
	echo "make-memstick.sh /path/to/directory/or/manifest /path/to/image/file"
	exit 1
fi

MAKEFSARG=${1}

if [ -f ${MAKEFSARG} ]; then
	BASEBITSDIR=`dirname ${MAKEFSARG}`
	METALOG=${MAKEFSARG}
elif [ -d ${MAKEFSARG} ]; then
	BASEBITSDIR=${MAKEFSARG}
	METALOG=
else
	echo "${MAKEFSARG} must exist"
	exit 1
fi

if [ -e ${2} ]; then
	echo "won't overwrite ${2}"
	exit 1
fi

<<<<<<< HEAD
echo '/dev/ufs/HardenedBSD_Install / ufs ro,noatime 1 1' > ${1}/etc/fstab
echo 'root_rw_mount="NO"' > ${1}/etc/rc.conf.local
makefs -B little -o label=HardenedBSD_Install -o version=2 ${2}.part ${1}
rm ${1}/etc/fstab
rm ${1}/etc/rc.conf.local
=======
echo '/dev/ufs/FreeBSD_Install / ufs ro,noatime 1 1' > ${BASEBITSDIR}/etc/fstab
echo 'root_rw_mount="NO"' > ${BASEBITSDIR}/etc/rc.conf.local
if [ -n "${METALOG}" ]; then
	metalogfilename=$(mktemp /tmp/metalog.XXXXXX)
	cat ${METALOG} > ${metalogfilename}
	echo "./etc/fstab type=file uname=root gname=wheel mode=0644" >> ${metalogfilename}
	echo "./etc/rc.conf.local type=file uname=root gname=wheel mode=0644" >> ${metalogfilename}
	MAKEFSARG=${metalogfilename}
fi
makefs -D -N ${BASEBITSDIR}/etc -B little -o label=FreeBSD_Install -o version=2 ${2}.part ${MAKEFSARG}
rm ${BASEBITSDIR}/etc/fstab
rm ${BASEBITSDIR}/etc/rc.conf.local
if [ -n "${METALOG}" ]; then
	rm ${metalogfilename}
fi
>>>>>>> origin/freebsd/current/main

# Make an ESP in a file.
espfilename=$(mktemp /tmp/efiboot.XXXXXX)
make_esp_file ${espfilename} ${fat32min} ${BASEBITSDIR}/boot/loader.efi

mkimg -s mbr \
    -b ${BASEBITSDIR}/boot/mbr \
    -p efi:=${espfilename} \
    -p freebsd:-"mkimg -s bsd -b ${BASEBITSDIR}/boot/boot -p freebsd-ufs:=${2}.part" \
    -a 2 \
    -o ${2}
rm ${espfilename}
rm ${2}.part

