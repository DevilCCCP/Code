#pragma once

#include <QImage>
#include <QString>

#include <Lib/Include/Common.h>
#include <Lib/Net/Receiver.h>


DefineClassS(Decoder);

class ThumbnailReceiver: public QObject, public Receiver
{
  Decoder*    mDecoder;

  Q_OBJECT

public:
  /*override */virtual bool ReceiveMessage(NetMessageS& msg) override;
  /*override */virtual void OnDisconnected() override;

private:
  bool OnEncodedFrame(NetMessageS& msg);

signals:
  void OnThumbnailOk(const QImage& image);
  void OnThumbnailFail(const QString& msg);
  void OnThumbnailFrame(bool ok);
  void Disconnected();

public:
  ThumbnailReceiver(Decoder* _Decoder);
};

