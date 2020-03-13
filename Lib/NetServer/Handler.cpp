#include <QMutexLocker>
#include <QIODevice>

#include <Lib/Log/Log.h>

#include "Handler.h"


void Handler::SendFile(QIODeviceS& _SendFile, int _TrafficLimit)
{
  QMutexLocker lock (&mSendMutex);
  if (mSendFile) {
    Log.Warning(QString("Send file while other not sent (override old sent)"));
  }
  mSendFile = _SendFile;
  mTrafficLimit = _TrafficLimit;
  mTrafficCalc.Reset();
}

bool Handler::DoCircle()
{
  return false;
}

bool Handler::Receive(SyncSocket* socket, bool& sendDone)
{
  QByteArray data;
  if (socket->ReadData(data)) {
    QString text = QString::fromUtf8(data.constData(), data.size());
    if (mDebug) {
      Log.Info(QString("Client '%1' get data:\n%2").arg(GetConnectionUri()->ToString()).arg(text));
    }
    sendDone = true;
  }
  return true;
}

void Handler::OnDisconnected()
{
}

void Handler::SendData(const QByteArray& data)
{
  QMutexLocker lock (&mSendMutex);
  mSendData.append(data);
}

void Handler::ClearData()
{
  QMutexLocker lock (&mSendMutex);
  mSendData.clear();
}

bool Handler::TakeData(QByteArray& data)
{
  QMutexLocker lock(&mSendMutex);
  if (!mSendData.isEmpty()) {
    data = mSendData.takeFirst();
    return true;
  } else if (mSendFile) {
    mTrafficCalc.UpdateFrame();
    if (mTrafficCalc.GetTraffic() < mTrafficLimit) {
      data = mSendFile->read(16 * 1024);
      if (int size = data.size()) {
        mTrafficCalc.AddFrame(size);
        return true;
      } else {
        mSendFile.clear();
        return false;
      }
    } else {
      data.clear();
      return true;
    }
  }

  return false;
}


Handler::Handler()
  : mDebug(false)
{
}

Handler::~Handler()
{
}
