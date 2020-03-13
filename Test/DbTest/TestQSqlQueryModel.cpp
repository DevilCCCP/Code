#include "TestQSqlQueryModel.h"


bool TestQSqlQueryModel::Prepare()
{
  mDb.reset(new Db());
  if (!mDb->OpenDefault() || !mDb->Connect()) {
    return false;
  }
  return true;
}

bool TestQSqlQueryModel::Do()
{
  auto q = mDb->MakeQuery(false);

  QString columns = "data0";
  for (int i = 1; i < 16; i++) {
    columns.append(QString(",data%1").arg(i));
  }
  QString query = QString("SELECT %2 FROM %1;").arg(mTable).arg(columns);

  q->prepare(query);
  q->exec();
  mModel.setQuery(*q);

  int result = 0;
  for (int i = 0; i < mModel.rowCount(); ++i) {
    for (int j = 0; j < 16; j++) {
      result += mModel.record(i).value(j).toString().size();
    }
  }

  printf("%i\n", result);
  return true;
}
