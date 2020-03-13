#include <Lib/Log/Log.h>
#include <Lib/Db/ObjectType.h>

#include "TreeSchema.h"


const int kMinIdDefault = 100;

void TreeSchema::Reload(const ObjectTypeTableS& objectTypeTable)
{
  mTypeItemMap.clear();
  for (int i = 0; i < mSchema.size(); i++) {
    const TreeRootItem& item = mSchema.at(i);
    if (item.getType() == "tmp") {
      mTypeItemMap[0] = i;
    } else if (const NamedItem* itemType = objectTypeTable->GetItemByName(item.getType())) {
      mTypeItemMap.insertMulti(itemType->Id, i);
    } else {
      Log.Warning(QString("Schema type not found (type: '%1')").arg(item.getType()));
    }
  }
  mInit = true;
}

bool TreeSchema::GetIndex(int objType, int& index)
{
  bool fitLast = index < 0;
  for (auto itr = mTypeItemMap.find(objType); itr != mTypeItemMap.end() && itr.key() == objType; itr++) {
    if (fitLast) {
      index = itr.value();
      return true;
    } else if (index == itr.value()) {
      fitLast = true;
    }
  }
  return false;
}


TreeSchema::TreeSchema()
  : mMinId(kMinIdDefault), mInit(false)
{
}

