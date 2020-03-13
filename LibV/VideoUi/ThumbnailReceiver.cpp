#include <QImage>

#include <Lib/Net/NetMessage.h>
#include <LibV/Include/VideoMsg.h>
#include <LibV/VideoUi/CameraForm.h>
#include <LibV/Decoder/Decoder.h>

#include "ThumbnailReceiver.h"


bool ThumbnailReceiver::ReceiveMessage(NetMessageS& msg)
{
  switch (msg->GetMessageType()) {
  case eMsgThumbnailOk:
    emit OnThumbnailOk(QImage::fromData((const uchar*)msg->GetMessageConstData(), msg->GetMessageDataSize()));
    return false;

  case eMsgThumbnailNo:
    emit OnThumbnailFail(QString::fromUtf8("Нет изображения"));
    return false;

  case eMsgOneFrame:
    emit OnThumbnailFrame(OnEncodedFrame(msg));
    return false;

  case eMsgVideoFrame:
    emit OnThumbnailFrame(OnEncodedFrame(msg));
    return false;

  default:
    emit OnThumbnailFail(QString::fromUtf8("Ошибка"));
    return true;
  }
}

void ThumbnailReceiver::OnDisconnected()
{
  emit OnThumbnailFail(QString::fromUtf8("Нет соединения"));
  emit Disconnected();
}

bool ThumbnailReceiver::OnEncodedFrame(NetMessageS& msg)
{
  FrameS frame(new Frame());
  frame->ReserveData(msg->GetMessageDataSize());
  memcpy(frame->Data(), msg->GetMessageConstData(), msg->GetMessageDataSize());

  if (frame->GetHeader()->HeaderSize == sizeof(Frame::Header)) {
    mDecoder->PushFrame(frame);
    return true;
  } else {
    return false;
  }
}


ThumbnailReceiver::ThumbnailReceiver(Decoder* _Decoder)
  : mDecoder(_Decoder)
{
}

