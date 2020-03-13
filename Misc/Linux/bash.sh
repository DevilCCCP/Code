#!/bin/bash

ProjAbbr="telo"
ThisDir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
HeadDir="$ThisDir/.."
BinDir="$ThisDir/bin/release"
PackageDir=

#sudo systemctl start ${ProjAbbr}_upd
sudo systemctl start ${ProjAbbr}_srvd


#!/bin/bash

ProjAbbr="telo"
ThisDir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
HeadDir="$ThisDir/.."
BinDir="$ThisDir/bin/release"
PackageDir=

echo "starting ${ProjAbbr}_upd"
sudo systemctl stop "${ProjAbbr}_upd"
echo "done $?"
echo "starting ${ProjAbbr}_srvd"
sudo systemctl stop "${ProjAbbr}_srvd"
echo "done $?"


#!/bin/bash

ProjAbbr="telo"
ThisDir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
HeadDir="$ThisDir"
BinDir="$ThisDir/bin/release/"
PackageDir=

sudo rsync -r --exclude='Var' --exclude='.lib' ${BinDir} ${PackageDir}/opt/${ProjAbbr}
sudo chown -R ${ProjAbbr}:${ProjAbbr} ${PackageDir}/opt/${ProjAbbr}


