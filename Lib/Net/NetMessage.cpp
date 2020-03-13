#include <QtEndian>

#include <Lib/Log/Log.h>

#include "NetMessage.h"


void NetMessage::GetReadHeaderBuffer(char *&buffer, int &size)
{
  mRawData.resize(HeaderSize());
  buffer = mRawData.data();
  size = HeaderSize();
}

void NetMessage::GetReadDataBuffer(char *&buffer, int &size)
{
  mRawData.resize(HeaderSize() + mMessageHeader.RawDataSize);
  buffer = mRawData.data() + HeaderSize();
  size = mMessageHeader.RawDataSize;
}

bool NetMessage::ReadAndValidateHeader()
{
  ReadHeader();
  return ValidateHeader();
}

bool NetMessage::ValidateHeader()
{
  return (mMessageHeader.Version & MessageHeader::kMagicMask) == MessageHeader::kMagic
      && mMessageHeader.RawDataSize < MessageHeader::kDataSizeLimit;
}

void NetMessage::ReadHeader()
{
  BeginRead();
  ReadInt(mRawData, mMessageHeader.Version);
  ReadInt(mRawData, mMessageHeader.RequestType);
  ReadInt(mRawData, mMessageHeader.MessageType);
  ReadInt(mRawData, mMessageHeader.MessageId);
  ReadInt(mRawData, mMessageHeader.RawDataSize);
  EndRead(HeaderSize());
  mPrepared = true;
}

void NetMessage::WriteHeader()
{
  mRawData.resize(HeaderSize());
  BeginWrite();
  WriteInt(mRawData, mMessageHeader.Version);
  WriteInt(mRawData, mMessageHeader.RequestType);
  WriteInt(mRawData, mMessageHeader.MessageType);
  WriteInt(mRawData, mMessageHeader.MessageId);
  WriteInt(mRawData, mMessageHeader.RawDataSize);
  EndWrite(HeaderSize());
  mPrepared = true;
}

void NetMessage::BeginRead()
{
  if (mRwCounter) {
    Log.Fatal("Message read unexpected", true);
  }
}

void NetMessage::EndRead(int totalSize)
{
  if (mRwCounter != totalSize) {
    Log.Fatal("Message header read format invalid", true);
  }
  mRwCounter = 0;
}

void NetMessage::ReadInt(const QByteArray& rawHeader, int& value)
{
  value = qFromLittleEndian<qint32>(reinterpret_cast<const uchar*>(rawHeader.constData() + mRwCounter));
  mRwCounter += 4;
}

void NetMessage::BeginWrite()
{
  if (mRwCounter) {
    Log.Fatal("Message write unexpected", true);
  }
}

void NetMessage::EndWrite(int totalSize)
{
  if (mRwCounter != totalSize) {
    Log.Fatal("Message write read format invalid", true);
  }
  mRwCounter = 0;
}

void NetMessage::WriteInt(QByteArray &rawHeader, const int &value)
{
  qToLittleEndian<qint32>(value, reinterpret_cast<uchar*>(rawHeader.data() + mRwCounter));
  mRwCounter += 4;
}

NetMessageS NetMessage::CreateForRead()
{
  return NetMessageS(new NetMessage);
}

NetMessageS NetMessage::CreateForRead(const QByteArray &_RawHeader)
{
  if (_RawHeader.size() >= HeaderSize()) {
    NetMessageS msg(new NetMessage);
    msg->mRawData = _RawHeader;
    return msg;
  } else {
    Log.Fatal("NetMessage create with too short header size");
    return NetMessageS();
  }
}

NetMessageS NetMessage::CreateForWrite(NetMessage::ERequestType _RequestType, int _MessageType, int _MessageId, int _RawDataSize)
{
  NetMessageS msg(new NetMessage);
  msg->mMessageHeader.Version = MessageHeader::kMagic + MessageHeader::kCurrentVersion;
  msg->mMessageHeader.RequestType = _RequestType;
  msg->mMessageHeader.MessageType = _MessageType;
  msg->mMessageHeader.MessageId = _MessageId;
  msg->mMessageHeader.RawDataSize = _RawDataSize;
  msg->mRawData.resize(_RawDataSize);
  msg->WriteHeader();
  return msg;
}

NetMessage::NetMessage()
  : mRwCounter(0), mPrepared(false)
{
  mMessageHeader.RequestType = eIllegal;
}

NetMessage::~NetMessage()
{
}
