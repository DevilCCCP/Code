#pragma once

#include <QIcon>
#include <QStringList>

#include "TreeItemA.h"


DefineClassS(TreeItemStandard);

class TreeItemStandard: public TreeItemA
{
  QIcon        mIcon;
  QStringList  mText;

protected:
  /*override */virtual QString Text(int section) const override;
  /*override */virtual QIcon Icon() const override;

public:
  TreeItemStandard(const QIcon& _Icon, const QStringList& _Text, TreeItemA* parent = 0);
  TreeItemStandard(const QStringList& _Text, TreeItemA* parent = 0);
  TreeItemStandard(const QIcon& _Icon, const QString& _Text, TreeItemA* parent = 0);
  TreeItemStandard(const QString& _Text, TreeItemA* parent = 0);
  TreeItemStandard(TreeItemA* parent = 0);
};

