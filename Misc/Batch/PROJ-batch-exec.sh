#!/bin/bash

export ShareDir=/usr/share/__ABBR___batch
export TmpDir=/tmp/__ABBR__batch
export HomeDir=/root/__ABBR___batch
export WorkDir=/tmp/__ABBR___batch_exec
export KeyFile=${HomeDir}/__ABBR___batch_public.pem
export Version=$(<"${ShareDir}"/Ver)
export ScriptFile=__ABBR__-batch-"${Version}".sh
export ArchFile=__ABBR__-batch-"${Version}".7z
export SignFile=__ABBR__-batch-"${Version}".sgn


if [ ! -f "${TmpDir}"/"${SignFile}" ]; then
  echo "Nothing to do"
  exit 0
fi

if [ -d "${WorkDir}" ]; then
  rm -rf "${WorkDir}" || exit 1
fi
mkdir ${WorkDir} || exit 2

if [ -f "${TmpDir}"/"${ScriptFile}" ]; then
  echo "Found script file"
  cp "${TmpDir}"/"${ScriptFile}" "${WorkDir}"/"${ScriptFile}"
  openssl dgst -sha256 -verify "${KeyFile}" -signature "${TmpDir}"/"${SignFile}" "${WorkDir}"/"${ScriptFile}" || exit 2
  echo "Script verified"
  bash -c "${WorkDir}"/"${ScriptFile}" || exit 3
  echo "Script done"
  exit 0
fi

if [ -f "${TmpDir}"/"${ArchFile}" ]; then
  echo "Found arch file"
  cp "${TmpDir}"/"${ArchFile}" "${WorkDir}"/"${ArchFile}"
  openssl dgst -sha256 -verify "${KeyFile}" -signature "${TmpDir}"/"${SignFile}" "${WorkDir}"/"${ArchFile}" || exit 2
  echo "Arch verified"
  7z x -o"${WorkDir}" "${WorkDir}"/"${ArchFile}" || exit 4
  [ -f "${WorkDir}"/"${ScriptFile}" ] || exit 5
  bash -c "${WorkDir}"/"${ScriptFile}" || exit 3
  echo "Script done"
  exit 0
fi
