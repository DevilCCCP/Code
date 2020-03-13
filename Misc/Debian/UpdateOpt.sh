#!/bin/bash

/opt/$ProjAbbr/${ProjAbbr}_srvd.exe stop || Fail
/opt/$ProjAbbr/${ProjAbbr}_upd.exe stop || Fail
sleep 3s
#git pull
#./\!rebuild.sh
sudo rsync "$ProjDir"/bin/release/*.exe /opt/$ProjAbbr
sudo chown $ProjAbbr:$ProjAbbr -R /opt/$ProjAbbr
/opt/$ProjAbbr/${ProjAbbr}_srvd.exe start
/opt/$ProjAbbr/${ProjAbbr}_upd.exe start
sleep 1s
