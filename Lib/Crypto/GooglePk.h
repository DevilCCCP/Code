#pragma once

#include <QByteArray>

#include <Lib/Include/Common.h>


DefineClassS(Rsa);

class GooglePk
{
public:
  static RsaS FromText(const QByteArray& pkText, QString& errorText);

private:
  GooglePk();
};

