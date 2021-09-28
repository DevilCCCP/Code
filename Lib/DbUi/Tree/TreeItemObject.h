#pragma once

#include <Lib/Db/Db.h>
#include <Lib/DbUi/Tree/TreeItemA.h>
#include <Lib/Db/ObjectType.h>


DefineClassS(TreeItemObject);
DefineClassS(ObjectModel);

class TreeItemObject: public TreeItemA
{
  ObjectModel* mCore;
  PROPERTY_GET(ObjectItemS, Item)
  ;
protected:
  /*override */virtual QString Text(int section) const override;
  /*override */virtual QIcon Icon() const override;

public:
  TreeItemObject(ObjectModel* _Core, const ObjectItemS& _Item, TreeItemA* parent = nullptr);
};

