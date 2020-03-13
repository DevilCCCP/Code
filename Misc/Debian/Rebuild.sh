#!/bin/bash


printf 'Remove build ... '
rm -r $HeadDir/build
printf 'Build ... '
mkdir $ProjDir/build  
qmake $HeadDir/$ProjName.pro -o $ProjDir/build || Fail
make -C $ProjDir/build -j || Fail
Ok
