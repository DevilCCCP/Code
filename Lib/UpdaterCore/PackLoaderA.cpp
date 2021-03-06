#include "PackLoaderA.h"


bool PackLoaderA::FindPack()
{
  if (mInit) {
    return true;
  }

  do {
    QByteArray ver;
    if (LoadVer(ver)) {
      break;
    }
    QString baseUri = getUri();
    if (!baseUri.endsWith('/')) {
      baseUri.append('/');
    }

#ifdef Q_OS_WIN32
    setUri(baseUri + "windows/");
#elif defined(Q_OS_LINUX)
    setUri(baseUri + "linux/");
#else
    setUri(baseUri + "other/");
#endif
    if (LoadVer(ver)) {
      break;
    }

    QFile settings(GetVarFile("os"));
    if (settings.open(QFile::ReadOnly | QFile::Text)) {
      QString os = QString::fromUtf8(settings.readAll());
      if (!os.isEmpty()) {
        if (!os.endsWith('/')) {
          os.append('/');
        }
        setUri(baseUri + os);
        if (LoadVer(ver)) {
          break;
        }
      }
    }

    setUri(baseUri);
    return false;
  } while (false);

  Log.Info(QString("Using update location '%1'").arg(getUri()));
  mInit = true;
  return true;
}


PackLoaderA::PackLoaderA()
  : mInit(false)
{
}

PackLoaderA::~PackLoaderA()
{
}

