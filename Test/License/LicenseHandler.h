#pragma once

#include <Lib/NetServer/HttpHandler.h>


class LicenseHandler: public HttpHandler
{
protected:
  /*override */virtual bool Post(const QString& path, const QList<QByteArray>& params, const QList<File>& files) override;

public:
  LicenseHandler();
};
