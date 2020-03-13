#pragma once

#include <QString>
#include <mfxstructures.h>


typedef class MFXVideoSession MfxVideoSession;
typedef QSharedPointer<MfxVideoSession> MfxVideoSessionS;
typedef class MFXVideoDECODE MfxVideoDecode;
typedef QSharedPointer<MfxVideoDecode> MfxVideoDecodeS;
typedef mfxBitstream MfxBitstream;
typedef mfxFrameInfo MfxFrameInfo;
typedef mfxFrameSurface1 MfxFrameSurface;
typedef mfxSyncPoint MfxSyncPoint;

inline QString MfxErrorToString(int err)
{
  switch (err) {
  case   0: return QStringLiteral("no error");
  case  -1: return QStringLiteral("unknown error");
  case  -2: return QStringLiteral("null pointer");
  case  -3: return QStringLiteral("undeveloped feature");
  case  -4: return QStringLiteral("failed to allocate memory");
  case  -5: return QStringLiteral("not enough buffer");
  case  -6: return QStringLiteral("invalid handle");
  case  -7: return QStringLiteral("lock the memory");
  case  -8: return QStringLiteral("not initialized");
  case  -9: return QStringLiteral("not found");
  case -10: return QStringLiteral("more data");
  case -11: return QStringLiteral("more surface");
  case -12: return QStringLiteral("aborted");
  case -13: return QStringLiteral("device lost");
  case -14: return QStringLiteral("incompatible video param");
  case -15: return QStringLiteral("invalid video param");
  case -16: return QStringLiteral("undefined behavior");
  case -17: return QStringLiteral("device failed");
  case -18: return QStringLiteral("more bitstream");
  case -19: return QStringLiteral("incompatible audio param");
  case -20: return QStringLiteral("invalid audio param");
  case -21: return QStringLiteral("gpu hang");
  case -22: return QStringLiteral("realloc surface");

  case   1: return QStringLiteral("wrn in execution");
  case   2: return QStringLiteral("wrn device busy");
  case   3: return QStringLiteral("wrn video param changed");
  case   4: return QStringLiteral("wrn partial acceleration");
  case   5: return QStringLiteral("wrn incompatible video param");
  case   6: return QStringLiteral("wrn value not changed");
  case   7: return QStringLiteral("wrn out of range");
  case  10: return QStringLiteral("wrn filter skipped");
  case  11: return QStringLiteral("wrn incompatible audio param");

  case   8: return QStringLiteral("task working");
  case   9: return QStringLiteral("task busy");
  }
  return QString::number(err);
}

