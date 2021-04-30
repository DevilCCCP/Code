#include <Lib/Log/Log.h>
#include <Lib/Dispatcher/Overseer.h>
#include <Lib/Ui/WidgetImageR.h>
#include <LibV/Decoder/Decoder.h>
#include <LibV/VideoUi/DecodeReceiver.h>

#include "QtRender.h"


bool QtRender::Init()
{
  if (mUseDecoder) {
    mDecoder.reset(new Decoder(false, false, eRawRgba, 0, false));
    mOverseer->RegisterWorker(mDecoder);
  }
  mDecodeReceiver.reset(new DecodeReceiver(true));
  mOverseer->RegisterWorker(mDecodeReceiver);
  if (mUseDecoder) {
    mDecoder->ConnectModule(mDecodeReceiver.data());
  }
  return true;
}

bool QtRender::SetRegion(const QRect& srcRegion, const QRect& destRegion)
{
  mSourceRect = srcRegion;
  mDestRect   = destRegion;
  return true;
}

bool QtRender::SetWidget(QWidget* destWidget)
{
  if (mParentWidget != destWidget) {
    mParentWidget = destWidget;

    if (mDrawWidget) {
      mDrawWidget->deleteLater();
    }
    if (!mParentWidget) {
      mDrawWidget = nullptr;
      return true;
    }
    mDrawWidget = new WidgetImageR(mParentWidget);
    mDrawWidget->setObjectName(QStringLiteral("widgetDraw"));
  }

  if (mDestRect.isNull()) {
    mDrawWidget->setGeometry(QRect(QPoint(0, 0), QSize(mParentWidget->size())));
  } else {
    mDrawWidget->setGeometry(mDestRect);
  }
  Log.Info(QString("Render to widget rect: (%1, %2, %3, %4)").arg(mDrawWidget->geometry().x()).arg(mDrawWidget->geometry().y())
           .arg(mDrawWidget->geometry().width()).arg(mDrawWidget->geometry().height()));
  mDrawWidget->show();

  QObject::connect(mDecodeReceiver.data(), &DecodeReceiver::OnDecodedImage, mDrawWidget, &QWidgetB::SetBackImage);
  return true;
}

bool QtRender::SetPause(bool paused)
{
  Q_UNUSED(paused);

  return true;
}

void QtRender::Release()
{
  if (mDrawWidget) {
    mDecodeReceiver->disconnect(mDrawWidget);
    mDrawWidget->deleteLater();
    mDrawWidget = nullptr;
  }
  if (mDecoder) {
    mDecoder->Stop();
  }
  if (mDecodeReceiver) {
    mDecodeReceiver->Stop();
  }
}

void QtRender::SetSource(Conveyor* source)
{
  if (mUseDecoder) {
    source->ConnectModule(mDecoder.data());
  } else {
    source->ConnectModule(mDecodeReceiver.data());
  }
  if (mDecodeReceiver) {
    mDecodeReceiver->SetPause(false);
  }
}

bool QtRender::PlayFrame(const FrameS& frame)
{
  if (mParentWidget) {
    if (mUseDecoder) {
      mDecoder->PushFrame(frame);
    } else {
      mDecodeReceiver->PushFrame(frame);
    }
  }
  return true;
}

void QtRender::ReleaseSource(Conveyor* source)
{
  if (mUseDecoder) {
    source->DisconnectModule(mDecoder.data());
  } else {
    source->DisconnectModule(mDecodeReceiver.data());
  }
  if (mDecodeReceiver) {
    mDecodeReceiver->SetPause(true);
  }
}

void QtRender::ClearImage()
{
  if (mDrawWidget) {
    mDrawWidget->SetBackImage(QImage());
  }
}

void QtRender::ConnectConsumer(CtrlWorker* consumer)
{
  if (mUseDecoder) {
    mDecoder->ConnectModule(consumer);
  } else {
    mDecodeReceiver->ConnectModule(consumer);
  }
}


QtRender::QtRender(const OverseerS& _Overseer, bool _UseDecoder)
  : mOverseer(_Overseer), mUseDecoder(_UseDecoder)
  , mParentWidget(nullptr), mDrawWidget(nullptr)
{
}

QtRender::~QtRender()
{
  Release();
}
