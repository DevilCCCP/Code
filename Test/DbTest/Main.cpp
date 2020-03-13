#include <QElapsedTimer>

#include <Lib/Include/QtAppCon.h>
#include <Lib/Db/Db.h>
#include <Lib/Ctrl/CtrlManager.h>
#include <Lib/Crypto/InnerCrypt.h>

#include "Tester.h"
#include "TestPlane.h"
#include "TestBigModel.h"
#include "TestQSqlQueryModel.h"


void DecryptConnection()
{
  QFile file(QCoreApplication::applicationDirPath() + "/.connection");
  if (!file.open(QIODevice::ReadOnly)) {
    Log.Fatal(QString("No connection file '%1'").arg("/.connection"));
    return;
  }

  QTextStream in(&file);
  in.setCodec("UTF-8");
  QString text;
  do {
    text = in.readLine().trimmed();
  } while (text.startsWith('#'));
  file.close();

  QStringList info = text.split("::");
  if (info.count() < 5) {
    Log.Fatal(QString("Invalid connection file '%1'").arg("/.connection"));
    return;
  } else {
    InnerCrypt crypt;
    QByteArray pwdEnc = QByteArray::fromBase64(info[4].toLatin1());
    QByteArray pwd = crypt.Decrypt(pwdEnc);
    if (pwd.isEmpty()) {
      pwd = pwdEnc;
      Log.Warning(QString("Decrypt connection password fail (error: '%1')").arg(crypt.ErrorText()));
      return;
    }
    info[4] = QString::fromUtf8(pwd);
    QString line = info.join("::");
    QFile file(QCoreApplication::applicationDirPath() + "/.dconnection");
    if (file.open(QFile::WriteOnly)) {
      file.write(line.toUtf8());
    }
  }
}

int qmain(int argc, char* argv[])
{
  Q_UNUSED(argc);
  Q_UNUSED(argv);

  DbS db(new Db());
  if (!db->OpenDefault() || !db->Connect()) {
    return -3001;
  }
  Log.Info(QString("Prepare tables"));
  auto q = db->MakeQuery();

  //DecryptConnection();

  q->prepare("DELETE FROM test_big;");
  if (!db->ExecuteNonQuery(q)) {
    return -3002;
  }

  //q->prepare("DROP TABLE IF EXISTS test_big;");
  //if (!db->ExecuteNonQuery(q)) {
  //  return -3002;
  //}
  //q->prepare("DROP TABLE IF EXISTS test_small;");
  //if (!db->ExecuteNonQuery(q)) {
  //  return -3102;
  //}

  //QString columns;
  //for (int i = 0; i < 16; i++) {
  //  columns.append(QString(",data%1 text").arg(i));
  //}
  //q->prepare(QString("CREATE TABLE test_big (_id bigserial NOT NULL %1"
  //                   ", CONSTRAINT test_big_pkey PRIMARY KEY (_id))"
  //                   " WITH (OIDS=FALSE);").arg(columns));
  //if (!db->ExecuteNonQuery(q)) {
  //  return -3003;
  //}

//CREATE TABLE test_big (_id bigserial NOT NULL ,data0 text,data1 text,data2 text,data3 text,data4 text
//,data5 text,data6 text,data7 text,data8 text,data9 text,data10 text,data11 text,data12 text,data13 text,data14 text,data15 text
//, CONSTRAINT test_big_pkey PRIMARY KEY (_id)) WITH (OIDS=FALSE);

  //q->prepare(QString("CREATE TABLE test_small (_id bigserial NOT NULL, data0 text"
  //                   ", CONSTRAINT test_small_pkey PRIMARY KEY (_id))"
  //                   " WITH (OIDS=FALSE);").arg(columns));
  //if (!db->ExecuteNonQuery(q)) {
  //  return -3004;
  //}

//  Tester(TestS(new TestPlaneInsert("test_big", 16, 10000))).DoTest(); // 26.5 s
  Tester(TestS(new TestBigModelIns("test_big", 16, 10000))).DoTest(); // 22.9 s
//  Tester(TestS(new TestPlaneSelect("test_big", 16, 10000))).DoTest(); // 2.5-2.6 = 124302316
//  Tester(TestS(new TestQSqlQueryModel("test_big", 16, 10000))).DoTest(); // 5.5 =    124302316
  Tester(TestS(new TestBigModel("test_big", 16, 10000))).DoTest(); // 5.5 =    124302316

//  Log.Info(QString("Remove tables"));
//  q->prepare("DROP TABLE IF EXISTS test_big; DROP TABLE IF EXISTS test_small;");
//  if (!db->ExecuteNonQuery(q)) {
//    return -3099;
//  }

  return 0;
}

