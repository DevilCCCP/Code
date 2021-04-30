#pragma once

#include <QDateTime>

#include <Lib/Include/Common.h>
#include <LibV/Include/Frame.h>


enum ECellCondition {
  eCellEmpty,
  eCellWriting,
  eCellPartialSaved,
  eCellFullSaved,
  eCellIllegal
};

/// Структура Cell (ячейки)
/// [
/// MagicA
/// CellFrontHeader - fixed size header, localed at cell start
/// .
/// FrameEx[N]      - vector (CellFrameHeader, frame)
/// unused space    - variable size
/// .
/// MagicB
/// CellIndex[N]    - vector disp of frame end from cell begin
/// .
/// MagicC
/// CellBackHeader  - fixed size header, localed at cell end
/// MagicD
/// ]

const int kCellMagicFlag = 0xFFFFFF7F;
const int kCellMagicX    = 0x00000080;
const int kCellMagicA    = 0x3636367A;
const int kCellMagicB    = 0x3636367B;
const int kCellMagicC    = 0x3636367C;
const int kCellMagicD    = 0x3636367D;
const int kCellMagicE    = 0x3636367D;

#pragma pack(push, 2)

struct CellFrontHeader
{
  /// format validation + X bit
  int    MagicA;

  /// cell validation
  int    UnitId;
  qint64 StartTime;

  /// back seeking
  int    PrevCell;
  qint64 PrevStartTime;

  CellFrontHeader(int magicX): MagicA(kCellMagicA | magicX) { }
  CellFrontHeader(): MagicA(0) { }
};

struct CellFrameHeader
{
  int   MagicE;

  CellFrameHeader(int magicX): MagicE(kCellMagicE | magicX) { }
};

struct CellIndexHeader
{
  int   MagicB;

  CellIndexHeader(int magicX): MagicB(kCellMagicB | magicX) { }
};

struct CellIndex
{
  int    Disp;
  qint64 Timestamp;
  bool   Key;

  //static bool operator<(const qint64& timestamp, const CellIndex& b) { return timestamp < b.Timestamp; }
  //bool operator<(const CellIndex& b) { return Timestamp < b.Timestamp; }
  //bool operator<(const int64& timestamp) { return Timestamp < timestamp; }
  //bool operator>(const CellIndex& b) { return Timestamp > b.Timestamp; }
  //bool operator>(const int64& timestamp) { return Timestamp > timestamp; }
  //bool operator<=(const CellIndex& b) { return Timestamp <= b.Timestamp; }
  //bool operator<=(const int64& timestamp) { return Timestamp <= timestamp; }
  //bool operator>=(const CellIndex& b) { return Timestamp >= b.Timestamp; }
  //bool operator>=(const int64& timestamp) { return Timestamp >= timestamp; }
};
inline bool operator<(const CellIndex& a, const CellIndex& b) { return a.Timestamp < b.Timestamp; }
inline bool operator==(const CellIndex& a, const CellIndex& b) { return a.Timestamp == b.Timestamp; }
inline bool operator<(const qint64& timestamp, const CellIndex& b) { return timestamp < b.Timestamp; }
inline bool operator==(const qint64& timestamp, const CellIndex& b) { return timestamp == b.Timestamp; }
inline bool operator<(const CellIndex& a, const qint64& timestamp) { return a.Timestamp < timestamp; }
inline bool operator==(const CellIndex& a, const qint64& timestamp) { return a.Timestamp == timestamp; }

struct CellBackHeader
{
  int            MagicC;
  ECellCondition Condition;

  int    FrameIndexDisp; // disp to MagicB

  /// cell validation
  int    UnitId;
  qint64 StartTime;

  /// front seeking
  int    NextCell;
  qint64 EndTime;

  int    Reserved1;
  int    Reserved2;

  int    MagicD;

  CellBackHeader(int magicX): MagicC(kCellMagicC | magicX), Reserved1(0), Reserved2(0), MagicD(kCellMagicD | magicX) { }
  CellBackHeader(): MagicC(0), Reserved1(0), Reserved2(0), MagicD(0) { }
};

#pragma pack(pop)
