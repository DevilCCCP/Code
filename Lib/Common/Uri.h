#pragma once
#include <QString>
#include <QStringList>
#include <QHostAddress>

#include <Lib/Include/Common.h>


class Uri
{
public:
  enum EType {
    eTcp,
    eUdp,
    eTypeIllegal
  };

private:
  EType        mType;
  QString      mHost;
  int          mPort;

public:
  EType Type() const { return mType; }
  QString& Host() { return mHost; }
  const QString& Host() const { return mHost; }
  int& Port() { return mPort; }
  const int& Port() const { return mPort; }

public:
  QString ToString() const;
  static Uri FromString(QString uriText);
  static Uri FromIpv4(const QHostAddress& host, int port);

  bool operator==(const Uri& other) const;
  bool operator<(const Uri& other) const;

private:
  bool Parse(EType type, const QString& text);

public:
  Uri();
  Uri(EType _Type);
  Uri(EType _Type, const QString& _Host, int _Port);
};
