#pragma once

#include <QByteArray>
#include <QString>
#include <QList>

#include <Lib/Include/Common.h>


DefineClassS(Tlv);

class TlvParser
{
  PROPERTY_GET_SET(QList<int>, Scheme)
  PROPERTY_GET_SET(bool,       ComplexType)

public:
  TlvS Parse(const QByteArray& data);
  bool Parse(const TlvS& head);

private:
  bool ParseElem(const QByteArray& tlvData, int& pos, int* value);

public:
  TlvParser();
};
