#pragma once

#include <QByteArray>
#include <QString>

#include "DebugWnd.h"


struct Object;

class FrameWnd: public FrameWndA
{
  QWidget*   mWidget;
  QString    mCaption;
  QByteArray mScreenBuffer;

public:
  /*override */virtual void Create(QWidget* w, int posX, int posY, int width, int height);

  /*override */virtual void SetCaption(const QString& caption);
  /*override */virtual void DrawWindow(const char* data, EImageType imageType, int save, int width, int height, int stride, const char *objData, int objSize);

private:
  void DrawImage(EImageType imageType, const char* src, int srcWidth, int srcHeight, int srcStride, char* dst, int dstWidth, int dstHeight, int dstStride, bool reverse = false);
  void DrawObject(const Object* object, int srcWidth, int srcHeight, char* dst, int dstWidth, int dstHeight, int dstStride);

public:
  FrameWnd();
  /*override */virtual ~FrameWnd();
};

