#include <QCoreApplication>
#include <QWidget>
#include <QImage>
#include <QFile>
#include <QDir>

#include <Lib/Log/Log.h>

#include "FrameWnd.h"
#include "FrameWindow.h"
#include "../AnalyticsA.h"


const int kFrameCountLimit = 2500;

void FrameWnd::Create(QWidget* w, int posX, int posY, int width, int height)
{
  mWidget = w;
  mWidget->move(posX, posY);
  mWidget->resize(width, height);
  mWidget->show();
}

void FrameWnd::SetCaption(const QString &caption)
{
  mCaption = caption;
  if (mWidget) {
    mWidget->setWindowTitle(mCaption);
  }
}

void FrameWnd::DrawWindow(const char *data, EImageType imageType, int save, int width, int height, int stride, const char *objData, int objSize)
{
  if (!mWidget) {
    return;
  }

  QImage shot(width, height, QImage::Format_RGB888);
  char* imgData = (char*)shot.scanLine(0);
  int imgStride = (char*)shot.scanLine(1) - imgData;
  DrawImage(imageType, data, width, height, stride, imgData, width, height, imgStride, true);
  if (objData && objSize > 0) {
    for (Object* object = (Object*)objData; objSize >= (int)sizeof(Object); objSize -= (int)sizeof(Object), object++) {
      DrawObject(object, width, height, imgData, width, height, imgStride);
    }
  }
  qobject_cast<FrameWindow*>(mWidget)->SetImage(shot);
  if (save > 0 && save < kFrameCountLimit) {
    if (save == 1) {
      QDir dir(QDir::current());
      dir.mkdir("img");
    }
    QString filename = QString("./img/%1_%2.jpg").arg(mCaption).arg(save, 6, 10, QChar('0'));
    shot.save(filename, "JPG", 95);
  }
}

void FrameWnd::DrawImage(EImageType imageType, const char *src, int srcWidth, int srcHeight, int srcStride, char *dst, int dstWidth, int dstHeight, int dstStride, bool reverse)
{
  if (imageType == eValue) {
    for (int j = 0; j < dstHeight; j++) {
      int sourceJ = srcHeight * j / dstHeight;
      const char* sline = src + sourceJ * srcStride;
      char* d = dst + j * dstStride;
      for (int i = 0; i < dstWidth; i++) {
        const char* s = sline + srcWidth * i / dstWidth;
        *d++ = *s;
        *d++ = *s;
        *d++ = *s;
      }
    }
  } else if (imageType == eValue2) {
    for (int j = 0; j < dstHeight; j++) {
      int sourceJ = srcHeight * j / dstHeight;
      const char* sline = src + sourceJ * srcStride;
      char* d = dst + j * dstStride;
      for (int i = 0; i < dstWidth; i++) {
        const char* s = sline + srcWidth * i / dstWidth;
        if (*s & 0x80) {
          if (!reverse) {
            *d++ = 0;
            *d++ = 0;
            *d++ = *s << 1;
          } else {
            *d++ = *s << 1;
            *d++ = 0;
            *d++ = 0;
          }
        } else {
          *d++ = 0;
          *d++ = *s << 1;
          *d++ = 0;
        }
      }
    }
  } else if (imageType == eIndex) {
    const char gIndexColorsS[5][3] = { { (char)(uchar)0, (char)(uchar)0, (char)(uchar)0 },
                                       { (char)(uchar)149, (char)(uchar)149, (char)(uchar)149 },
                                       { (char)(uchar)0, (char)(uchar)129, (char)(uchar)0 },
                                       { (char)(uchar)0, (char)(uchar)127, (char)(uchar)129 },
                                       { (char)(uchar)0, (char)(uchar)0, (char)(uchar)129 }
                                     };
    const char gIndexColorsR[5][3] = { { (char)(uchar)0, (char)(uchar)0, (char)(uchar)0 },
                                       { (char)(uchar)149, (char)(uchar)149, (char)(uchar)149 },
                                       { (char)(uchar)0, (char)(uchar)129, (char)(uchar)0 },
                                       { (char)(uchar)129, (char)(uchar)127, (char)(uchar)0 },
                                       { (char)(uchar)129, (char)(uchar)0, (char)(uchar)0 }
                                     };
    const char gIndexColors[5][3] = { { 0 } };
    if (!reverse) {
      memcpy((char*)gIndexColors, gIndexColorsS, sizeof(gIndexColors));
    } else {
      memcpy((char*)gIndexColors, gIndexColorsR, sizeof(gIndexColors));
    }
    for (int j = 0; j < dstHeight; j++) {
      int sourceJ = srcHeight * j / dstHeight;
      const char* sline = src + sourceJ * srcStride;
      char* d = dst + j * dstStride;
      for (int i = 0; i < dstWidth; i++) {
        const char* s = sline + srcWidth * i / dstWidth;
        int ind = (*s > 5)? 5: *s;
        *d++ = gIndexColors[ind][0];
        *d++ = gIndexColors[ind][1];
        *d++ = gIndexColors[ind][2];
      }
    }
  } else if (imageType == eHyst) {
    const int* datai = reinterpret_cast<const int*>(src);
    int countMax = datai[0];
    if (countMax> 0) {
      for (int j = 0; j < dstHeight; j++) {
        int index = 255 * j / dstHeight + 1;
        int top = qMin(datai[index] * dstWidth / countMax, dstWidth);
        char* d = dst + j * dstStride;
        for (int i = 0; i < top; i++) {
          *d++ = 0;
          *d++ = -127;
          *d++ = 0;
        }
        memset(d, 0, (dstWidth - top) * 3);
      }
    }
  } else {
    for (int j = 0; j < dstHeight; j++) {
      char* d = dst + j * dstStride;
      for (int i = 0; i < dstWidth; i++) {
        if (!reverse) {
          *d++ = 0;
          *d++ = 0;
          *d++ = -127;
        } else {
          *d++ = -127;
          *d++ = 0;
          *d++ = 0;
        }
      }
    }
  }
}

