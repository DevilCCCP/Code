#pragma once

#include <QMutex>
#include <QString>

#include <Lib/Ctrl/CtrlWorker.h>
#include <Lib/Include/Common.h>


DefineClassS(FrameWndA);
DefineClassS(DebugWnd);
DefineClassS(QByteArray);
class QWidget;

enum EImageType {
  eValue,
  eValue2,
  eIndex,
  eHyst
};

/*abstract */class FrameWndA
{
public:
  /*new */virtual void Create(QWidget* w, int posX, int posY, int width, int height) = 0;

  /*new */virtual void SetCaption(const QString& caption) = 0;
  /*new */virtual void DrawWindow(const char* data, EImageType imageType, int save, int width, int height, int stride, const char *objData = nullptr, int objSize = 0) = 0;
  /*new */virtual ~FrameWndA() = 0;
};

class DebugWnd: public CtrlWorker
{
  QList<FrameWndAS> mFrameWndList;
  QList<QString>    mFrameCaptions;
  int               mFrameCount;
  int               mFrameX;
  int               mWidth;
  int               mHeight;
  int               mStride;

  struct DrawStruct {
    int         Index;
    EImageType  ImageType;
    int         Save;
    QByteArrayS Frame;
    int         Stride;
  };

  QMutex            mDrawMutex;
  QList<DrawStruct> mDrawList;
  QList<QWidget*>   mWindowPool;

public:
  /*override */virtual const char* Name() { return "DebugWnd"; }
  /*override */virtual const char* ShortName() { return "W"; }

protected:
//  /*override */virtual bool DoInit();
  /*override */virtual bool DoCircle();
//  /*override */virtual void DoRelease();

public:
  void SetDimentions(int width, int height, int defaultStride);
  void SetFrameCount(int _FrameCount);
  void DrawWindow(int index, const QString& caption, EImageType imageType, int save, const QByteArrayS& frame, int stride = 0, const char* objData = nullptr, int objSize = 0);

public:
  DebugWnd(const QList<QWidget*>& _WindowPool);
  virtual ~DebugWnd();
};

