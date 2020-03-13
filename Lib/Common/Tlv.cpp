#include <Lib/Log/Log.h>

#include "Tlv.h"


TlvS Tlv::At(int typeA)
{
  for (auto itr = Childs.begin(); itr != Childs.end(); itr++) {
    const TlvS& child = *itr;
    if (child->Type == typeA) {
      return child;
    }
  }
  return TlvS();
}

TlvS Tlv::At(int typeA, int typeB)
{
  TlvS a = At(typeA);
  if (a) {
    a = a->At(typeB);
  }
  return a;
}

TlvS Tlv::At(int typeA, int typeB, int typeC)
{
  TlvS a = At(typeA);
  if (a) {
    a = a->At(typeB, typeC);
  }
  return a;
}

TlvS Tlv::AddChild()
{
  Tlv* child;
  TlvS node(child = new Tlv());
  Childs.append(node);
  node->Parent = this;
  return node;
}

void Tlv::DumpAll(const QString& level)
{
  for (auto itr = Childs.begin(); itr != Childs.end(); itr++) {
    const TlvS& child = *itr;
    DumpOne(level, child);
  }
}

void Tlv::DumpOne(const QString& level, const TlvS& node)
{
  Log.Info(QString("%1t: 0x%2 v(%3): %4 %5").arg(level).arg(node->Type, 0, 16).arg(node->Value.size())
                   .arg(DumpValueHex(node->Value)).arg(DumpValueText(node->Value)));
  node->DumpAll(level + ' ');
}

inline char Hex(int value)
{
  int h = value & 0xf;
  switch (h)
  {
  case 0: return '0';
  case 1: return '1';
  case 2: return '2';
  case 3: return '3';
  case 4: return '4';
  case 5: return '5';
  case 6: return '6';
  case 7: return '7';
  case 8: return '8';
  case 9: return '9';
  case 10: return 'A';
  case 11: return 'B';
  case 12: return 'C';
  case 13: return 'D';
  case 14: return 'E';
  case 15: return 'F';
  default: return '?';
  }
}

QString Tlv::DumpValueHex(const QByteArray& value)
{
  QString sb("[ ");
  for (int i = 0; i < value.size(); i++) {
    if (i >= 10) {
      sb.append(", ...");
      break;
    } else if (i) {
      sb.append(", ");
    }
    sb.append(Hex(value[i] >> 4));
    sb.append(Hex(value[i]));
  }
  sb.append(" ]");
  return sb;
}

QString Tlv::DumpValueText(const QByteArray& value)
{
  QString sb("\"");
  for (int i = 0; i < value.size(); i++) {
    if (i >= 20) {
      sb.append(">");
      break;
    } else if ((uchar)value.at(i) < 0x20) {
      sb.append('_');
    } else if ((uchar)value.at(i) > 0x7f) {
      sb.append('.');
    } else {
      sb.append(value[i]);
    }
  }
  sb.append("\"");
  return sb;
}


Tlv::Tlv()
{
}
