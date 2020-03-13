#!/bin/bash

PWD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
DestDir=$1
DestFile=$2
RootDir=$PWD/../..
LocalDir=$RootDir/Local


echo Deploy versions
svnversion ../.. >"$DestDir/$DestFile.ver"
svnversion ../.. >./$DestFile.ver

/bin/bash "$PWD/deploy.sh" "$LocalDir" "$DestDir" Version.ini
