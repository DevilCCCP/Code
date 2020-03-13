#include <QtGlobal>
#include <QNetworkAccessManager>
#include <QCryptographicHash>
#include <QEventLoop>
#include <QTimer>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QXmlStreamReader>
#include <QMutexLocker>
#include <math.h>

#include <Lib/Log/Log.h>

#include "PtzOnvif.h"

#ifndef QT_NO_DEBUG
//#define LOG_SOAP
#endif


const int kWaitPrepare = 5000;
const QString kTimeFormat("yyyy-MM-ddTHH:mm:ss.zzzZ");

bool PtzOnvif::DoInit()
{
  return true;
}

bool PtzOnvif::DoCircle()
{
  InitPtz();

  return true;
}

bool PtzOnvif::GetAbilities(int& _AbilityFlag)
{
  Q_UNUSED(_AbilityFlag);

  return false;
}

bool PtzOnvif::GetPosition(PtzVector& _Position)
{
  if (!PrepareMove()) {
    return false;
  }

  QByteArray xml = AddSecurity(mGetStatusXml);
  QMutexLocker lock(&mCmdMutex);
  if (!SendCommand("GetStatus", xml, true)) {
    return false;
  }

  if (!ParseStatus(_Position)) {
    Log.Warning(QString("PTZ: Parse position fail"));
    return false;
  }

  return true;
}

bool PtzOnvif::GetRange(PtzVector& _MinPosition, PtzVector& _MaxPosition, PtzVector& _MinSpeed, PtzVector& _MaxSpeed)
{
  if (!PrepareMove()) {
    return false;
  }

  _MinPosition = mMinPosition;
  _MaxPosition = mMaxPosition;
  _MinSpeed = mMinSpeed;
  _MaxSpeed = mMaxSpeed;
  return true;
}

bool PtzOnvif::GetHome(PtzVector& _HomePosition)
{
  Q_UNUSED(_HomePosition);

  return false;
}

bool PtzOnvif::SetPosition(const PtzVector& _Position)
{
  if (!PrepareMove()) {
    return false;
  }

  QByteArray xml = AddSecurity(mAbsoluteMoveXml)
      .replace("%%DATA%%", GetXmlVector("Position", _Position, false));
  QMutexLocker lock(&mCmdMutex);
  if (!SendCommand("AbsoluteMove", xml, true)) {
    return false;
  }

  lock.unlock();
  Log.Info(QString("PTZ set position: x: %1, y: %2, z: %3").arg(_Position.PosX).arg(_Position.PosY).arg(_Position.PosZ));
  return true;
}

bool PtzOnvif::SetPosition(const PtzVector& _Position, const PtzVector& _Speed)
{
  if (!PrepareMove()) {
    return false;
  }

  QByteArray xml = AddSecurity(mAbsoluteMoveXml)
      .replace("%%DATA%%", GetXmlVector("Position", _Position, false) + GetXmlVector("Speed", _Speed, true));
  QMutexLocker lock(&mCmdMutex);
  if (!SendCommand("AbsoluteMove", xml, true)) {
    return false;
  }

  lock.unlock();
  Log.Info(QString("PTZ set position: x: %1, y: %2, z: %3; v:(%4, %5, %6)")
           .arg(_Position.PosX).arg(_Position.PosY).arg(_Position.PosZ)
           .arg(_Speed.PosX).arg(_Speed.PosY).arg(_Speed.PosZ));
  return true;
}

bool PtzOnvif::RelativeMove(const PtzVector& _Position)
{
  if (!PrepareMove()) {
    return false;
  }

  QByteArray xml = AddSecurity(QByteArray(mRelativeMoveXml))
      .replace("%%DATA%%", GetXmlVector("Translation", _Position, true));
  QMutexLocker lock(&mCmdMutex);
  if (!SendCommand("RelativeMove", xml, true)) {
    return false;
  }
  lock.unlock();

  Log.Info(QString("PTZ relative move: x: %1, y: %2, z: %3").arg(_Position.PosX).arg(_Position.PosY).arg(_Position.PosZ));
  return true;
}

