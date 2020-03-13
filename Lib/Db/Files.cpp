#include <QSqlQuery>
#include <QVariant>

#include "Files.h"


QString FilesTable::TableName()
{
  return "files";
}

QString FilesTable::Columns()
{
  return "_object,name,mime_type,data";
}

bool FilesTable::OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItemT<qint64> >& item)
{
  Files* it;
  item.reset(it = new Files());
  it->ObjectId = q->value(index++).value<int>();
  it->Name = q->value(index++).value<QString>();
  it->MimeType = q->value(index++).value<QString>();
  it->Data = q->value(index++).value<QByteArray>();
  return true;
}

bool FilesTable::OnRowWrite(QSqlQueryS& q, int& index, const DbItemT<qint64>& item)
{
  const Files& it = static_cast<const Files&>(item);
  q->bindValue(index++, Db::ToKey(it.ObjectId));
  q->bindValue(index++, it.Name);
  q->bindValue(index++, it.MimeType);
  q->bindValue(index++, it.Data);
  return true;
}


FilesTable::FilesTable(const Db& _Db)
  : DbTableT<qint64, Files>(_Db)
{
}
