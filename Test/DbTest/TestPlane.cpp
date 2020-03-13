#include "TestPlane.h"


bool TestPlaneInsert::Prepare()
{
  mDb.reset(new Db());
  if (!mDb->OpenDefault() || !mDb->Connect()) {
    return false;
  }

  for (int i = 0; i < 100; i++) {
    QString value;
    int length = 200 + (qrand() % 1200);
    for (int j = 0; j < length; j++) {
      value.append(QChar((char)('a' + (qrand() % 20))));
    }
    mTestData.append(value);
  }
  return true;
}

bool TestPlaneInsert::Do()
{
  auto q = mDb->MakeQuery();

  QString columns = "data0";
  QString values = "?";
  for (int i = 1; i < 16; i++) {
    columns.append(QString(",data%1").arg(i));
    values.append(",?");
  }
  QString query = QString("INSERT INTO %1 (%2) VALUES (%3);").arg(mTable).arg(columns).arg(values);

  q->prepare(query);
  for (int i = 0; i < mCount; i++) {
    for (int j = 0; j < 16; j++) {
      q->bindValue(j, mTestData[(qrand() % mTestData.size())]);
    }
    mDb->ExecuteNonQuery(q);
  }
  return true;
}


bool TestPlaneSelect::Prepare()
{
  mDb.reset(new Db());
  if (!mDb->OpenDefault() || !mDb->Connect()) {
    return false;
  }

  return true;
}

bool TestPlaneSelect::Do()
{
  auto q = mDb->MakeQuery();

  QString columns = "data0";
  for (int i = 1; i < 16; i++) {
    columns.append(QString(",data%1").arg(i));
  }
  QString query = QString("SELECT %2 FROM %1;").arg(mTable).arg(columns);

  q->prepare(query);
  if (!mDb->ExecuteQuery(q)) {
    return false;
  }

  int result = 0;
  while (q->next()) {
    for (int j = 0; j < 16; j++) {
      result += q->value(j).toString().size();
    }
  }

  printf("%i\n", result);
  return true;
}