bool PtzOnvif::RelativeMove(const PtzVector& _Position, const PtzVector& _Speed)
{
  if (!PrepareMove()) {
    return false;
  }

  QByteArray xml = AddSecurity(mRelativeMoveXml)
      .replace("%%DATA%%", GetXmlVector("Translation", _Position, true) + GetXmlVector("Speed", _Speed, true));
  QMutexLocker lock(&mCmdMutex);
  if (!SendCommand("RelativeMove", xml, true)) {
    return false;
  }

  lock.unlock();
  Log.Info(QString("PTZ relative move: x: %1, y: %2, z: %3").arg(_Position.PosX).arg(_Position.PosY).arg(_Position.PosZ));
  return true;
}

bool PtzOnvif::ContinuousMove(const PtzVector& _Speed, int _Timeout)
{
  if (!PrepareMove()) {
    return false;
  }

  QByteArray xml = AddSecurity(mContinuousMoveXml)
      .replace("%%DATA%%", GetXmlVector("Velocity", _Speed, true) + GetXmlTimeout(_Timeout));
  QMutexLocker lock(&mCmdMutex);
  if (!SendCommand("ContinuousMove", xml, true)) {
    return false;
  }

  lock.unlock();
  Log.Info(QString("PTZ continuous move: x: %1, y: %2, z: %3").arg(_Speed.PosX).arg(_Speed.PosY).arg(_Speed.PosZ));
  return true;
}

bool PtzOnvif::StopMove()
{
  if (!PrepareMove()) {
    return false;
  }

  QByteArray xml = AddSecurity(mStopXml);
  QMutexLocker lock(&mCmdMutex);
  if (!SendCommand("Stop", xml, true)) {
    return false;
  }

  lock.unlock();
  Log.Info(QString("PTZ stop"));
  return true;
}

bool PtzOnvif::MoveHome()
{
  return false;
}

bool PtzOnvif::MoveHome(const PtzVector& _Speed)
{
  Q_UNUSED(_Speed);

  return false;
}

bool PtzOnvif::SetHome(const PtzVector& _Position)
{
  Q_UNUSED(_Position);

  return false;
}

bool PtzOnvif::InitPtz()
{
  if (mInit) {
    return true;
  }

  QMutexLocker lock(&mCmdMutex);
  if (mPtzUri.isEmpty()) {
    QByteArray xml = AddSecurity(mGetCapabilitiesXml);
    if (!SendCommand("GetCapabilities", xml, false)) {
      return false;
    }

    if (!ParseCapabilities()) {
      Log.Warning(QString("Can't find PTZ capability"));
      mPtzUri = mUri;
    }
  }

  if (mProfileToken.isEmpty()) {
    QByteArray xml = AddSecurity(mGetProfilesXml);
    if (!SendCommand("GetProfiles", xml, false)) {
      return false;
    }

    if (ParseProfiles()) {
      mGetStatusXml.replace("%%PROFILE%%", mProfileToken);
      mAbsoluteMoveXml.replace("%%PROFILE%%", mProfileToken);
      mRelativeMoveXml.replace("%%PROFILE%%", mProfileToken);
      mContinuousMoveXml.replace("%%PROFILE%%", mProfileToken);
      mCmdWait.wakeAll();
      mInit = true;
      return true;
    } else {
      Log.Error(QString("Can't find profily token"));
    }
  }
  return false;
}

bool PtzOnvif::PrepareMove()
{
  QMutexLocker lock(&mCmdMutex);
  if (!mProfileToken.isEmpty()) {
    return true;
  }

  return mCmdWait.wait(&mCmdMutex, kWaitPrepare);
}

