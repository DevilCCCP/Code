#!/bin/bash


source $ScriptDir/Common.sh

function Clear {
  printf 'Clear ... '
  rm -rf $PackageDir || Fail
  Ok
}

function FailClear {
  echo 'Fail'
  Clear
  exit 1
}

if [[ $HasCluster && $HasServer ]]; then
  HasAll=true
  PackExt=_all
  PackageName="${ProjAbbr}_all"
elif [[ !($HasCluster) && !($HasServer) ]]; then
  HasAll=true
  PackExt=_all
  HasCluster=true
  HasServer=true
  PackageName="${ProjAbbr}"
elif [[ $HasCluster ]]; then
  PackExt=_cst
  PackageName="${ProjAbbr}_cst"
elif [[ $HasServer ]]; then
  PackExt=_srv
  PackageName="${ProjAbbr}_srv"
fi

BinDir=${ProjDir}/bin/release/
ProjVersion=$(awk -F "=" '/Ver/ {print $2}' ${BinDir}/Version.ini | sed 's/\r$//')
PackArch=$(dpkg --print-architecture)
Package="${PackageName}_${ProjVersion}-1_${PackArch}"
PackageDir="${HOME}/${Package}"


printf "Preparing package ${col_green}${Package}${col_normal}\n"

if [ -d "${PackageDir}" ]; then
  Clear
fi

printf 'Copy source ... '
mkdir $PackageDir || Fail
mkdir ${PackageDir}/opt || Fail
mkdir ${PackageDir}/opt/${ProjAbbr} || Fail
rsync -r --exclude='Var' --exclude='.lib' ${BinDir} ${PackageDir}/opt/${ProjAbbr}
Ok

os1=$(lsb_release -is)
os2=$(lsb_release -rs)
printf "Creating pack control (OS: $os1.$os2) ... "

mkdir ${PackageDir}/DEBIAN || Fail
if [ "$HasServer" ]; then
  if [ -f ${ProjDir}/Local/control.$os1.$os2 ]; then
    cat ${ProjDir}/Local/control.$os1.$os2 | sed "s/__VERSION__/${ProjVersion}/g" | sed "s/__ARCH__/${PackArch}/g" > ${PackageDir}/DEBIAN/control || Fail
  else
    cat ${ProjDir}/Local/control | sed "s/__VERSION__/${ProjVersion}/g" | sed "s/__ARCH__/${PackArch}/g" > ${PackageDir}/DEBIAN/control || Fail
  fi
else
  if [ -f ${ProjDir}/Local/control.$os1.$os2.cluster ]; then
    cat ${ProjDir}/Local/control.$os1.$os2.cluster | sed "s/__VERSION__/${ProjVersion}/g" | sed "s/__ARCH__/${PackArch}/g" > ${PackageDir}/DEBIAN/control || Fail
  else
    cat ${ProjDir}/Local/control.cluster | sed "s/__VERSION__/${ProjVersion}/g" | sed "s/__ARCH__/${PackArch}/g" > ${PackageDir}/DEBIAN/control || Fail
  fi
fi
cat ${ProjDir}/Local/dirs | sed "s/__ABBR__/${ProjAbbr}/g" > ${PackageDir}/DEBIAN/dirs || Fail
Ok

printf 'Creating inst scripts ... '
for f in preinst postinst prerm postrm; do
  if [ -e ${HeadDir}/Misc/Debian/${f} ]; then
    cat ${HeadDir}/Misc/Debian/${f} | sed "s/__ABBR__/${ProjAbbr}/g" > ${PackageDir}/DEBIAN/${f} || Fail
  elif [ -e ${HeadDir}/Misc/Debian/${f}${PackExt} ]; then
    cat ${HeadDir}/Misc/Debian/${f}${PackExt} | sed "s/__ABBR__/${ProjAbbr}/g" > ${PackageDir}/DEBIAN/${f} || Fail
  else
    Fail
  fi
  if [ -e ${ProjDir}/Local/${f} ]; then
    cat ${ProjDir}/Local/${f} | sed "s/__ABBR__/${ProjAbbr}/g" >> ${PackageDir}/DEBIAN/${f} || Fail
  fi
  echo exit 0 >> ${PackageDir}/DEBIAN/${f} || Fail
  chmod +x ${PackageDir}/DEBIAN/${f} || Fail
done
cat ${BinDir}/Updates/instd.service | sed "s/__ABBR__/${ProjAbbr}/g" > ${PackageDir}/opt/${ProjAbbr}/Updates/instd.service || Fail
Ok

if [ "$HasCluster" ];then
printf 'Creating postgres scripts ... '
mkdir ${PackageDir}/opt/${ProjAbbr}/Install || Fail
for f in CreateUsers.sql CreateGroups.sql CreateDb.sql Install.sql connection.ini; do
  cat ${ProjDir}/Install/Db/${f} | sed "s/__ABBR__/${ProjAbbr}/g" > ${PackageDir}/opt/${ProjAbbr}/Install/${f} || Fail
  test -e ${PackageDir}/opt/${ProjAbbr}/Install/${f} || Fail
done
else
printf 'Creating postgres connection ... '
mkdir ${PackageDir}/opt/${ProjAbbr}/Install || Fail
for f in Update.sql connection.ini; do
  cat ${ProjDir}/Install/Db/${f} | sed "s/__ABBR__/${ProjAbbr}/g" > ${PackageDir}/opt/${ProjAbbr}/Install/${f} || Fail
  test -e ${PackageDir}/opt/${ProjAbbr}/Install/${f} || Fail
done
fi
Ok

printf 'Creating etc ... '
mkdir ${PackageDir}/etc/
mkdir ${PackageDir}/etc/sudoers.d/
SudoPre="%%${ProjAbbr} ALL = (root) NOPASSWD: ";
SudoPreSvc="${SudoPre} /usr/sbin/service ${ProjAbbr}_"
printf "${SudoPreSvc}srvd *\n${SudoPreSvc}upd *\n${SudoPreSvc}instd start\n${SudoPre} /bin/fuser -k /opt/${ProjAbbr}*\n" > ${PackageDir}/etc/sudoers.d/${ProjAbbr} || Fail
if [[ $HasCluster ]]; then
  printf "${SudoPre} /usr/sbin/service postgresql restart\n" > ${PackageDir}/etc/sudoers.d/${ProjAbbr}_postgres || Fail
  chmod 0440 ${PackageDir}/etc/sudoers.d/${ProjAbbr}_postgres || Fail
fi
chmod 0440 ${PackageDir}/etc/sudoers.d/${ProjAbbr} || Fail
Ok

printf 'Creating md5 ... '
md5deep -r ${PackageDir}/opt > ${PackageDir}/DEBIAN/md5sums || hashdeep -r ${PackageDir}/opt > ${PackageDir}/DEBIAN/md5sums || Fail
Ok

printf 'Packing ... '
cd ${HOME} || Fail
fakeroot dpkg-deb --build $Package > /dev/null || Fail
Ok

Clear
exit 0