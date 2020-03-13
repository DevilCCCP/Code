#pragma once

#include <QByteArray>

#include <Lib/Include/Common.h>


DefineClassS(TrFrame);

class TrFrame
{
public:
  int              mType;
  const QByteArray mData;

public:
  TrFrame(int _Type, const QByteArray& _Data)
    : mType(_Type), mData(_Data)
  { }
};