bool PtzOnvif::TakeRespond(const char* name)
{
  if (!TakeElement("Envelope", true) || !TakeElement("Body", true) || !TakeElement(name, true)) {
    Log.Error(QString("PTZ respond '%1' bad syntax").arg(name));
    return false;
  }

  return true;
}

bool PtzOnvif::TakeElement(const char* name, bool warn)
{
  for (; mXmlReader->readNextStartElement(); mXmlReader->skipCurrentElement()) {
    if (mXmlReader->name() == name) {
      return true;
    }
  }

  if (warn) {
    Log.Warning(QString("PTZ respond element not found '%1'").arg(name));
  }
  return false;
}

bool PtzOnvif::ParseCapabilities()
{
  if (!TakeRespond("GetCapabilitiesResponse")) {
    return false;
  }

  if (TakeElement("Capabilities") && TakeElement("PTZ")) {
    for (; mXmlReader->readNextStartElement(); ) {
      if (mXmlReader->name() == "XAddr") {
        if (mXmlReader->readNext() == QXmlStreamReader::Characters) {
          mPtzUri = mXmlReader->text().toString();
          Log.Info(QString("PTZ uses uri: '%1'").arg(mPtzUri));
          return true;
        }
      } else {
        mXmlReader->skipCurrentElement();
      }
    }
  }
  return false;
}

bool PtzOnvif::ParseProfiles()
{
  if (!TakeRespond("GetProfilesResponse")) {
    return false;
  }

  for (; TakeElement("Profiles"); mXmlReader->skipCurrentElement()) {
    QStringRef token = mXmlReader->attributes().value("token");
    if (token.isEmpty()) {
      Log.Warning(QString("Profile token is invalid"));
      continue;
    }

    mProfileToken = token.toUtf8();
    Log.Info(QString("Using profile '%1'").arg(mProfileToken.constData()));
    ParseLimits();
    return true;
  }

  return false;
}

bool PtzOnvif::ParseLimits()
{
  if (!TakeElement("PTZConfiguration")) {
    return false;
  }

  for (; mXmlReader->readNextStartElement(); mXmlReader->skipCurrentElement()) {
    if (mXmlReader->name() == "PanTiltLimits") {
      ParseRanges(&mMinPosition.PosX, &mMaxPosition.PosX, &mMinPosition.PosY, &mMaxPosition.PosY);
    } else if (mXmlReader->name() == "ZoomLimits") {
      ParseRanges(&mMinPosition.PosZ, &mMaxPosition.PosZ);
    }
  }
  if (mMinPosition.PosZ < 0) {
    Log.Warning(QString("Z min range is negative, set to zero (z: %1)").arg(mMinPosition.PosZ));
    mMinPosition.PosZ = 0;
  }
  Log.Info(QString("PTZ: x:[%1, %2]; y:[%3, %4]; z: [%5, %6]")
           .arg(mMinPosition.PosX).arg(mMaxPosition.PosX)
           .arg(mMinPosition.PosY).arg(mMaxPosition.PosY)
           .arg(mMinPosition.PosZ).arg(mMaxPosition.PosZ));
  return true;
}

bool PtzOnvif::ParseStatus(PtzVector& position)
{
  if (!TakeRespond("GetStatusResponse") || !TakeElement("PTZStatus")) {
    return false;
  }

  for (; mXmlReader->readNextStartElement(); mXmlReader->skipCurrentElement()) {
    if (mXmlReader->name() == "Position") {
      return ParsePosition(position);
    }
  }

  return false;
}

