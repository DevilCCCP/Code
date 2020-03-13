#!/bin/bash

head=$1
dst=$2
src="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
scripts="storage_resize.sql storage_cell.sql storage_current_cell.sql get_current_cell.sql get_next_cell.sql"

/bin/bash "$head/Lib/Include/deploy.sh" "$head/Misc/Db/StorageV" "$dst/Scripts" "$scripts"
