#!/bin/bash


if [ -d $PackDir ]; then
  printf 'Remove old update pack ... '
  rm -r $PackDir || Fail
  printf 'Ok\n'
fi

printf 'Build ... '
if [ ! -d $ProjDir/build ]; then
  mkdir $ProjDir/build
fi
qmake $HeadDir/$ProjName.pro -o $ProjDir/build || Fail
make -C $ProjDir/build -j2 >> /dev/null || Fail
printf 'Ok\n'

printf 'Prepare update pack ... '
BinDir=$ProjDir/bin/release
PackInfo=$BinDir/.info

mkdir $PackDir
echo 'D .'>$PackInfo
for file in $BinDir/${ProjAbbr}_*.exe; do
  echo X ${file##*/}>>$PackInfo
done

for file in $BinDir/*.sh; do
  echo E ${file##*/}>>$PackInfo
done

if [ -d $PackScript ]; then
echo 'D Scripts'>>$PackInfo
if [ ! -d $BinDir/Scripts ]; then
  mkdir $BinDir/Scripts
fi
for i1 in $(seq 0 9); do
 for i2 in $(seq 0 9); do
  for i3 in $(seq 0 9); do
   upFile=Up${i1}${i2}${i3}.sh
   if [ -f $PackScript/$upFile ]; then
     echo S ${upFile##*/}>>$PackInfo
     cp $PackScript/$upFile $BinDir/Scripts/$upFile
   fi
  done
 done
done
fi

PackSql=$ProjDir/Db/Update
echo 'D Updates'>>$PackInfo
for i1 in $(seq 0 9); do
 for i2 in $(seq 0 9); do
  for i3 in $(seq 0 9); do
   upFile=Up${i1}${i2}${i3}.sql
   if [ -f $PackSql/$upFile ]; then
     echo Q ${upFile##*/}>>$PackInfo
     cp $PackSql/$upFile $BinDir/Updates/$upFile
   fi
  done
 done
done

if [ -f $ProjDir/Install/.info ]; then
  cat $ProjDir/Install/.info>>$PackInfo
fi

echo D .>>$PackInfo
echo F Version.ini>>$PackInfo
echo I ${ProjAbbr}_inst.exe>>$PackInfo

printf 'Ok\n'

printf 'Create update pack ... '
$ProjDir/bin/release/${ProjAbbr}_inst.exe pack $BinDir $PackDir || Fail

Ok
