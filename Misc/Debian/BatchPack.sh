#!/bin/bash


export BatchDir="$HeadDir"/Misc/Batch

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

ProjVersion='1.0'
PackArch=$(dpkg --print-architecture)
PackageName="${ProjAbbr}"_batch
Package="${PackageName}"_"${ProjVersion}"-1_"${PackArch}"
PackagePriv="${PackageName}"_private.pem
PackagePub="${PackageName}"_public.pem
PackageDir="${HOME}"/"${Package}"


printf "Preparing package ${col_green}${Package}${col_normal}\n"

if [ -d "${PackageDir}" ]; then
  Clear
fi
mkdir "${PackageDir}" || Fail

printf "Creating key pair ."
openssl genrsa -out "${HOME}"/"${PackagePriv}" 1024 > /dev/null 2>&1 || Fail
printf "."
mkdir ${PackageDir}/root || Fail
mkdir ${PackageDir}/root/"${ProjAbbr}"_batch || Fail
printf ". "
openssl rsa -in "${HOME}"/"${PackagePriv}" -out "${PackageDir}"/root/"${ProjAbbr}"_batch/"${PackagePub}" -pubout -outform PEM > /dev/null 2>&1 || Fail
Ok

printf "Creating pack control ... "
mkdir ${PackageDir}/DEBIAN || Fail
cat ${BatchDir}/control | sed "s/__VERSION__/${ProjVersion}/g" | sed "s/__ARCH__/${PackArch}/g" | sed "s/__NAME__/${ProjName}/g" > ${PackageDir}/DEBIAN/control || Fail
Ok

printf 'Copy source ... '
mkdir ${PackageDir}/etc || Fail
mkdir ${PackageDir}/etc/systemd || Fail
mkdir ${PackageDir}/etc/systemd/system || Fail
for filePath in ${BatchDir}/PROJ_batch*; do
  fileName=${filePath##*/}
  fileName=${fileName/PROJ/$ProjAbbr}
  cat ${filePath} | sed "s~__ABBR__~${ProjAbbr}~g" > ${PackageDir}/etc/systemd/system/${fileName} || Fail
done

mkdir ${PackageDir}/usr || Fail
mkdir ${PackageDir}/usr/libexec || Fail
for filePath in ${BatchDir}/PROJ-batch*; do
  fileName=${filePath##*/}
  fileName=${fileName/PROJ/$ProjAbbr}
  cat ${filePath} | sed "s~__ABBR__~${ProjAbbr}~g" | sed "s~__URI__~"${ProjUri}"~g" > ${PackageDir}/usr/libexec/${fileName} || Fail
  chmod +x ${PackageDir}/usr/libexec/${fileName} || Fail
done

mkdir ${PackageDir}/usr/share || Fail
mkdir ${PackageDir}/usr/share/"${ProjAbbr}"_batch || Fail
echo 0 > ${PackageDir}/usr/share/"${ProjAbbr}"_batch/Ver
Ok

printf 'Creating inst scripts ... '
for f in postinst; do
  if [ -e ${HeadDir}/Misc/Debian/${f}_batch ]; then
    cat ${HeadDir}/Misc/Debian/${f}_batch | sed "s/__ABBR__/${ProjAbbr}/g" > ${PackageDir}/DEBIAN/${f} || Fail
  else
    Fail
  fi
  echo exit 0 >> ${PackageDir}/DEBIAN/${f} || Fail
  chmod +x ${PackageDir}/DEBIAN/${f} || Fail
done
Ok

printf 'Creating etc ... '
mkdir ${PackageDir}/etc/sudoers.d || Fail
printf "%%${ProjAbbr} ALL = (root) NOPASSWD:  /usr/sbin/service ${ProjAbbr}_batch start\n%%${ProjAbbr}loader ALL = (root) NOPASSWD:  /usr/sbin/service ${ProjAbbr}_batch start\n" > ${PackageDir}/etc/sudoers.d/${ProjAbbr}_batch || Fail
chmod 0440 ${PackageDir}/etc/sudoers.d/${ProjAbbr}_batch || Fail
Ok

printf 'Creating md5 ... '
md5deep -r "${PackageDir}"/etc "${PackageDir}"/root "${PackageDir}"/usr > "${PackageDir}"/DEBIAN/md5sums || hashdeep -r "${PackageDir}"/etc "${PackageDir}"/root "${PackageDir}"/usr > "${PackageDir}"/DEBIAN/md5sums || Fail
Ok

printf 'Packing ... '
cd ${HOME} || Fail
fakeroot dpkg-deb --build $Package > /dev/null || Fail
Ok

Clear
exit 0
