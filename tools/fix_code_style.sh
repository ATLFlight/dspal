#!/bin/bash
TOOLSDIR=$( dirname "${BASH_SOURCE[0]}" )
if [ "$1" = "-p" ]; then
PRUNE_DIRS=$2
fi
for d in ${PRUNE_DIRS}; do
PRUNE_CMD="${PRUNE_CMD} -name $d -prune -o "
done
for f in $(find . ${PRUNE_CMD} -path "*.git/*" -prune -o -name '*.c' -o -name '*.cpp' -o -name '*.hpp' -o -name '*.h' -print); do
	astyle --options=${TOOLSDIR}/astylerc --preserve-date --quiet < $f > $f.pretty
	diffsize=$(diff -y --suppress-common-lines $f $f.pretty | wc -l)
	if [ $diffsize -ne 0 ]; then
		cp $f.pretty $f
	fi
	rm -f $f.pretty
done
