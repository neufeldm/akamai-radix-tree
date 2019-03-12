#!/bin/sh
zcat "$@" | sed 's/[_,]/ /g' | awk '{print $1"/"$2" "$3}' | less
