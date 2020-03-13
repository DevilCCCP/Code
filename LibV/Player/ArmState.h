#pragma once

enum ArmSignal {
  eSignalNone  = 0,
  ePowerOff    = 1 << 0,
  eHideDesktop = 1 << 1,
  eHideOther   = 1 << 2
};

enum ELayoutType {
  eEmptyLayout   = 0,
  ePreloadLayout = 1,
  eNoPrime       = 2,
  ePrimeHv       = 3,
  ePrimeHorz     = 4,
  ePrimeVert     = 5,
  eOneCamera     = 6,
  eDbLayout      = 7,
  eIllegalLayout
};

struct ArmState {
  volatile int    Signal;

  volatile int    LayoutCounter;
  volatile int    LayoutType;
  volatile int    LayoutCount;
  volatile int    LayoutMonitor1;
  volatile int    CameraGroup;
  volatile qint64 Timestamp;
  volatile int    Reserved9;
  volatile int    Reserved10;
  volatile int    Reserved11;
  volatile int    LayoutEndCounter;

  volatile int    CameraCounter;
  volatile int    CameraSimple;
  volatile qint64 TimestampSimple;
  volatile int    CameraSimpleX;
  volatile int    CameraSimpleY;
  volatile int    CameraSimpleW;
  volatile int    CameraSimpleH;
  volatile int    CameraSimpleChange;
  volatile int    Reserved21;
  volatile int    Reserved22;
  volatile int    Reserved23;
  volatile int    CameraEndCounter;
};

struct MonitorState {
  volatile int    FullScreenLayout;
  volatile int    Reserved2;
  volatile int    Reserved3;
  volatile int    Reserved4;
};
