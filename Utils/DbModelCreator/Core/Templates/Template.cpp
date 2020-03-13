#include <QSqlQuery>
#include <QVariant>

#include "ClassT.h"


bool ClassT::Equals(const DbItemT<TypeT>& other)
{
  const ClassT& vs = static_cast<const ClassT&>(other);
  return DbItemT<TypeT>::Equals(other)EQUALITY;
}

qint64 ClassT::Key(int index) const
{
  switch (index) {
SWITCH_CASES_KEY
  }
  return 0;
}

void ClassT::SetKey(int index, qint64 id)
{
  Q_UNUSED(id);

  switch (index) {
SWITCH_CASES_SKEY
  }
}

QString ClassT::Text(int column) const
{
  switch (column) {
SWITCH_CASES_GET
  }
  return QString();
}

bool ClassT::SetText(int column, const QString& text)
{
  Q_UNUSED(text);

  switch (column) {
SWITCH_CASES_SET
  }
  return false;
}

QVariant ClassT::Data(int column) const
{
  switch (column) {
SWITCH_CASES_DATA
  }
  return QVariant();
}

bool ClassT::SetData(int column, const QVariant& data)
{
  Q_UNUSED(data);

  switch (column) {
SWITCH_CASES_SDATA
  }
  return false;
}

QString ClassTTable::TableName()
{
  return "TABLE_NAME";
}

QString ClassTTable::Columns()
{
  return "COLUMNS";
}

bool ClassTTable::OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItemT<TypeT> >& item)
{
  ClassT* it;
  item.reset(it = new ClassT());ROW_READ
  return true;
}

bool ClassTTable::OnRowWrite(QSqlQueryS& q, int& index, const DbItemT<TypeT>& item)
{
  const ClassT& it = static_cast<const ClassT&>(item);ROW_WRITE
  return true;
}

bool ClassTTable::CreateDefaultItem(QSharedPointer<DbItemT<TypeT> >& item)
{
  item.reset(new ClassT());
  QSharedPointer<ClassT> it = qSharedPointerCast<ClassT>(item);
NAME_CPP1  if (!Insert(it)) {
    return false;
  }
  return true;
}

void ClassTTable::NewDefaultItem(QSharedPointer<DbItemT<TypeT> >& item)
{
  item.reset(new ClassT());
}

QStringList ClassTTable::Headers() const
{
  return QStringList()COLUMN_LIST;
}

QString ClassTTable::Icon() const
{
  return QString(":/Icons/ClassT.png");
}


ClassTTable::ClassTTable(const Db& _Db)
  : DbTableT<TypeT, ClassT>(_Db)NAME_CPP2
{
}
