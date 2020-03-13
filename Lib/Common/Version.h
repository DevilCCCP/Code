#pragma once

#include <QString>

#include <Lib/Include/Common.h>


class Version
{
  static QString    mLocalVersion;

  PROPERTY_GET(int, MajorVersion)
  PROPERTY_GET(int, MinorVersion)
  PROPERTY_GET(int, Revision)
  ;
public:
  static bool InitLocalVersion();
  static QString LocalVersion();

  bool LoadFromString(const QString& textVersion);
  bool LoadFromPack(const QString& packPath);
  bool LoadFromThis();

  bool operator!() const { return IsEmpty(); }
  bool operator==(const Version& other) const { return Eq(other); }
  bool operator!=(const Version& other) const { return !Eq(other); }
  bool operator<(const Version& other) const { return Less(other); }
  bool operator>(const Version& other) const { return other.Less(*this); }
  bool operator<=(const Version& other) const { return !other.Less(*this); }
  bool operator>=(const Version& other) const { return !Less(other); }
  QString ToString() const;
  QString ToFilename() const;

private:
  bool IsEmpty() const;
  bool Eq(const Version& other) const;
  bool Less(const Version& other) const;

public:
  Version();
  Version(int _Major);
  Version(int _Major, int _Minor);
  Version(int _Major, int _Minor, int Revision);
};
