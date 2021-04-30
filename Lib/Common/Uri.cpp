#include "Uri.h"


QString Uri::ToString() const
{
  switch (mType) {
  case eTcp: return QString("tcp::%1:%2").arg(mHost).arg(mPort);
  case eUdp: return QString("udp::%1:%2").arg(mHost).arg(mPort);
  case eTypeIllegal:
    break;
  }
  return QString();
}

Uri Uri::FromString(QString uriText)
{
  Uri uri;
  if (uriText.size() >= 5) {
    switch (uriText[0].toLatin1()) {
    case 't':
      if (uriText.startsWith("tcp::")) {
        uri.Parse(eTcp, uriText.right(uriText.size() - 5));
      }
      break;

    case 'u':
      if (uriText.startsWith("udp::")) {
        uri.Parse(eUdp, uriText.right(uriText.size() - 5));
      }
      break;
    }
  }
  return uri;
}

Uri Uri::FromIpv4(const QHostAddress& host, int port)
{
  Uri uri;
  uint addr = host.toIPv4Address();
  uint a1 = addr >> 24;
  uint a2 = ((addr >> 16) & 0xff);
  uint a3 = ((addr >> 8) & 0xff);
  uint a4 = (addr & 0xff);
  uri.mType = eTcp;
  uri.mHost = QString("%1.%2.%3.%4").arg(a1).arg(a2).arg(a3).arg(a4);
  uri.mPort = port;
  return uri;
}

bool Uri::operator==(const Uri& other) const
{
  if (mType == other.mType) {
    switch (mType) {
    case eTcp:
    case eUdp: return mHost == other.mHost && mPort == other.mPort;
    case eTypeIllegal:
    default: return true;
    }
  }
  return false;
}

bool Uri::operator<(const Uri& other) const
{
  if (mType == other.mType) {
    switch (mType) {
    case eTcp:
    case eUdp: return (mHost < other.mHost)? true: mPort < other.mPort;
    case eTypeIllegal:
    default: return false;
    }
  } else {
    return mType < other.mType;
  }
}

Uri& Uri::operator=(const Uri& other)
{
  mType = other.mType;
  switch (mType) {
  case eTcp: case eUdp:
    mHost = other.mHost;
    mPort = other.mPort;
    break;

  case eTypeIllegal:
  default:
    break;
  }
  return *this;
}

bool Uri::Parse(Uri::EType type, const QString& text)
{
  QStringList pair = text.split(':');
  if (pair.size() == 2) {
    mHost = pair[0];
    bool ok;
    mPort = pair[1].toInt(&ok);
    if (ok) {
      mType = type;
      return true;
    }
  }
  return false;
}


Uri::Uri()
  : mType(eTypeIllegal), mPort(0)
{
}

Uri::Uri(Uri::EType _Type)
  : mType(_Type), mPort(0)
{
}

Uri::Uri(Uri::EType _Type, const QString& _Host, int _Port)
  : mType(_Type), mHost(_Host), mPort(_Port)
{
}