bool PtzOnvif::ParsePosition(PtzVector& position)
{
  bool hasX = false;
  bool hasY = false;
  bool hasZ = false;
  for (; mXmlReader->readNextStartElement(); mXmlReader->skipCurrentElement()) {
    if (mXmlReader->name() == "PanTilt") {
      position.PosX = CoordToLocal(mXmlReader->attributes().value("x"), mMinPosition.PosX, mMaxPosition.PosX, 0, &hasX);
      position.PosY = CoordToLocal(mXmlReader->attributes().value("y"), mMinPosition.PosY, mMaxPosition.PosY, 0, &hasY);
      if (mFlipVert) {
        position.PosY = 1.0f - position.PosY;
      }
      if (mFlipHorz) {
        position.PosX = 1.0f - position.PosX;
      }
    } else if (mXmlReader->name() == "Zoom") {
      position.PosZ = CoordToLocal(mXmlReader->attributes().value("x"), mMinPosition.PosZ, mMaxPosition.PosZ, 0, &hasZ);
      if (mZoomScale != 1) {
        if (position.PosZ >= 0) {
          position.PosZ = pow(position.PosZ, (float)(mZoomScale));
        } else {
          position.PosZ = -pow(-position.PosZ, (float)(mZoomScale));
        }
      }
    }
  }

  if (!hasZ) {
    position.PosZ = PtzVector::NoValue();
  }
  Log.Info(QString("PTZ position: x: %1, y: %2, z: %3").arg(position.PosX).arg(position.PosY).arg(position.PosZ));
  return hasX && hasY;
}

bool PtzOnvif::ParseRanges(float* minX, float* maxX, float* minY, float* maxY)
{
  if (mXmlReader->readNextStartElement() && mXmlReader->name() == "Range") {
    for (; mXmlReader->readNextStartElement(); ) {
      if (mXmlReader->name() == "XRange") {
        if (!ParseRange(minX, maxX)) {
          return false;
        }
      } else if (mXmlReader->name() == "YRange" && minY && maxY) {
        if (!ParseRange(minY, maxY)) {
          return false;
        }
      } else {
        mXmlReader->skipCurrentElement();
      }
    }
    return true;
  }

  return false;
}

bool PtzOnvif::ParseRange(float* minX, float* maxX)
{
  bool hasMin = false;
  bool hasMax = false;
  for (; mXmlReader->readNextStartElement(); mXmlReader->skipCurrentElement()) {
    if (mXmlReader->name() == "Min") {
      if (mXmlReader->readNext() == QXmlStreamReader::Characters) {
        *minX = mXmlReader->text().toFloat(&hasMin);
      }
    } else if (mXmlReader->name() == "Max") {
      if (mXmlReader->readNext() == QXmlStreamReader::Characters) {
        *maxX = mXmlReader->text().toFloat(&hasMax);
      }
    }
  }
  return hasMin && hasMax;
}

QByteArray PtzOnvif::GetXmlVector(const char* name, const PtzVector& vect, bool relative)
{
  QByteArray result = QByteArray("      <tptz:") + name + ">\n";
  if (PtzVector::IsValid(vect.PosX) || PtzVector::IsValid(vect.PosY)) {
    float x = vect.PosX;
    float y = vect.PosY;
    if (mFlipVert) {
      y = (relative)? -y: 1.0f - y;
    }
    if (mFlipHorz) {
      x = (relative)? -x: 1.0f - x;
    }
    x = CoordToCamera(x, mMinPosition.PosX, mMaxPosition.PosX, relative);
    y = CoordToCamera(y, mMinPosition.PosY, mMaxPosition.PosY, relative);
    result.append("        <tt:PanTilt x=\"" + QByteArray::number(x, 'f', 3)
                  + "\" y=\"" + QByteArray::number(y, 'f', 3) + "\"/>\n");
  }
  if (PtzVector::IsValid(vect.PosZ)) {
    float z = vect.PosZ;
    if (mZoomScale != 1) {
      if (z >= 0) {
        z = pow(z, (float)(1.0/mZoomScale));
      } else {
        z = -pow(-z, (float)(1.0/mZoomScale));
      }
    }
    z = CoordToCamera(z, mMinPosition.PosZ, mMaxPosition.PosZ, relative);
    result.append("        <tt:Zoom x=\"" + QByteArray::number(z, 'f', 3) + "\"/>\n");
  }
  result.append(QByteArray("      </tptz:") + name + ">\n");
  return result;
}

