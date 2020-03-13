#include <QSslSocket>
#include <QTcpSocket>

#include <Lib/Log/Log.h>

#include "SyncSocket.h"

const int kWorkCircleMs = 100;
const int kTimeoutWarningMs = 2000;
const int kTimeoutReadWriteMs = 60000;
const int kTimeoutEncryptionMs = 30000;


QString SyncSocket::SocketErrorString()
{
  return mTcpSocket->errorString();
}

QHostAddress SyncSocket::PeerAddress()
{
  return mTcpSocket->peerAddress();
}

bool SyncSocket::SslWait()
{
#ifndef QT_NO_SSL
  QSslSocket* sslSocket = dynamic_cast<QSslSocket*>(mTcpSocket.data());
  if (!sslSocket) {
    Log.Warning(QString("Try SSL handshake over non-SSL socket"));
    return true;
  }

  QElapsedTimer timer;
  timer.start();
  bool warn = false;
  while (!sslSocket->waitForEncrypted(kWorkCircleMs)) {
    if (!SayWork()) {
      return false;
    }
    if (timer.elapsed() > kTimeoutWarningMs) {
      if (timer.elapsed() > kTimeoutEncryptionMs) {
        Log.Error(QString("Wait encryption timeout"));
        return false;
      }
      if (!warn) {
        Log.Warning(QString("Wait encryption for too long"));
        warn = true;
      }
    }
  }
#endif
  return true;
}

bool SyncSocket::ReadyRead(int size)
{
  if (mTcpSocket->state() != QAbstractSocket::ConnectedState) {
    mError = true;
    return false;
  }

  if (mTcpSocket->bytesAvailable() < size) {
    bool ready = mTcpSocket->waitForReadyRead(mReadCircleMs);

    if (!ready) {
      if (!mTcpSocket->isReadable()) {
        Log.Warning(QString("Connection unreadable (uri: '%1', error: '%2', state: %3)")
                    .arg(GetInfo()).arg(mTcpSocket->errorString()).arg(mTcpSocket->state()));
        mError = true;
        return false;
      } else if (mTcpSocket->state() != QAbstractSocket::ConnectedState) {
        if (mTcpSocket->state() != QAbstractSocket::UnconnectedState) {
          Log.Warning(QString("Connection read state fail (uri: '%1', error '%2', state: %3)")
                      .arg(GetInfo()).arg(mTcpSocket->errorString()).arg(mTcpSocket->state()));
        }
        mError = true;
        return false;
      } else if (!mTcpSocket->isValid()) {
        Log.Warning(QString("Connection read invalid (uri: '%1', error '%2')")
                    .arg(GetInfo()).arg(mTcpSocket->errorString()));
        mError = true;
        return false;
      }
      return false;
    }
  }

  return mTcpSocket->bytesAvailable() >= size;
}

bool SyncSocket::ReadData(QByteArray& buffer, int maxSize)
{
  int hasSize = buffer.size();
  if (hasSize >= maxSize) {
    return true;
  }
  if (!ReadyRead()) {
    return false;
  }

  int addSize = mTcpSocket->bytesAvailable();
  if (hasSize + addSize > maxSize) {
    addSize = maxSize - hasSize;
  }
  buffer.resize(hasSize + addSize);
  if (addSize == mTcpSocket->read(buffer.data() + hasSize, addSize)) {
    return true;
  } else {
    Log.Warning(QString("Read data fail (uri: '%1', error: '%2')").arg(GetInfo()).arg(mTcpSocket->errorString()));
    mError = true;
    return false;
  }
}

bool SyncSocket::ReadData(char *buffer, int size)
{
  if (size <= 0) {
    return true;
  }

  if (!ReadyRead(size)) {
    return false;
  }
  int read = (int)mTcpSocket->read(buffer, size);
  if (size == read) {
    return true;
  } else {
    Log.Warning(QString("Read data fail (uri: '%1', error: '%2')").arg(GetInfo()).arg(mTcpSocket->errorString()));
    mError = true;
    return false;
  }
}

