#pragma once

#include "Tlv.h"


class TlvConstructor
{
  const bool  mSimpleType;
  const bool  mSimpleValue;

  QList<TlvS> mRootList;
  TlvS        mItem;

public:
  TlvS AppendRoot(int tag);
  TlvS AppendRoot(int tag, const QByteArray& value);
  TlvS AppendItem(int tag);
  TlvS AppendItem(int tag, const QByteArray& value);
  TlvS AppendItem(const TlvS& item, int tag);
  TlvS AppendItem(const TlvS& item, int tag, const QByteArray& value);

  QByteArray Construct() const;
  void Construct(QByteArray& data) const;

  void ConstructItem(const TlvS& item, QByteArray& data) const;

public:
  TlvConstructor();
};
