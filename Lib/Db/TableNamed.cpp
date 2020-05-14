#include "TableNamed.h"
#include "Db.h"


bool NamedItem::Equals(const TableItem &other) const
{
  const NamedItem& other_ = dynamic_cast<const NamedItem&>(other);
  return Id == other_.Id && Name == other_.Name && Descr == other_.Descr;
}

NamedItem::NamedItem()
{ }

NamedItem::NamedItem(int _Id, const QString& _Name, const QString& _Descr)
  : TableItem(_Id), Name(_Name), Descr(_Descr)
{ }

NamedItem::~NamedItem()
{ }

bool TableNamed::OnRowFillItem(QueryS &q, TableItemS &unit)
{
  NamedItem* namedItem = static_cast<NamedItem*>(unit.data());
  namedItem->Name = q->value(1).toString();
  namedItem->Descr = q->value(2).toString();
  return true;
}

void TableNamed::CreateIndexes()
{
  if (mNameIndexed) {
    CreateNameIndex();
  }
}

void TableNamed::ClearIndexes()
{
  mNameIndex.clear();
}

void TableNamed::SelectOneByName(const QString &name, QString &queryText)
{
  queryText = QString("%1 WHERE name = '%2'").arg(Select()).arg(name);
}

void TableNamed::Clear()
{
  mNameIndex.clear();
  Table::Clear();
}

void TableNamed::CreateNameIndex()
{
  for (auto itr = mItems.begin(); itr != mItems.end(); itr++) {
    const NamedItem* item = static_cast<const NamedItem*>(itr.value().data());
    mNameIndex.insertMulti(item->Name, item);
  }
}

const NamedItem* TableNamed::GetItemByName(const QString &name)
{
  auto itr = mNameIndex.find(name);
  if (itr != mNameIndex.end()) {
    auto itrn = itr + 1;
    if (itrn == mNameIndex.end() || itrn.key() != name) {
      return itr.value();
    } else {
      return nullptr;
    }
  }

  QString queryText;
  SelectOneByName(name, queryText);

  auto q = mDb.MakeQuery();
  q->prepare(queryText);
  if (mDb.ExecuteQuery(q)) {
    while (q->next()) {
      TableItemS unit;
      if (OnRowLoad(q, unit)) {
        mItems[unit->Id] = unit;
        NamedItem* unitData = static_cast<NamedItem*>(unit.data());
        mNameIndex[name] = unitData;
        return unitData;
      }
    }
  }
  return nullptr;
}

bool TableNamed::GetTypeIdByName(const QString& name, int& id)
{
  if (const NamedItem* item = GetItemByName(name)) {
    id = item->Id;
    return true;
  }
  return false;
}


TableNamed::TableNamed(const Db& _Db, bool _NameIndexed)
  : Table(_Db), mNameIndexed(_NameIndexed)
{
}

TableNamed::~TableNamed()
{ }