bool SyncSocket::WriteData(const QByteArray &buffer, bool sync)
{
  if (mTcpSocket->state() != QAbstractSocket::ConnectedState) {
    return false;
  }
//  Log.Trace(QString("WriteData '%1 %2 %3 %4' (size: %5)").arg((int)(byte)buffer.at(0), 2, 16).arg((int)(byte)buffer.at(1), 2, 16)
//            .arg((int)(byte)buffer.at(2), 2, 16).arg((int)(byte)buffer.at(3), 2, 16).arg(buffer.size()));
  if (buffer.size() != mTcpSocket->write(buffer)) {
    if (mTcpSocket->error() != QAbstractSocket::RemoteHostClosedError) {
      Log.Warning(QString("Write data fail (uri: '%1', error: '%2')").arg(GetInfo()).arg(mTcpSocket->errorString()));
    }
    return false;
  }
  return !sync || WriteSync();
}

bool SyncSocket::WriteSync()
{
  QElapsedTimer timer;
  timer.start();
  bool warning = false;
  while (mTcpSocket->bytesToWrite() > 0) {
    bool ok = mTcpSocket->waitForBytesWritten(kWorkCircleMs);
    if (!SayWork()) {
      return false;
    }
    if (!ok) {
      if (!mTcpSocket->isValid()) {
        Log.Warning(QString("Connection write invalid (uri: '%1', error: '%2')")
                    .arg(GetInfo()).arg(mTcpSocket->errorString()));
        return false;
      } else if (mTcpSocket->state() != QAbstractSocket::ConnectedState) {
        if (mTcpSocket->error() != QAbstractSocket::RemoteHostClosedError) {
          Log.Warning(QString("Connection write fail (uri: '%1', error: '%2', state: %3)")
                      .arg(GetInfo()).arg(mTcpSocket->errorString()).arg(mTcpSocket->state()));
        }
        return false;
      }
    }

    if (timer.elapsed() >= kTimeoutWarningMs && !warning) {
      Log.Warning(QString("Connection write wait too long (uri: '%1', error: '%2')")
                  .arg(GetInfo()).arg(mTcpSocket->errorString()));
      warning = true;
    }
    if (timer.elapsed() >= kTimeoutReadWriteMs) {
      Log.Warning(QString("Connection aborting due to 'too long sending' (uri: '%1', error: '%2')")
                  .arg(GetInfo()).arg(mTcpSocket->errorString()));
      return false;
    }
  }
  return true;
}

bool SyncSocket::DisconnectSync()
{
  QElapsedTimer timer;
  timer.start();
  bool warning = false;
  while (mTcpSocket->state() != QTcpSocket::UnconnectedState && !mTcpSocket->waitForDisconnected(kWorkCircleMs)) {
    if (!SayWork()) {
      return false;
    }

    if (timer.elapsed() >= kTimeoutWarningMs && !warning) {
      Log.Warning(QString("Disconnect wait too long (uri: '%1', error: '%2')")
                  .arg(GetInfo()).arg(mTcpSocket->errorString()));
      warning = true;
    }
    if (timer.elapsed() >= kTimeoutReadWriteMs) {
      Log.Warning(QString("Disconnect aborting (uri: '%1', error: '%2')")
                  .arg(GetInfo()).arg(mTcpSocket->errorString()));
      return false;
    }
  }
  return true;
}

bool SyncSocket::IsConnected()
{
  return mTcpSocket->state() == QTcpSocket::ConnectedState;
}

void SyncSocket::CloseSocket()
{
  if (mTcpSocket) {
    if (IsConnected()) {
      mTcpSocket->disconnectFromHost();
      DisconnectSync();
    }
    mTcpSocket->close();
    mTcpSocket.clear();
  }
}


SyncSocket::SyncSocket()
  : CtrlWorker(1)
  , mReadCircleMs(1), mError(false)
{
  setFastThread(true);
}

SyncSocket::~SyncSocket()
{
}

