#!/usr/bin/env bash

#
# CDDL HEADER START
#
# This file and its contents are supplied under the terms of the
# Common Development and Distribution License ("CDDL"), version 1.0.
# You may only use this file in accordance with the terms of version
# 1.0 of the CDDL.
#
# A full copy of the text of the CDDL should have accompanied this
# source.  A copy of the CDDL is also available via the Internet at
# http://www.illumos.org/license/CDDL.
#
# CDDL HEADER END
#

#
# Copyright (c) 2015 by Delphix. All rights reserved.
# Copyright (C) 2016 Lawrence Livermore National Security, LLC.
# Copyright (c) 2017, Intel Corporation.
#

BASE_DIR=$(dirname "$0")
SCRIPT_COMMON=common.sh
if [ -f "${BASE_DIR}/${SCRIPT_COMMON}" ]; then
	. "${BASE_DIR}/${SCRIPT_COMMON}"
else
	echo "Missing helper script ${SCRIPT_COMMON}" && exit 1
fi

# shellcheck disable=SC2034
PROG=zloop.sh
GDB=${GDB:-gdb}

DEFAULTWORKDIR=/var/tmp
DEFAULTCOREDIR=/var/tmp/zloop

function usage
{
	cat >&2 <<EOF

$0 [-hl] [-c <dump directory>] [-f <vdev directory>]
  [-m <max core dumps>] [-s <vdev size>] [-t <timeout>]
  [-I <max iterations>] [-- [extra ztest parameters]]

  This script runs ztest repeatedly with randomized arguments.
  If a crash is encountered, the ztest logs, any associated
  vdev files, and core file (if one exists) are moved to the
  output directory ($DEFAULTCOREDIR by default). Any options
  after the -- end-of-options marker will be passed to ztest.

  Options:
    -c  Specify a core dump directory to use.
    -f  Specify working directory for ztest vdev files.
    -h  Print this help message.
    -l  Create 'ztest.core.N' symlink to core directory.
    -m  Max number of core dumps to allow before exiting.
    -s  Size of vdev devices.
    -t  Total time to loop for, in seconds. If not provided,
        zloop runs forever.
    -I  Max number of iterations to loop before exiting.

EOF
}

function or_die
{
	# shellcheck disable=SC2068
	if ! $@; then
		echo "Command failed: $*"
		exit 1
	fi
}

case $(uname) in
FreeBSD)
	coreglob="z*.core"
	;;
Linux)
	# core file helpers
	origcorepattern="$(cat /proc/sys/kernel/core_pattern)"
	coreglob="$(grep -E -o '^([^|%[:space:]]*)' /proc/sys/kernel/core_pattern)*"

	if [[ $coreglob = "*" ]]; then
		echo "Setting core file pattern..."
		echo "core" > /proc/sys/kernel/core_pattern
		coreglob="$(grep -E -o '^([^|%[:space:]]*)' \
		    /proc/sys/kernel/core_pattern)*"
	fi
	;;
*)
	exit 1
	;;
esac

function core_file
{
	# shellcheck disable=SC2012,SC2086
	ls -tr1 $coreglob 2>/dev/null | head -1
}

function core_prog
{
	# shellcheck disable=SC2154
	prog=$ZTEST
	core_id=$($GDB --batch -c "$1" | grep "Core was generated by" | \
	    tr  \' ' ')
	if [[ "$core_id" == *"zdb "* ]]; then
		# shellcheck disable=SC2154
		prog=$ZDB
	fi
	printf "%s" "$prog"
}

function store_core
{
	core="$(core_file)"
	if [[ $ztrc -ne 0 ]] || [[ -f "$core" ]]; then
		df -h "$workdir" >>ztest.out
		coreid=$(date "+zloop-%y%m%d-%H%M%S")
		foundcrashes=$((foundcrashes + 1))

		# zdb debugging
		zdbcmd="$ZDB -U "$workdir/zpool.cache" -dddMmDDG ztest"
		zdbdebug=$($zdbcmd 2>&1)
		echo -e "$zdbcmd\n" >>ztest.zdb
		echo "$zdbdebug" >>ztest.zdb

		dest=$coredir/$coreid
		or_die mkdir -p "$dest"
		or_die mkdir -p "$dest/vdev"

		if [[ $symlink -ne 0 ]]; then
			or_die ln -sf "$dest" "ztest.core.${foundcrashes}"
		fi

		echo "*** ztest crash found - moving logs to $dest"

		or_die mv ztest.history "$dest/"
		or_die mv ztest.zdb "$dest/"
		or_die mv ztest.out "$dest/"
		or_die mv "$workdir/ztest*" "$dest/vdev/"

		if [[ -e "$workdir/zpool.cache" ]]; then
			or_die mv "$workdir/zpool.cache" "$dest/vdev/"
		fi

		# check for core
		if [[ -f "$core" ]]; then
			coreprog=$(core_prog "$core")
			coredebug=$($GDB --batch --quiet \
			    -ex "set print thread-events off" \
			    -ex "printf \"*\n* Backtrace \n*\n\"" \
			    -ex "bt" \
			    -ex "printf \"*\n* Libraries \n*\n\"" \
			    -ex "info sharedlib" \
			    -ex "printf \"*\n* Threads (full) \n*\n\"" \
			    -ex "info threads" \
			    -ex "printf \"*\n* Backtraces \n*\n\"" \
			    -ex "thread apply all bt" \
			    -ex "printf \"*\n* Backtraces (full) \n*\n\"" \
			    -ex "thread apply all bt full" \
			    -ex "quit" "$coreprog" "$core" 2>&1 | \
			    grep -v "New LWP")

			# Dump core + logs to stored directory
			echo "$coredebug" >>"$dest/ztest.gdb"
			or_die mv "$core" "$dest/"

			# Record info in cores logfile
			echo "*** core @ $coredir/$coreid/$core:" | \
			    tee -a ztest.cores
		fi

		if [[ $coremax -gt 0 ]] &&
		   [[ $foundcrashes -ge $coremax ]]; then
			echo "exiting... max $coremax allowed cores"
			exit 1
		else
			echo "continuing..."
		fi
	fi
}

# parse arguments
# expected format: zloop [-t timeout] [-c coredir] [-- extra ztest args]
coredir=$DEFAULTCOREDIR
basedir=$DEFAULTWORKDIR
rundir="zloop-run"
timeout=0
size="512m"
coremax=0
symlink=0
iterations=0
while getopts ":ht:m:I:s:c:f:l" opt; do
	case $opt in
		t ) [[ $OPTARG -gt 0 ]] && timeout=$OPTARG ;;
		m ) [[ $OPTARG -gt 0 ]] && coremax=$OPTARG ;;
		I ) [[ -n $OPTARG ]] && iterations=$OPTARG ;;
		s ) [[ -n $OPTARG ]] && size=$OPTARG ;;
		c ) [[ -n $OPTARG ]] && coredir=$OPTARG ;;
		f ) [[ -n $OPTARG ]] && basedir=$(readlink -f "$OPTARG") ;;
		l ) symlink=1 ;;
		h ) usage
		    exit 2
		    ;;
		* ) echo "Invalid argument: -$OPTARG";
		    usage
		    exit 1
	esac
