#!/bin/bash

export ShareDir=/usr/share/__ABBR___batch
export TmpDir=/tmp/__ABBR__batch
export Version=$(<"${ShareDir}"/Ver)
export List=batch_list"${Version}".txt


if [ -d "${TmpDir}" ]; then
  rm -rf "${TmpDir}" || exit 1
fi
mkdir ${TmpDir} || exit 2
chmod g+w ${TmpDir}

for i in {1..30}; do
  wget __URI__/"${List}" -P "${TmpDir}"
  if [ $? -eq 0 ]; then
    break
  else
    sleep 1
  fi
done

chmod -R g+w ${TmpDir}

if [ ! -f "${TmpDir}"/"${List}" ]; then
  exit 3
fi

if [ ! -s "${TmpDir}"/"${List}" ]; then
  echo Batch script list is empty
  exit 0
fi

for i in {1..30}; do
  wget -i "${TmpDir}"/"${List}" -P "${TmpDir}"
  if [ $? -eq 0 ]; then
    break
  else
    sleep 1
  fi
done

chmod -R g+w ${TmpDir}

sudo /usr/sbin/service __ABBR___batch start
