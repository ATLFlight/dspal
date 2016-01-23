#!/bin/bash
TOOLSDIR=$( dirname "${BASH_SOURCE[0]}" )
for f in $(find . -name '*.c' -o -name '*.cpp' -o -name '*.hpp' -o -name '*.h'); do
	astyle --options=${TOOLSDIR}/astylerc --preserve-date --quiet < $f > $f.pretty
	diffsize=$(diff -y --suppress-common-lines $f $f.pretty | wc -l)
	if [ $diffsize -ne 0 ]; then
		cp $f.pretty $f
	fi
	rm -f $f.pretty
done
