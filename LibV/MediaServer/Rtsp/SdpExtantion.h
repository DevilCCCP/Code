#pragma once

#include <Lib/Include/Common.h>


DefineClassS(SdpExtantion);

class /*interface */SdpExtantion
{
public:
  /*new */virtual bool SdpExtraInit(const QString& mediaName, QByteArray& sdpText) = 0;
};

