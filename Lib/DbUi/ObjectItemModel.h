#pragma once

#include <QAbstractTableModel>

#include <Lib/Db/Db.h>
#include <Lib/Db/ObjectType.h>


DefineClassS(ObjectItemModel);

class ObjectItemModel: public QAbstractTableModel
{
  PROPERTY_GET_SET(QStringList,    Headers)
  PROPERTY_GET_SET(QIcon,          Icon)
  PROPERTY_GET(QList<ObjectItemS>, Items)

public:
  /*override */virtual int rowCount(const QModelIndex &parent) const Q_DECL_OVERRIDE;
  /*override */virtual int columnCount(const QModelIndex &parent) const Q_DECL_OVERRIDE;
  /*override */virtual QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
  /*override */virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;

public:
  void SetList(const QList<ObjectItemS>& _Items);
  void UpdateList(const QList<ObjectItemS>& _Items);

  bool GetItem(const QModelIndex& index, ObjectItemS& item) const;

public:
  ObjectItemModel(QObject* parent = 0);
};

