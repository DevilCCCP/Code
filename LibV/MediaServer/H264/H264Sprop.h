#pragma once

#include <Lib/Include/Common.h>


DefineClassS(H264Sprop);
DefineClassS(BitstreamReader);
DefineClassS(BitstreamWriter);
DefineStructS(H264Sps);
DefineStructS(H264Pps);
DefineStructS(H264Sei);
DefineStructS(H264Slice);

class H264Sprop
{
  struct NalUnit {
    int forbidden_zero_bit;
    int nal_ref_idc;
    int nal_unit_type;
  };

public:
  enum NaluType {
    eNaluNone    = 0,
    eSliceNonIdr = 1,
    eSlicePartA  = 2,
    eSlicePartB  = 3,
    eSlicePartC  = 4,
    eSliceIdr    = 5,
    eSei         = 6,
    eSps         = 7,
    ePps         = 8,
  };

//  Table 7-3 – Name association to slice_type
//  slice_type Name of slice_type
//  0 P (P slice)
//  1 B (B slice)
//  2 I (I slice)
//  3 SP (SP slice)
//  4 SI (SI slice)
  enum SliceType {
    eSliceP    = 0, ///< Predicted
    eSliceB    = 1, ///< Bi-dir predicted
    eSliceI    = 2, ///< Intra
    eSliceSp   = 3, ///< Switching Predicted
    eSliceSi   = 4, ///< Switching Intra
  };

private:
  NalUnit    mNalUnit;
  H264SpsS   mSps;
  H264PpsS   mPps;
  H264SeiS   mSei;
  H264SliceS mSlice;

  BitstreamReaderS mReader;
  BitstreamWriterS mWriter;

public:
  const H264Sps&   GetSps()   const { return *mSps; }
  const H264Pps&   GetPps()   const { return *mPps; }
  const H264Slice& GetSlice() const { return *mSlice; }

  int WriteSize()        const { return 48; }
  bool HasSps()          const { return mSps; }
  bool HasPps()          const { return mPps; }
  bool HasSei()          const { return mSei; }
  bool HasSlice()        const { return mSlice; }
  NaluType GetNaluType() const { return (NaluType)mNalUnit.nal_unit_type; }
  bool IsRefference()    const { return mNalUnit.nal_ref_idc != 0; }
  void ClearSps()   { return mSps.reset(); }
  void ClearPps()   { return mPps.reset(); }
  void ClearSlice() { return mSlice.reset(); }

  int Width() const;
  int Height() const;
  SliceType GetSliceType() const;
  QString NaluTypeToString() const;
  QString SliceTypeToString() const;

public:
  int Write(char* data, int size);

private:
  void WriteSps();
  void WriteSpsVuiParameters();
  int WritePps();
  int WriteTrail();

public:
  bool Test(const char* data, int size);
  bool Parse(const char* data, int size, bool useSlice = false);
  bool AtEnd();
  bool MoreRbspData();

private:
  bool ParseSps();
  bool ParsePps();
  bool ParseSei();
  bool ParseSlice();
  bool ParseTrail();

  bool ParseSpsScalingList(int sizeOfScalingList);
  bool ParseSpsVuiParameters();
  bool ParseHdrParameters();

  bool ParseSeiMsg();

  bool ParseSliceRefPicListReordering();
  bool ParseSlicePredWeightTable();
  bool ParseSliceDecRefPicMarking();

public:
  H264Sprop();
  ~H264Sprop();
};