done
# pass remaining arguments on to ztest
shift $((OPTIND - 1))

# enable core dumps
ulimit -c unlimited
export ASAN_OPTIONS=abort_on_error=1:disable_coredump=0

if [[ -f "$(core_file)" ]]; then
	echo -n "There's a core dump here you might want to look at first... "
	core_file
	echo
	exit 1
fi

if [[ ! -d $coredir ]]; then
	echo "core dump directory ($coredir) does not exist, creating it."
	or_die mkdir -p "$coredir"
fi

if [[ ! -w $coredir ]]; then
	echo "core dump directory ($coredir) is not writable."
	exit 1
fi

or_die rm -f ztest.history
or_die rm -f ztest.zdb
or_die rm -f ztest.cores

ztrc=0		# ztest return value
foundcrashes=0	# number of crashes found so far
starttime=$(date +%s)
curtime=$starttime
iteration=0

# if no timeout was specified, loop forever.
while (( timeout == 0 )) || (( curtime <= (starttime + timeout) )); do
	if (( iterations > 0 )) && (( iteration++ == iterations )); then
		break
	fi

	zopt="-G -VVVVV"

	# start each run with an empty directory
	workdir="$basedir/$rundir"
	or_die rm -rf "$workdir"
	or_die mkdir "$workdir"

	# switch between three types of configs
	# 1/3 basic, 1/3 raidz mix, and 1/3 draid mix
	choice=$((RANDOM % 3))

	# ashift range 9 - 15
	align=$(((RANDOM % 2) * 3 + 9))

	# randomly use special classes
	class="special=random"

	if [[ $choice -eq 0 ]]; then
		# basic mirror only
		parity=1
		mirrors=2
		draid_data=0
		draid_spares=0
		raid_children=0
		vdevs=2
		raid_type="raidz"
	elif [[ $choice -eq 1 ]]; then
		# fully randomized mirror/raidz (sans dRAID)
		parity=$(((RANDOM % 3) + 1))
		mirrors=$(((RANDOM % 3) * 1))
		draid_data=0
		draid_spares=0
		raid_children=$((((RANDOM % 9) + parity + 1) * (RANDOM % 2)))
		vdevs=$(((RANDOM % 3) + 3))
		raid_type="raidz"
	else
		# fully randomized dRAID (sans mirror/raidz)
		parity=$(((RANDOM % 3) + 1))
		mirrors=0
		draid_data=$(((RANDOM % 8) + 3))
		draid_spares=$(((RANDOM % 2) + parity))
		stripe=$((draid_data + parity))
		extra=$((draid_spares + (RANDOM % 4)))
		raid_children=$(((((RANDOM % 4) + 1) * stripe) + extra))
		vdevs=$((RANDOM % 3))
		raid_type="draid"
	fi

	zopt="$zopt -K $raid_type"
	zopt="$zopt -m $mirrors"
	zopt="$zopt -r $raid_children"
	zopt="$zopt -D $draid_data"
	zopt="$zopt -S $draid_spares"
	zopt="$zopt -R $parity"
	zopt="$zopt -v $vdevs"
	zopt="$zopt -a $align"
	zopt="$zopt -C $class"
	zopt="$zopt -s $size"
	zopt="$zopt -f $workdir"

	cmd="$ZTEST $zopt $*"
	desc="$(date '+%m/%d %T') $cmd"
	echo "$desc" | tee -a ztest.history
	echo "$desc" >>ztest.out
	$cmd >>ztest.out 2>&1
	ztrc=$?
	grep -E '===|WARNING' ztest.out >>ztest.history

	store_core

	curtime=$(date +%s)
done

echo "zloop finished, $foundcrashes crashes found"

# restore core pattern.
case $(uname) in
Linux)
	echo "$origcorepattern" > /proc/sys/kernel/core_pattern
	;;
*)
	;;
esac

uptime >>ztest.out

if [[ $foundcrashes -gt 0 ]]; then
	exit 1
fi
