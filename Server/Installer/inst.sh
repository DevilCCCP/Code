#!/bin/bash

head=$1
dst=$2
src="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
files="instd.service"


if [ ! -d "$dst/Updates" ]; then
 mkdir "$dst/Updates"
fi

/bin/bash "$head/Lib/Include/deploy.sh" "$src/src" "$dst/Updates" "$files"
