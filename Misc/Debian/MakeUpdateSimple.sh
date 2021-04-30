#!/bin/bash


if [ -d $PackDir ]; then
  printf 'Remove old update pack ... '
  rm -r $PackDir || Fail
  printf 'Ok\n'
fi

/bin/bash "$ScriptDir/Build.sh" || Fatal

printf 'Prepare update pack ... '
BinDir=$ProjDir/bin/release
PackInfo=$BinDir/.info

mkdir $PackDir

if [ ! -f $ProjDir/Install/.info$ProjModule ]; then
  printf " no .info$ProjModule "
  Fail
fi
cat $ProjDir/Install/.infoDevice>$PackInfo

echo D .>>$PackInfo
echo F Version.ini>>$PackInfo
echo I ${ProjAbbr}_up.exe>>$PackInfo

printf 'Ok\n'

printf 'Create update pack ... '
$ProjDir/bin/release/${ProjAbbr}_up.exe pack $BinDir $PackDir || Fail

Ok
