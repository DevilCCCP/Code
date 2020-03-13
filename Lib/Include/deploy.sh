#!/bin/bash

function DiffTime {
 filename=$1
 source_file=$source/$filename
 dest_file=$dest/$filename
 if [ -e $dest_file ] && [ $dest_file -nt $source_file ]; then
  files_up2date=$((files_up2date + 1))
 fi
 files_total=$((files_total + 1))
}

function Deploy {
 filename=$1
 source_file=$source/$filename
 dest_file=$dest/$filename
 echo "cp $source_file -> $dest_file"
 cp $source_file $dest_file
}


if [ "$#" -ne 3 ]; then
  echo Usage: Deploy "source path" "dest path" "file1_sub_path file2_sub_path ..."
  exit
fi

source=$1
dest=$2
files=$3

echo Deploy \'$source\' -\> \'$dest\'

if [ ! -d "$dest" ]; then
  mkdir "$dest"
fi

files_up2date=0
files_total=0

for i in $files; do
 DiffTime $i
done

if [ $files_up2date -ne $files_total ]; then
 echo Up to date files $files_up2date/$files_total
 for i in $files; do
  Deploy $i
 done
else
 echo All files up to date
fi
