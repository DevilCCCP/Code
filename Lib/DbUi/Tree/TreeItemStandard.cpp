#include "TreeItemStandard.h"


QString TreeItemStandard::Text(int section) const
{
  return mText.value(section);
}

QIcon TreeItemStandard::Icon() const
{
  return mIcon;
}


TreeItemStandard::TreeItemStandard(const QIcon& _Icon, const QStringList& _Text, TreeItemA* parent)
  : TreeItemA(parent)
  , mIcon(_Icon), mText(_Text)
{
}

TreeItemStandard::TreeItemStandard(const QStringList& _Text, TreeItemA* parent)
  : TreeItemA(parent)
  , mText(_Text)
{
}

TreeItemStandard::TreeItemStandard(const QIcon& _Icon, const QString& _Text, TreeItemA* parent)
  : TreeItemA(parent)
  , mIcon(_Icon), mText(QStringList() << _Text)
{
}

TreeItemStandard::TreeItemStandard(const QString& _Text, TreeItemA* parent)
  : TreeItemA(parent), mText(QStringList() << _Text)
{
}

TreeItemStandard::TreeItemStandard(TreeItemA* parent)
  : TreeItemA(parent)
{
}
