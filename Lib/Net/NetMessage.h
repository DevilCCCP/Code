#pragma once

#include <QSharedPointer>
#include <QByteArray>

#include <Lib/Include/Common.h>

DefineClassS(NetMessage);

class NetMessage
{
public:
  enum ERequestType {
    ePing,
    eRequest,
    eRespond,
    eMessage,
    eIllegal
  };

private:
  struct MessageHeader
  {
    static const int kMagic = 0x06661300;
    static const int kMagicMask = 0xffffff00;
    static const int kCurrentVersion = 1;
    static const int kDataSizeLimit = 512 * 1024 * 1024;

    int          Version;
    int          RequestType;
    int          MessageType;
    int          MessageId;
    int          RawDataSize;

    MessageHeader(): Version(0) { }
  };

  MessageHeader mMessageHeader;
  QByteArray    mRawData;
  int           mRwCounter;
  bool          mPrepared;

public:
  static int HeaderSize() { return 20; }

  ERequestType GetRequestType() { return (ERequestType)mMessageHeader.RequestType; }
  const int& GetMessageType() { return mMessageHeader.MessageType; }
  const int& GetMessageId() { return mMessageHeader.MessageId; }
  char* GetMessageData() { mRawData.resize(HeaderSize() + mMessageHeader.RawDataSize); return mRawData.data() + HeaderSize(); }
  const char* GetMessageConstData() { return mRawData.data() + HeaderSize(); }
  int GetMessageDataSize() { return mMessageHeader.RawDataSize; }
  const QByteArray& GetRawData() const { return mRawData; }

public:
  void GetReadHeaderBuffer(char*& buffer, int& size);
  void GetReadDataBuffer(char*& buffer, int& size);

  bool ReadAndValidateHeader();
  bool ValidateHeader();

private:
  void ReadHeader();
  void WriteHeader();

  inline void BeginRead();
  inline void EndRead(int totalSize);
  inline void ReadInt(const QByteArray &rawHeader, int &value);

  inline void BeginWrite();
  inline void EndWrite(int totalSize);
  inline void WriteInt(QByteArray &rawHeader, const int &value);

public:
  static NetMessageS CreateForRead();
  static NetMessageS CreateForRead(const QByteArray& _RawHeader);
  static NetMessageS CreateForWrite(NetMessage::ERequestType _RequestType, int _MessageType, int _MessageId, int _RawDataSize);

private:
  NetMessage(); // write message
public:
  ~NetMessage();
};

