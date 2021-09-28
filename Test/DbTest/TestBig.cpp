#include <QSqlQuery>
#include <QVariant>

#include "TestBig.h"


bool TestBig::Equals(const DbItemT<int>& other) const
{
  const TestBig& vs = static_cast<const TestBig&>(other);
  return DbItemT<int>::Equals(other) && std::equal(BigData, BigData + 16, vs.BigData);
}

qint64 TestBig::Key(int index) const
{
  switch (index) {

  }
  return 0;
}

void TestBig::SetKey(int index, qint64 id)
{
  Q_UNUSED(id);

  switch (index) {

  }
}

QString TestBig::Text(int column) const
{
  if (column >= 0 && column < 16) {
    return BigData[column];
  }
  return QString();
}

bool TestBig::SetText(int column, const QString& text)
{
  Q_UNUSED(text);

  if (column >= 0 && column < 16) {
    BigData[column] = text;
    return true;
  }
  return false;
}

QVariant TestBig::Data(int column) const
{
  if (column >= 0 && column < 16) {
    return QVariant(BigData[column]);
  }
  return QVariant();
}

bool TestBig::SetData(int column, const QVariant& data)
{
  Q_UNUSED(data);

  if (column >= 0 && column < 16) {
    BigData[column] = data.toString();
    return true;
  }
  return false;
}

QString TestBigTable::TableName()
{
  return "test_big";
}

QString TestBigTable::Columns()
{
  return "big_data";
}

bool TestBigTable::OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItemT<int> >& item)
{
  TestBig* it;
  item.reset(it = new TestBig());
  for (int i = 0; i < 16; i++) {
    it->BigData[i] = q->value(index++).value<QString>();
  }
  return true;
}

bool TestBigTable::OnRowWrite(QSqlQueryS& q, int& index, const DbItemT<int>& item)
{
  const TestBig& it = static_cast<const TestBig&>(item);
  for (int i = 0; i < 16; i++) {
    q->bindValue(index++, it.BigData[i]);
  }
  return true;
}

bool TestBigTable::CreateDefaultItem(QSharedPointer<DbItemT<int> >& item)
{
  item.reset(new TestBig());
  QSharedPointer<TestBig> it = qSharedPointerCast<TestBig>(item);
  if (!Insert(it)) {
    return false;
  }
  return true;
}

void TestBigTable::NewDefaultItem(QSharedPointer<DbItemT<int> >& item)
{
  item.reset(new TestBig());
}

QStringList TestBigTable::Headers() const
{
  return QStringList() << "Big data";
}

QString TestBigTable::Icon() const
{
  return QString(":/Icons/TestBig.png");
}


TestBigTable::TestBigTable(const Db& _Db)
  : DbTableT<int, TestBig>(_Db)
{
}
