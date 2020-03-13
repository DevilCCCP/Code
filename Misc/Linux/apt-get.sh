#!/bin/bash

function fail {
  echo Last command fail, aborting
  exit
}

echo update repositories
apt-get update || fail
apt-get upgrade || fail

echo install enviroment
apt-get install mc || fail
apt-get install winbind libnss-winbind || fail
apt-get install subversion || fail

echo install build
apt-get install qt5-default libqt5sql5-psql make g++ libssl-dev || fail

echo install video libraries
apt-get install libavcodec-dev libswscale-dev libavformat-dev libavdevice-dev || fail
apt-get install liblivemedia-dev || fail

echo install db
apt-get install postgresql
