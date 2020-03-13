#include <QSqlQuery>
#include <QVariant>

#include "TestBig.h"


QString TestBigTable::TableName()
{
  return "test_big";
}

QString TestBigTable::Columns()
{
  QStringList columns;
  for (int i = 0; i < 16; i++) {
    columns << QString("data%1").arg(i);
  }
  return columns.join(',');
}

bool TestBigTable::OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItemB>& item)
{
  TestBig* testBig;
  item.reset(testBig = new TestBig());
  for (int i = 0; i < 16; i++) {
    testBig->mData[i] = q->value(index++).toString();
  }
  return true;
}

bool TestBigTable::OnRowWrite(QSqlQueryS& q, int& index, const DbItemB& item)
{
  const TestBig& testBig = static_cast<const TestBig&>(item);
  for (int i = 0; i < 16; i++) {
    q->bindValue(index++, testBig.mData[i]);
  }
  return true;
}

int TestBigTable::TotalSize()
{
  auto q = getDb()->MakeQuery();
  q->prepare(GetSelect(QString()) + "FROM " + TableName());
  if (!getDb()->ExecuteQuery(q)) {
    return false;
  }

  int result = 0;
  while (q->next()) {
    int index = 1;
    QSharedPointer<DbItemB> item;
    if (OnRowRead(q, index, item)) {
      const TestBig& testBig = static_cast<const TestBig&>(*item);
      for (int j = 0; j < 16; j++) {
        result += testBig.mData[j].size();
      }
    }
  }
  return result;
}


TestBigTable::TestBigTable(const DbS& _Db)
  : DbTableT<qint64, TestBig>(_Db)
{
}
