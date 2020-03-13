#include <Lib/Log/Log.h>

#include "TlvParser.h"
#include "Tlv.h"


TlvS TlvParser::Parse(const QByteArray& data)
{
  TlvS head(new Tlv());
  head->Type = 0;
  head->Value = data;
  head->Parent = nullptr;
  if (!Parse(head)) {
    head.clear();
  }
  return head;
}

bool TlvParser::Parse(const TlvS& head)
{
  for (int pos = 0; pos < head->Value.size(); ) {
    TlvS node = head->AddChild();

    if (mComplexType) {
      if (!ParseElem(head->Value, pos, &node->Type)) {
        return false;
      }
    } else {
      node->Type = (int)(uchar)head->Value[pos++];
    }
    int length;
    if (!ParseElem(head->Value, pos, &length)) {
      return false;
    }
    if (node->Type == 0 && length == 0) {
      return true;
    }
    if (pos + length > head->Value.size()) {
      return false;
    }

    node->Value = head->Value.mid(pos, length);
    pos += length;
    for (auto itr = mScheme.begin(); itr != mScheme.end(); itr++) {
      int type = *itr;
      if (type == node->Type && !Parse(node)) {
        return false;
      }
    }

    if (pos == head->Value.size()) {
      return true;
    }
  }
  return false;
}

bool TlvParser::ParseElem(const QByteArray& tlvData, int& pos, int* value)
{
  if (pos >= tlvData.size()) {
    return false;
  }
  int val = (int)(uchar)tlvData[pos++];
  if (val >= 0x80) {
    int len = val - 0x80;
    if (len == 0 || pos + len > tlvData.size()) {
      return false;
    }

    val = 0;
    for (int i = 0; i < len; i++, pos++) {
      val <<= 8;
      val |= (int)(uchar)tlvData[pos];
    }
  }

  if (value) {
    *value = val;
  }
  return true;
}


TlvParser::TlvParser()
  : mComplexType(true)
{ }
