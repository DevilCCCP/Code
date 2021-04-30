#!/bin/bash

source $ScriptDir/Common.sh


if [ ! -d "${BatchDir}" ]; then
  echo "BatchDir not specified or doesn't exists"
  exit 1
fi

if [ ! -f "${BatchDir}"/"${ProjAbbr}"_batch_private.pem ]; then
  echo "${ProjAbbr}_batch_private.pem doesn't exists"
  exit 2
fi

for fileName in "${BatchDir}"/*.sh; do
  if [ -f ${fileName} ]; then
    baseName=${fileName%.*}
    openssl dgst -sha256 -sign "${BatchDir}"/"${ProjAbbr}"_batch_private.pem -out ${baseName}.sgn ${baseName}.sh
  fi
done

for fileName in "${BatchDir}"/*.7z; do
  if [ -f ${fileName} ]; then
    baseName=${fileName%.*}
    openssl dgst -sha256 -sign "${BatchDir}"/"${ProjAbbr}"_batch_private.pem -out ${baseName}.sgn ${baseName}.7z
  fi
done
