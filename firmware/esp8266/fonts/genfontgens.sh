#!/bin/sh

set -e

FLIST="10x16 12x16 16x16 6x10 apple4x6"

for font in $FLIST; do
	echo "Generating font ${font} generator"
 	gcc -O2 font_${font}.c -o ${font}-gen -I../user -DFONTGEN -I../../linux/ -DHOST
	./${font}-gen ../smallfs/${font}
done
