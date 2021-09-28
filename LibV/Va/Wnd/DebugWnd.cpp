#include <QApplication>
#include <QDesktopWidget>
#include <QMutexLocker>

#include "DebugWnd.h"
#ifdef ANAL_DEBUG
#include "FrameWnd.h"
#endif

FrameWndA::~FrameWndA()
{
}


bool DebugWnd::DoCircle()
{
  QMutexLocker lock(&mDrawMutex);
  while (!mDrawList.isEmpty()) {
    DrawStruct info = mDrawList.takeFirst();
    FrameWndAS frameWnd = mFrameWndList[info.Index];
    lock.unlock();
    frameWnd->DrawWindow(info.Frame->data(), info.ImageType, info.Save, mWidth, mHeight, info.Stride);
    lock.relock();
  }
  return true;
}

void DebugWnd::SetDimentions(int width, int height, int defaultStride)
{
  mWidth = width;
  mHeight = height;
  mStride = defaultStride;
}

void DebugWnd::SetFrameCount(int _FrameCount)
{
  mFrameCount = qMax(1, _FrameCount);
  mFrameX = qMax(0, (mFrameCount - 1) / 2);
}

void DebugWnd::DrawWindow(int index, const QString &caption, EImageType imageType, int save, const QByteArrayS& frame, int stride, const char *objData, int objSize)
{
#ifdef ANAL_DEBUG
  QMutexLocker lock(&mDrawMutex);
  mFrameCount = qMax(mFrameCount, index + 1);
  while (mFrameWndList.size() <= index) {
    if (index >= mWindowPool.size()) {
      break;
    }
    QRect desktopRect = qApp->desktop()->availableGeometry();
//    RECT desktopRect;
//    SystemParametersInfoA(SPI_GETWORKAREA, 0, &desktopRect, 0);
    int desktopWidth = desktopRect.width();
    int desktopHeight = desktopRect.height();

    FrameWndAS newFrame(new FrameWnd());
    if (index == 0) {
      newFrame->Create(mWindowPool[index], desktopRect.left(), desktopRect.top()
                       , desktopWidth * mFrameX / (mFrameX + 1), desktopHeight * mFrameX / (mFrameX + 1));
    } else if (index <= mFrameX + 1) {
      int ind = index - 1;
      newFrame->Create(mWindowPool[index], desktopRect.left() + ind * desktopWidth / (mFrameX + 1), desktopRect.top() + desktopHeight * mFrameX / (mFrameX + 1)
                       , desktopWidth / (mFrameX + 1), desktopHeight / (mFrameX + 1));
    } else {
      int ind = mFrameX - (index - (mFrameX + 1));
      newFrame->Create(mWindowPool[index], desktopRect.left() + mFrameX * desktopWidth / (mFrameX + 1), desktopRect.top() + desktopHeight * ind / (mFrameX + 1)
                       , desktopWidth / (mFrameX + 1), desktopHeight / (mFrameX + 1));
    }
    mFrameWndList.append(newFrame);
    mFrameCaptions.append("");
  }
  FrameWndAS& frameWnd = mFrameWndList[index];

  if (mFrameCaptions[index] != caption) {
    mFrameCaptions[index] = caption;
    frameWnd->SetCaption(caption);
  }
  if (stride <= 0) {
    stride = mStride;
  }
  if (index == 0) {
    lock.unlock();
    frameWnd->DrawWindow(frame->data(), imageType, save, mWidth, mHeight, stride, objData, objSize);
  } else {
//    for (auto itr = mDrawList.begin(); itr != mDrawList.end(); itr++) {
//      DrawStruct& info = *itr;
//      if (info.Index == index) {
//        mDrawList.erase(itr);
//        break;
//      }
//    }

    DrawStruct info;
    info.Index = index;
    info.ImageType = imageType;
    info.Frame = frame;
    info.Stride = stride;
    info.Save = save;
    mDrawList.append(info);
    WakeUp();
  }
#endif
}


DebugWnd::DebugWnd(const QList<QWidget*>& _WindowPool)
  : CtrlWorker(1000)
  , mFrameCount(6), mFrameX(2), mWidth(0), mHeight(0), mStride(0)
  , mWindowPool(_WindowPool)
{
}

DebugWnd::~DebugWnd()
{
}


