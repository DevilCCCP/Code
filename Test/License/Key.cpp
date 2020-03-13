#include <QFile>
#include <QCoreApplication>

#include <Lib/Include/License.h>
#include <Lib/Log/Log.h>


int SaveKey()
{
  QByteArray sn = GetSn(false);
  Log.Info(QString("S/n: %1").arg(QString::fromUtf8(sn)));

  QFile file(QCoreApplication::applicationDirPath().append("/key.ini"));
  if (!file.open(QIODevice::WriteOnly)) {
    Log.Fatal("Can't create key file");
    return -1;
  }

  for (int i = 0; i <= kLcCount; i++) {
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(sn);
    if (i) {
      hash.addData((char*)&i, 4);
    }
//    Log.Info(QString("Line %1: %2").arg(i, 2).arg(QString(hash.result().toHex())));
    if (i) {
      file.write(hash.result().toHex().prepend(':'));
    } else {
      file.write(hash.result().toHex());
    }
  }

//  const char* vars[] = { "Caption"
//                         , "Description"
//                         , "IdentifyingNumber"
//                         , "Name"
//                         , "SKUNumber"
//                         , "UUID"
//                         , "Vendor"
//                         , "Version"
//                         ,""
//                       };
//  for (int i = 0; *vars[i]; i++) {
//    char s[80];
//    int j = 0;
//    for (; vars[i][j]; j++) {
//      s[j] = (char)(vars[i][j] - j);
//    }
//    s[j] = 0;
//    Log.Info(s);
//  }
  file.close();
  file.setPermissions(QFile::ReadOwner | QFile::ReadUser | QFile::ReadGroup | QFile::ReadOther);
  return 0;
}

