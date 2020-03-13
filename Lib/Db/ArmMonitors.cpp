#include <QSqlQuery>
#include <QVariant>

#include "ArmMonitors.h"


QString ArmMonitorsTable::TableName()
{
  return "arm_monitors";
}

QString ArmMonitorsTable::Columns()
{
  return "_object,name,descr,num,width,height,size,used";
}

bool ArmMonitorsTable::OnRowRead(QSqlQueryS& q, int& index, QSharedPointer<DbItem>& item)
{
  ArmMonitors* am;
  item.reset(am = new ArmMonitors());
  am->Object = q->value(index++).toInt();
  am->Name = q->value(index++).toString();
  am->Descr = q->value(index++).toString();
  am->Num = q->value(index++).toInt();
  am->Width = q->value(index++).toInt();
  am->Height = q->value(index++).toInt();
  QStringList point = q->value(index++).toString().split(QRegExp("\\D"), QString::SkipEmptyParts);
  if (point.size() == 2) {
    am->Size = QPoint(point[0].toInt(), point[1].toInt());
  }
  am->Used = q->value(index++).toBool();
  return true;
}

bool ArmMonitorsTable::OnRowWrite(QSqlQueryS& q, int& index, const DbItem& item)
{
  const ArmMonitors& am = static_cast<const ArmMonitors&>(item);
  q->bindValue(index++, am.Object);
  q->bindValue(index++, am.Name);
  q->bindValue(index++, am.Descr);
  q->bindValue(index++, am.Num);
  q->bindValue(index++, am.Width);
  q->bindValue(index++, am.Height);
  q->bindValue(index++, QString("(%1,%2)").arg(am.Size.x()).arg(am.Size.y()));
  q->bindValue(index++, am.Used);
  return true;
}


ArmMonitorsTable::ArmMonitorsTable(const Db& _Db)
  : DbTableT<int, ArmMonitors>(_Db)
{
}
