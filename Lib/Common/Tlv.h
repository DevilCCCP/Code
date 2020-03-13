#pragma once

#include <QByteArray>
#include <QString>
#include <QList>

#include <Lib/Include/Common.h>


DefineClassS(Tlv);

class Tlv
{
public:
  int         Type;
  QByteArray  Value;
  QList<TlvS> Childs;
  Tlv*        Parent;

  TlvS At(int typeA);

  TlvS At(int typeA, int typeB);

  TlvS At(int typeA, int typeB, int typeC);

  TlvS AddChild();

  void Dump() { DumpAll(""); }

private:
  void DumpAll(const QString& level);
  void DumpOne(const QString& level, const TlvS& node);

  static QString DumpValueHex(const QByteArray& value);
  static QString DumpValueText(const QByteArray& value);

public:
  Tlv();
};
