#include "TlvConstructor.h"


TlvS TlvConstructor::AppendRoot(int tag)
{
  mItem.reset(new Tlv());
  mItem->Type = tag;

  mRootList.append(mItem);
  return mItem;
}

TlvS TlvConstructor::AppendRoot(int tag, const QByteArray& value)
{
  mItem.reset(new Tlv());
  mItem->Type = tag;
  mItem->Value = value;

  mRootList.append(mItem);
  return mItem;
}

TlvS TlvConstructor::AppendItem(int tag)
{
  if (!mItem) {
    return TlvS();
  }

  Tlv* lastItem = mItem.data();
  mItem.reset(new Tlv());
  mItem->Type = tag;

  lastItem->Childs.append(mItem);
  return mItem;
}

TlvS TlvConstructor::AppendItem(int tag, const QByteArray& value)
{
  if (!mItem) {
    return TlvS();
  }

  Tlv* lastItem = mItem.data();
  mItem.reset(new Tlv());
  mItem->Type = tag;
  mItem->Value = value;

  lastItem->Childs.append(mItem);
  return mItem;
}

TlvS TlvConstructor::AppendItem(const TlvS& item, int tag)
{
  mItem.reset(new Tlv());
  mItem->Type = tag;

  item->Childs.append(mItem);
  return mItem;
}

TlvS TlvConstructor::AppendItem(const TlvS& item, int tag, const QByteArray& value)
{
  mItem.reset(new Tlv());
  mItem->Type = tag;
  mItem->Value = value;

  item->Childs.append(mItem);
  return mItem;
}

QByteArray TlvConstructor::Construct() const
{
  QByteArray data;
  Construct(data);
  return data;
}

void TlvConstructor::Construct(QByteArray& data) const
{
  for (const TlvS& item: mRootList) {
    ConstructItem(item, data);
  }
}

void TlvConstructor::ConstructItem(const TlvS& item, QByteArray& data) const
{
  if (!item->IsValid()) {
    return;
  }

  QByteArray subData;
  for (const TlvS& subItem: item->Childs) {
    ConstructItem(subItem, subData);
  }

  if (mSimpleType) {
    data.append((char)(uchar)item->Type);
  }
  int size = item->Value.size() + subData.size();
  if (mSimpleValue && size <= 0xff) {
    data.append((char)(uchar)size);
    data.append(item->Value);
    data.append(subData);
  }
}

TlvConstructor::TlvConstructor()
  : mSimpleType(true), mSimpleValue(true)
{
}