QByteArray PtzOnvif::GetXmlTimeout(int timeout)
{
  Log.Warning(QString("PTZ Timeout field unused (timeout: %1)").arg(timeout));

  return QByteArray();
}

QByteArray PtzOnvif::AddSecurity(const QByteArray& xml)
{
  QByteArray security = mSecurityXml;
  security.replace("%%USER%%", mLogin.toUtf8());
  for (int i = 0; i < mNonce.size(); i++) {
    if (++(mNonce.data()[i])) {
      break;
    }
  }
  security.replace("%%NONCE%%", mNonce.toBase64());
  QByteArray date = QDateTime::currentDateTimeUtc().toString(kTimeFormat).toUtf8();
  security.replace("%%TIME%%", date);

  QCryptographicHash hash(QCryptographicHash::Sha1);
  hash.addData(mNonce + date + mPassword.toUtf8());
  QByteArray passHash = hash.result().toBase64();
  security.replace("%%PASSWORD%%", passHash);

  QByteArray secured = xml;
  secured.replace("%%SECURITY%%", security);
  return secured;
}

bool PtzOnvif::SendXml()
{

  Log.Info(QString("Send EWS ok"));
  return true;
}

bool PtzOnvif::SendCommand(const QString& cmd, const QByteArray& xml, bool ptz)
{
#ifdef LOG_SOAP
  QFile debugFile1(".onvif_in");
  if (debugFile1.open(QFile::WriteOnly)) {
    debugFile1.write(xml);
    Log.Trace(QString("Request store in file '%1'").arg(debugFile1.fileName()));
  }
#endif

  QScopedPointer<QNetworkAccessManager> m(mNetManager = new QNetworkAccessManager());
  QScopedPointer<QEventLoop> e(mEventLoop  = new QEventLoop());

  QNetworkRequest request = QNetworkRequest(QUrl(ptz? mPtzUri: mUri));
  request.setHeader(QNetworkRequest::ContentTypeHeader
                    , QStringLiteral("application/soap+xml; charset=utf-8; action=\"http://www.onvif.org/ver10/media/wsdl/") + cmd + "\"");
  request.setHeader(QNetworkRequest::ContentLengthHeader, xml.size());
  request.setRawHeader("Connection", "Close");
//  request.setRawHeader("Accept-Encoding", "gzip, deflate");

  QNetworkReply* netReply = mNetManager->post(request, xml);
  QObject::connect(netReply, &QNetworkReply::finished, mEventLoop, &QEventLoop::quit);
//  QObject::connect(netReply, &QNetworkReply::finished, this, &PtzOnvif::OnCommandFinish);
//  QObject::connect(mNetManager, &QNetworkAccessManager::authenticationRequired, &auth, &PtzOnvif::AuthenticationRequired);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
  QTimer::singleShot(15000, mEventLoop, &QEventLoop::quit);
#else
  QTimer::singleShot(15000, mEventLoop, SLOT(quit()));
#endif
  mEventLoop->exec();
  //do {
  //  mEventLoop->processEvents();
  //} while (!netReply->isFinished());

  if (netReply->error() != QNetworkReply::NoError) {
    Log.Warning(QString("Send PTZ cmd fail (cmd: '%1', err: '%2')").arg(cmd).arg(netReply->errorString()));
    return false;
  }

  int retCode = netReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  if (retCode != 200) {
    Log.Info(QString("Send PTZ cmd rejected (cmd: '%1', code: %2)").arg(cmd).arg(retCode));
    return false;
  }
  mResultXml = netReply->readAll();
  mXmlReaderS.reset(mXmlReader = new QXmlStreamReader(mResultXml));

#ifdef LOG_SOAP
  QFile debugFile2(".onvif_out");
  if (debugFile2.open(QFile::WriteOnly)) {
    debugFile2.write(mResultXml);
    Log.Trace(QString("Respond store in file '%1'").arg(debugFile2.fileName()));
  }
#endif

  Log.Trace(QString("Send '%1' ok").arg(cmd));
  return true;
}

