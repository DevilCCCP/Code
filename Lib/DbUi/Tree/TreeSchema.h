#pragma once

#include <QString>

#include <Lib/Include/Common.h>
#include <Lib/Db/Db.h>


DefineClassS(TreeSchema);

class TreeRootItem {
  PROPERTY_GET_SET(QString, Type)
  PROPERTY_GET_SET(QString, Name)
  PROPERTY_GET_SET(QString, Description)
  PROPERTY_GET_SET(bool   , Expanded)
  PROPERTY_GET_SET(bool   , UnlinkedOnly)
  ;
public:
  TreeRootItem() { }
  TreeRootItem(const char* _Type, const char* _Name, bool _Expanded, bool _UnlinkedOnly = false, const char* _Description = nullptr)
    : mType(_Type), mName(_Name), mDescription(_Description), mExpanded(_Expanded), mUnlinkedOnly(_UnlinkedOnly) { }
};

class TreeSchema
{
  QList<TreeRootItem>   mSchema;

  QMultiMap<int, int>   mTypeItemMap;
  int                   mMinId;
  bool                  mInit;

public:
  bool IsInit() { return mInit; }
  int GetMinId() { return mMinId; }
  void SetSchema(const QList<TreeRootItem>& _Schema) { mSchema = _Schema; mInit = false; }
  void SetMinId(int _MinId) { mMinId = _MinId; }
  const QList<TreeRootItem>& GetSchema() const { return mSchema; }
  int GetSize() const { return mSchema.size(); }

public:
  void Reload(const ObjectTypeTableS& objectTypeTable);
  bool GetIndex(int objType, int& index);

public:
  TreeSchema();
};

