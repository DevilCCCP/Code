#!/bin/bash


printf 'Build ... '
if [ ! -d $ProjDir/build ]; then
  mkdir $ProjDir/build  
fi
qmake $HeadDir/$ProjName.pro -o $ProjDir/build || Fail
arch=$(uname -m)
core=4
if [ "$arch" == "armv7l" ]; then
 core=1
fi
printf "${arch} threads ${core} ... "
make -C $ProjDir/build -j${core} >> /dev/null || Fail
Ok