bool PtzOnvif::ReadResourceFile(const char* fileName, QByteArray& fileData)
{
  QFile file(QString(":/Ptz/%1").arg(fileName));
  if (!file.open(QFile::ReadOnly)) {
    Log.Warning(QString("Resource file open fail (name: '%1', err: '%2')").arg(fileName).arg(file.errorString()));
    return false;
  }
  fileData = file.readAll();
  if (fileData.isEmpty()) {
    Log.Warning(QString("Resource file is empty (name: '%1', err: '%2')").arg(fileName).arg(file.errorString()));
    return false;
  }

  return true;
}

float PtzOnvif::CoordToLocal(const QStringRef& text, float valueMin, float valueMax, float valueDefault, bool* ok)
{
  bool hasX;
  qreal x = text.toFloat(&hasX);
  if (hasX) {
    x = (x - valueMin) / (valueMax - valueMin);
  } else {
    x = valueDefault;
  }
  if (ok) {
    *ok = hasX;
  }
  return x;
}

float PtzOnvif::CoordToCamera(float value, float valueMin, float valueMax, bool relative)
{
  return relative? CoordToCameraRelative(value, valueMin, valueMax): CoordToCameraAbsolut(value, valueMin, valueMax);
}

float PtzOnvif::CoordToCameraAbsolut(float value, float valueMin, float valueMax)
{
  float coord = valueMin + (valueMax - valueMin) * value;
  return qMin(qMax(valueMin, coord), valueMax);
}

float PtzOnvif::CoordToCameraRelative(float value, float valueMin, float valueMax)
{
  float coordRange = valueMax - valueMin;
  float coord = coordRange * value;
  return qMin(qMax(-coordRange, coord), coordRange);
}

void PtzOnvif::OnCommandFinish()
{
}


PtzOnvif::PtzOnvif(SettingsA& settings)
  : mInit(false)
  , mXmlReader(nullptr)
{
  Q_INIT_RESOURCE(Ptz);

  mUri = settings.GetMandatoryValue("Uri", true).toString();
  mFlipVert = settings.GetValue("VFlip", false).toBool();
  mFlipHorz = settings.GetValue("HFlip", false).toBool();
  mZoomScale = settings.GetValue("ZoomScale", 1).toReal();
  mLogin = settings.GetValue("Login", "admin").toString();
  mPassword = settings.GetValue("Password", "admin").toString();
  qsrand(QDateTime::currentMSecsSinceEpoch());
  for (int i = 0; i < 16; i++) {
    mNonce.append((char)qrand());
  }
  if (mZoomScale < 0) {
    mZoomScale = 1.0 / (-mZoomScale);
  }
  if (mZoomScale < 0.01) {
    mZoomScale = 0.01;
    Log.Warning(QString("Zoom scale is too low, fix to %1").arg(mZoomScale));
  }
  if (mZoomScale > 100.0) {
    mZoomScale = 100.0;
    Log.Warning(QString("Zoom scale is too big, fix to %1").arg(mZoomScale));
  }

  ReadResourceFile("GetCapabilities.xml", mGetCapabilitiesXml);
  ReadResourceFile("GetProfiles.xml", mGetProfilesXml);
  ReadResourceFile("GetStatus.xml", mGetStatusXml);
  ReadResourceFile("AbsoluteMove.xml", mAbsoluteMoveXml);
  ReadResourceFile("ContinuousMove.xml", mContinuousMoveXml);
  ReadResourceFile("RelativeMove.xml", mRelativeMoveXml);
  ReadResourceFile("Stop.xml", mStopXml);
  ReadResourceFile("Vector.xml", mVectorXml);
  ReadResourceFile("Security.xml", mSecurityXml);
}

PtzOnvif::~PtzOnvif()
{
}

