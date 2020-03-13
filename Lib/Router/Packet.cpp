#include <QDataStream>

#include "Packet.h"


const QByteArray kRecepientMagic = QByteArray::fromHex("13666613661366");
const int kDataSizeMax = 10 * 1024 * 1024;

int Packet::HeaderSize()
{
  return kRecepientMagic.size() + sizeof(int) + sizeof(int) + sizeof(int);
}

bool Packet::HasHeader()
{
  return mType != ePacketIllegal;
}

int Packet::MinimumSize()
{
  if (mType != ePacketIllegal) {
    return mData.size();
  } else {
    return HeaderSize();
  }
}

void Packet::Clear()
{
  mType = ePacketIllegal;
  mId   = -1;
}

bool Packet::ReadHeader(QDataStream* stream)
{
  mType = ePacketIllegal;
  QByteArray header(kRecepientMagic.size(), (char)0);
  stream->readRawData(header.data(), header.size());
  if (header != kRecepientMagic) {
    return false;
  }

  int type = -1;
  *stream >> type;
  if (type < 0 || type >= ePacketIllegal) {
    return false;
  }
  mId = 0;
  *stream >> mId;
  int dataSize = 0;
  *stream >> dataSize;
  if (dataSize < 0 || dataSize > kDataSizeMax) {
    return false;
  }

  if (stream->status() != QDataStream::Ok) {
    return false;
  }
  mData.resize(dataSize);
  mType = (PacketType)type;
  return true;
}

bool Packet::ReadData(QDataStream* stream)
{
  if (mData.size() == 0) {
    return true;
  }

  stream->readRawData(mData.data(), mData.size());
  if (stream->status() != QDataStream::Ok) {
    return false;
  }
  return true;
}

bool Packet::Write(QDataStream* stream) const
{
  stream->writeRawData(kRecepientMagic.constData(), kRecepientMagic.size());
  *stream << (int)mType;
  *stream << mId;
  *stream << mData.size();
  stream->writeRawData(mData.constData(), mData.size());

  if (stream->status() != QDataStream::Ok) {
    return false;
  }
  return true;
}


Packet::Packet()
{
  Clear();
}
