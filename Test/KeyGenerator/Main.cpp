#include <QCoreApplication>
#include <QDateTime>
#include <QUuid>
#include <QDir>
#include <QFile>

#include <Lib/Crypto/InnerCrypt.h>


QFile gLogFile;

QString RandPass()
{

#if QT_VERSION >= QT_VERSION_CHECK(5, 11,0)
  return QString("%1-%2").arg(QUuid::createUuid().toString(QUuid::WithoutBraces), QUuid::createUuid().toString(QUuid::WithoutBraces));
#else
  return QString("%1-%2").arg(QUuid::createUuid().toString().mid(1, 36), QUuid::createUuid().toString().mid(1, 36));
#endif
}

void WriteFile(const char* filename, const QString& text)
{
  QFile file(qApp->applicationDirPath() + "/" + filename);
  if (file.open(QFile::WriteOnly)) {
    QByteArray data = text.toUtf8();
    if (file.write(data) == data.size()) {
      gLogFile.write("write ok\n");
    } else {
      gLogFile.write("write not all\n");
    }
  } else {
    gLogFile.write("write open fail\n");
  }
}

int Decrypt(const QString& path)
{
  printf("Decrypting file %s\n", path.toUtf8().constData());
  QFile conFile(path);
  if (!conFile.open(QFile::ReadOnly)) {
    printf("Open file fail\n");
    return -1;
  }
  QString text;
  while (!conFile.atEnd()) {
    text = QString::fromUtf8(conFile.readLine());
    if (!text.startsWith('#')) {
      break;
    }
  }
  QStringList parts = text.split("::");
  if (parts.size() != 5) {
    printf("Bad file format\n");
    return -2;
  }
  QString pass = parts[4];
  InnerCrypt cryptor;
  QString dpass = QString::fromLatin1(cryptor.Decrypt(QByteArray::fromBase64(pass.toLatin1())));
  parts[4] = dpass;
  QString dtext = parts.join("::");

  QFileInfo fi(conFile);
  QFile dconFile(fi.absoluteDir().absoluteFilePath(
                   QString("d%1.%2").arg(fi.completeBaseName()).arg(fi.suffix())));
  if (!dconFile.open(QFile::WriteOnly)) {
    printf("Open decrypted file fail\n");
    return -3;
  }
  QByteArray data = dtext.toUtf8();
  dconFile.write(data);
  return 0;
}

int main(int argc, char* argv[])
{
  QCoreApplication q(argc, argv);

  if (QCoreApplication::arguments().size() < 4) {
    if (QCoreApplication::arguments().size() > 1) {
      QString command = QCoreApplication::arguments().at(1);
      if (command == "decrypt" && QCoreApplication::arguments().size() > 2) {
        QString path = QCoreApplication::arguments().at(2);
        return Decrypt(path);
      }
    }
    printf("Usage: <exe> prefix server port [su_pass] [user_pass]\n");
    return -1;
  }
  gLogFile.setFileName(qApp->applicationDirPath() + "/Install.log");
  if (!gLogFile.open(QFile::Append)) {
    gLogFile.open(stdout, QIODevice::WriteOnly | QIODevice::Text);
  }
  gLogFile.write(QCoreApplication::arguments().join(' ').toLocal8Bit() + "\n");

  QString prefix = QCoreApplication::arguments().at(1);
  QString server = QCoreApplication::arguments().at(2);
  QString port   = QCoreApplication::arguments().at(3);
  QString suPass = QCoreApplication::arguments().size() > 4
      ? QCoreApplication::arguments().at(4): RandPass();
  QString usrPass = QCoreApplication::arguments().size() > 5
      ? QCoreApplication::arguments().at(5): RandPass();

  WriteFile("pwd_su", suPass);
  WriteFile("pwd_user", usrPass);
  WriteFile("pwd.conf", QString("127.0.0.1:%1:*:root:%2").arg(port, suPass));
  InnerCrypt cryptor;
  QByteArray suKey = cryptor.Encrypt(suPass.toLatin1());
  QByteArray usrKey = cryptor.Encrypt(usrPass.toLatin1());

  if (cryptor.Decrypt(suKey) != suPass.toLatin1() || cryptor.Decrypt(usrKey) != usrPass.toLatin1()) {
    gLogFile.write("decrypt failed\n");
    printf("decrypt failed\n");
  }
  WriteFile("connect1.ini", QString("%1::%2::%3db::%4::%5")
            .arg(server, port, prefix, prefix + "1", usrKey.toBase64()));
  WriteFile("connect2.ini", QString("%1::%2::%3db::%4::%5")
            .arg(server, port, prefix, prefix + "2", suKey.toBase64()));

  QString createRoles = QString("CREATE ROLE %1 LOGIN\n"
                                " ENCRYPTED PASSWORD '%2'\n"
                                " NOSUPERUSER INHERIT NOCREATEDB CREATEROLE REPLICATION;\n"
                                "CREATE ROLE %3 LOGIN\n"
                                " ENCRYPTED PASSWORD '%4'\n"
                                " NOSUPERUSER INHERIT NOCREATEDB NOCREATEROLE NOREPLICATION;\n")
                                .arg(prefix + "2", suPass, prefix + "1", usrPass);

  WriteFile("create_roles.sql", createRoles);
  return 0;
}

