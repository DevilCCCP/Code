#include <Lib/Log/Log.h>
#include <Lib/Dispatcher/Overseer.h>
#include <Lib/Ui/QWidgetB.h>
#include <LibV/Decoder/Decoder.h>
#include <LibV/VideoUi/DecodeReceiver.h>

#include "QtRender.h"


bool QtRender::Init()
{
  mDecoder.reset(new Decoder(false, false, eRawRgba));
  mOverseer->RegisterWorker(mDecoder);
  mDecodeReceiver.reset(new DecodeReceiver(true));
  mOverseer->RegisterWorker(mDecodeReceiver);
  mDecoder->ConnectModule(mDecodeReceiver.data());
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
  mParentWidget = destWidget;
  if (mDrawWidget) {
    mDrawWidget->deleteLater();
  }
  mDrawWidget = new QWidgetB(mParentWidget);
  mDrawWidget->setObjectName(QStringLiteral("widgetDraw"));
  if (mDestRect.isNull()) {
    mDrawWidget->setGeometry(QRect(QPoint(0, 0), QSize(mParentWidget->size())));
  } else {
    mDrawWidget->setGeometry(mDestRect);
  }
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
  source->ConnectModule(mDecoder.data());
}

bool QtRender::PlayFrame(const FrameS& frame)
{
  mDecoder->PushFrame(frame);
  return true;
}


QtRender::QtRender(const OverseerS& _Overseer)
  : mOverseer(_Overseer)
  , mParentWidget(nullptr), mDrawWidget(nullptr)
{
}

QtRender::~QtRender()
{
  Release();
}
