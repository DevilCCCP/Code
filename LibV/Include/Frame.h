#pragma once
#include <vector>
#include <QSharedPointer>

#include <Lib/Include/Common.h>
#include <Lib/Include/FrameA.h>


DefineClassS(Frame);

struct Connection
{
  enum EStatus
  {
    eNone,
    eConnecting,
    eConnected,
    eNoFrames,
    eNotConnected,
    eDisconnected,
    eAuthFail,
    eRefused,
    eStatusIllegal
  };

  inline static const char* StatusToString(EStatus status)
  {
    switch (status) {
    case eNone:          return "None";
    case eConnecting:    return "Connecting";
    case eConnected:     return "Connected";
    case eNoFrames:      return "NoFrames";
    case eNotConnected:  return "NotConnected";
    case eDisconnected:  return "Disconnected";
    case eAuthFail:      return "AuthFail";
    case eRefused:       return "Refused";
    case eStatusIllegal: return "Illegal";
    default:             return "Unknown";
    }
  }
};

enum ECompression
{
  eCmprNone = 0,
  eRawRgb   = 1,
  eRawYuv   = 2,
  eRawNv12  = 3,
  eRawYuvP  = 4,
  eRawRgba  = 5,
  eRawNv12A = 6, // NV12 align 32 bytes (vert and horz)
  eJpeg     = 0x100,
  eMpeg4    = 0x101,
  eH264     = 0x102,
  eRawAuF16 = 0x8001,
  eRawAuS16 = 0x8002,
  eRawAuS32 = 0x8003,
  eAac16b   = 0x8100,

  eRawAnyMask     = 0x00ff,
  eTypeMask       = 0xff00,
  eRawVideo       = 0x0000,
  eCompessedVideo = 0x0100,
  eAnyAudio       = 0x8000,
  eCompessedAudio = 0x8100,
};

inline static const char* CompressionToString(ECompression compression)
{
  switch (compression) {
  case eCmprNone:  return "None";
  case eRawRgb:    return "RawRgb";
  case eRawYuv:    return "RawYuv";
  case eRawNv12:   return "RawNv12";
  case eRawYuvP:   return "RawYuvP";
  case eJpeg:      return "Jpeg";
  case eMpeg4:     return "Mpeg4";
  case eH264:      return "H264";
  case eRawAuF16:  return "RawAudioFloat16";
  case eAac16b:    return "eAac16b";
  default:         return "Unknown";
  }
}

#pragma pack(push, 2)
class Frame: public FrameA
{
private:
  std::vector<char> RawStorage; // все объявленные данные на самом деле лежат в этом векторе
  int               Disp;       // по этому смещению (на случай необходимости добавить заголовок переменной длины)
  char*             RawData;

public:
  struct Header
  {
    int               Size;
    int               HeaderSize;
    qint64            Timestamp;
    bool              Key;

    ECompression      Compression;
    ECompression      CompressionAudio;
    int               Width;
    int               Height;

    int               VideoDataSize;
    int               AudioDataSize;
    int               ObjectDataSize;
  };

  struct StatusHeader
  {
    int                 Size;
    int                 HeaderSize;
    Connection::EStatus Status;
  };

  struct Data
  {
    char*             VideoData;
    char*             AudioData;
    char*             ObjectData;
  };

  const qint64& Timestamp() const { return GetHeader()->Timestamp; }
  char* Data() { return RawData; }
  int Size() const { return GetHeader()->Size; }
  void SetDisp(int _Disp) { Disp = _Disp; RawData = RawStorage.data() + Disp; }
  int GetDisp() const { return Disp; }

  void AppendData(char* data, int size) { RawStorage.insert(RawStorage.end(), data, data + size); RawData = RawStorage.data() + Disp; }
  void ReserveData(int size) { RawStorage.resize(Disp + sizeof(Header) + size); RawData = RawStorage.data() + Disp; }
  char* VideoData() const { return RawData + sizeof(Header); }
  int VideoDataSize() const { return GetHeader()->VideoDataSize; }
  char* AudioData() const { return RawData + sizeof(Header) + GetHeader()->VideoDataSize; }
  int AudioDataSize() const { return GetHeader()->AudioDataSize; }
  char* ObjectData() const { return RawData + sizeof(Header) + GetHeader()->VideoDataSize + GetHeader()->AudioDataSize; }
  int ObjectDataSize() const { return GetHeader()->ObjectDataSize; }

  Header* InitHeader() { Header* h = GetHeader(); size_t sz = sizeof(Header); memset(h, 0, sz); h->HeaderSize = sz; return h; }
  Header* GetHeader() { return reinterpret_cast<Header*>(RawData); }
  const Header* GetHeader() const { return reinterpret_cast<Header*>(RawData); }
  StatusHeader* GetStatusHeader() const { return reinterpret_cast<StatusHeader*>(RawData); }

  bool IsFrame() const { return GetHeader()->HeaderSize == sizeof(Header); }
  bool IsStatus() const { return GetStatusHeader()->HeaderSize == sizeof(StatusHeader); }

  Frame(): Disp(0), RawData(nullptr) { }
  Frame(char* _RawData): Disp(0), RawData(_RawData) { }
};
#pragma pack(pop)
