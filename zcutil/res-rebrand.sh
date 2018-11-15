#!/bin/bash
# Copyright (c) 2018 The Resistance developers
#
# This is the script we used for initial automated re-branding of Zcash to
# Resistance, followed by manual adjustments.  A goal of these changes is to
# ensure we don't misattribute our other changes to Zcash while at the same
# time keeping Zcash's copyright statements, change logs, magic values, etc.
# where appropriate.

# Fail on first error
set -e

# Require exactly one parameter - target directory tree
test $# -eq 1
D=$1
test -d $D

function stats()
{
	for W in Zcash Resistance zcash resistance; do
		echo "$W seen `fgrep -r $W $D | wc -l` times in `fgrep -rl $W $D | wc -l` files"
	done
	for W in ZEC RES zcutil resutil; do
		echo "$W seen `fgrep -rw $W $D | wc -l` times in `fgrep -rwl $W $D | wc -l` files"
	done
}

echo "Before:"
stats

fgrep -lriZ --exclude-dir=.git --exclude-dir=contrib --exclude='*release-notes*' --exclude=res-rebrand.sh \
	--exclude=chainparams.cpp \
	zcash -- $D | xargs -0 sed -i '
/[Cc]opyright\|https\{0,1\}:\|github\|libzcash\|rustzcash\|zcash\/\|Zcash\.h\|zcashconsensus\|ZcashPoW\|memcpy\|zcashd_screen\|[ *]v[01]\.\| 1\.0\./! {
s/Zcash/Resistance/g; s/zcash/resistance/g
}'

fgrep -lrwZ --exclude-dir=.git --exclude-dir=contrib --exclude='*release-notes*' --exclude=res-rebrand.sh \
	ZEC -- $D | xargs -0 sed -i 's/ZEC/RES/g'

fgrep -lrwZ --exclude-dir=.git --exclude-dir=contrib --exclude='*release-notes*' --exclude=res-rebrand.sh \
	zcutil -- $D | xargs -0 sed -i 's/zcutil/resutil/g'

echo "After:"
stats

cd $D # So that we don't rename parts of the pathname

for N in doc/man/zcash*.1; do
	mv $N ${N/zcash/resistance}
done

mv zcutil resutil
