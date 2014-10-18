#!/bin/bash

function usage() {
	echo "Usage: $(basename $0) git-repository"
	echo "       $(basename $0) directory1 directory1"
	echo "       $(basename $0) tar1 tar2"
}

function print_header() {
	echo "LoC;Changed LoC;Relative Code Churn;"
}

function diff() {
	while read -r cur; do
		if [ -z $prev ]; then
			local prev=$cur
			continue
		fi

		git diff --numstat $prev $cur | grep '^[[:digit:]]' | grep '\.c$\|\.h$' | awk 'BEGIN { churn=0 } { churn+=$1; churn+=$2 } END { print churn }'

		local prev=$cur
	done < <(git rev-list --all --no-merges --reverse) | awk 'BEGIN { churn=0 } { churn+=$1 } END { print churn }'
}

function getLoC() {
	tmpfile=$(mktemp) || ( echo "Error: could not create temporary file!" && exit 1 )

	find -regex '.*/.*\.\(c\|h\)$' -print0 > $tmpfile
	local loc=$(wc --files0-from=$tmpfile | tail -n 1 | awk ' {print $1 }')

	rm $tmpfile
	echo $loc
}

function float_eval()
{
	local float_scale=3
	local stat=0
	local result=0.0
	if [[ $# -gt 0 ]]; then
		result=$(echo "scale=$float_scale; $*" | bc -q 2>/dev/null)
		stat=$?
		if [[ $stat -eq 0  &&  -z "$result" ]]; then stat=1; fi
	fi
	echo $result
	return $stat
}

function calculate() {
	local loc=$(getLoC)
	local changed_loc=$(diff)
	local relative_churn=$(float_eval "$changed_loc / $loc")
	echo "${loc};${changed_loc};${relative_churn}"
}

function commit_all() {
	git add -A >/dev/null 2>&1
	git commit -a -m "Automatic commit of $1" >/dev/null 2>&1
}

function commit_dir() {
	[ ! -d $1 ] && echo "Error: Not a directory: $1" && exit 1
	rsync -aq --delete --exclude='.git/' ${1}/ .
	commit_all "$1"
}

function import_dirs() {
	tmprepo=$(mktemp -d) || ( echo "Error: could not create temporary directory!" && exit 1 )

	local dir1=$(realpath $1)
	local dir2=$(realpath $2)
	
	cd $tmprepo
	git init >/dev/null 2>&1
	commit_dir $dir1
	commit_dir $dir2
	echo $tmprepo
}

function check_tar() {
	tar -taf $1 >/dev/null
	return $?
}

function import_tars() {
	tmpdir=$(mktemp -d) || ( echo "Error: could not create temporary directory!" && exit 1 )

	if ! check_tar $1; then
		echo "Error. Something is wrong with the tar: $1"
		exit 1
	fi

	if ! check_tar $2; then
		echo "Error. Something is wrong with the tar: $2"
		exit 1
	fi

	tar -C $tmpdir -xaf $1 >/dev/null
	tar -C $tmpdir -xaf $2 >/dev/null
	
	local tmprepo=$(import_dirs ${tmpdir}/*)
	rm -rf $tmpdir
	echo $tmprepo
}

while getopts ":h" opt; do
	case $opt in
		h)
			usage
			exit 1
			;;
		\?)
			echo "Invalid option: -$OPTARG" >&2
			exit 1
			;;
	esac
done

if [ -z $1 ]; then
	usage
	exit 1
fi

delete=0

if [ -z $2 ]; then
	# analyzing a git repository
	if [ ! -d $1/.git ]; then
		echo "$1 is not a git repository!"
		echo
		usage
		exit 1
	fi
	gitrepo=$1
else
	if [ -d $1 ] && [ -d $2 ]; then
		# analyzing two directories
		gitrepo=$(import_dirs $1 $2)
		delete=1
	elif [ -f $1 ] && [ -f $2 ]; then
		# analyzing two tars
		gitrepo=$(import_tars $1 $2)
		delete=1
	else
		echo "Error: input arguments must be of same type."
		exit 1
	fi
fi

[ ! -d "${gitrepo/.git}" ] && echo "Error." && exit 1
cd $gitrepo
print_header
calculate

if [ $delete -ne 0 ] && [ ! -z $gitrepo ] && [ -d $gitrepo ]; then
	rm -rf $gitrepo
fi

