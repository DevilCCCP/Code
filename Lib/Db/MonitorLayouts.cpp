#include <QSqlQuery>
#include <QVariant>

#include "MonitorLayouts.h"


QString MonitorLayoutsTable::TableName()
{
  return "arm_monitor_layouts";
}

QString MonitorLayoutsTable::Columns()
{
  return "_amonitor,place";// << "flag";
}

bool MonitorLayoutsTable::OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItem>& item)
{
  MonitorLayouts* al;
  item.reset(al = new MonitorLayouts());
  al->Monitor = q->value(index++).toInt();
  QStringList box = q->value(index++).toString().split(QRegExp("\\D"), QString::SkipEmptyParts);
  if (box.size() == 4) {
    al->Place.setRight(box[0].toInt());
    al->Place.setBottom(box[1].toInt());
    al->Place.setLeft(box[2].toInt());
    al->Place.setTop(box[3].toInt());
  }
//  al->Flag = q->value(index++).toInt();
  return true;
}

bool MonitorLayoutsTable::OnRowWrite(QSqlQueryS& q, int& index, const DbItem& item)
{
  const MonitorLayouts& al = static_cast<const MonitorLayouts&>(item);
  q->bindValue(index++, al.Monitor);
  q->bindValue(index++, QString("(%1,%2),(%3,%4)")
               .arg(al.Place.right()).arg(al.Place.bottom()).arg(al.Place.left()).arg(al.Place.top()));
//  q->bindValue(index++, al.Flag);
  return true;
}


MonitorLayoutsTable::MonitorLayoutsTable(const Db& _Db)
  : DbTableT<int, MonitorLayouts>(_Db)
{
}