void FrameWnd::DrawObject(const Object* object, int srcWidth, int srcHeight, char* dst, int dstWidth, int dstHeight, int dstStride)
{
  const int kColorIndexes = 4;
  const int gObjectColors[kColorIndexes][3] = { {255, 255, 255}, {0, 255, 0}, {0, 0, 255}, {255, 0, 0} };

  struct Rectangle objRect;
  objRect.Left = object->Dimention.Left * dstWidth / srcWidth;
  objRect.Right = (object->Dimention.Right - 1) * dstWidth / srcWidth;
  objRect.Top = object->Dimention.Top * dstHeight / srcHeight;
  objRect.Bottom = (object->Dimention.Bottom - 1) * dstHeight / srcHeight;
  if (objRect.Top > dstHeight - 2 || objRect.Left > dstWidth - 2 || objRect.Bottom < 2 || objRect.Right < 2) {
    return;
  }
  struct Rectangle objRectSafe;
  objRectSafe.Left = qMax(0, objRect.Left);
  objRectSafe.Right = qMin(dstWidth - 1, objRect.Right);
  objRectSafe.Top = qMax(0, objRect.Top);
  objRectSafe.Bottom = qMin(dstHeight - 1, objRect.Bottom);
  int ind = object->Color / 1000;
  if (ind < 0 || ind >= kColorIndexes) {
    ind = 0;
  }
  int prec = qMin(100, object->Color % 1000);
  char b = (char)(uchar)(gObjectColors[ind][0] * prec / 100);
  char g = (char)(uchar)(gObjectColors[ind][1] * prec / 100);
  char r = (char)(uchar)(gObjectColors[ind][2] * prec / 100);

  if (objRect.Top >= 0) {
    char* d = dst + 3 * objRectSafe.Left + objRectSafe.Top * dstStride;
    char* d2 = dst + 3 * objRectSafe.Left + (objRectSafe.Top + 1) * dstStride;
    for (int i = objRectSafe.Left; i <= objRectSafe.Right; i++) {
      *d++ = *d2++ = b;
      *d++ = *d2++ = g;
      *d++ = *d2++ = r;
    }
  }

  if (objRect.Bottom < dstHeight) {
    char* d = dst + 3 * objRectSafe.Left + objRectSafe.Bottom * dstStride;
    char* d2 = dst + 3 * objRectSafe.Left + (objRectSafe.Bottom - 1) * dstStride;
    for (int i = objRectSafe.Left; i <= objRectSafe.Right; i++) {
      *d++ = *d2++ = b;
      *d++ = *d2++ = g;
      *d++ = *d2++ = r;
    }
  }

  if (objRect.Left >= 0) {
    char* d = dst + 3 * objRectSafe.Left + objRectSafe.Top * dstStride;
    for (int j = objRectSafe.Top; j <= objRectSafe.Bottom; j++) {
      *d++ = b; *d++ = g; *d++ = r;
      *d++ = b; *d++ = g; *d++ = r;
      d += dstStride - 6;
    }
  }

  if (objRect.Right < dstWidth) {
    char* d = dst + 3 * (objRectSafe.Right - 1) + objRectSafe.Top * dstStride;
    for (int j = objRectSafe.Top; j <= objRectSafe.Bottom; j++) {
      *d++ = b; *d++ = g; *d++ = r;
      *d++ = b; *d++ = g; *d++ = r;
      d += dstStride - 6;
    }
  }
}


FrameWnd::FrameWnd()
  : mWidget(nullptr)
{
}

FrameWnd::~FrameWnd()
{
  if (mWidget) {
    mWidget->deleteLater();
  }
}


