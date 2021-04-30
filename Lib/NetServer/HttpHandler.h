#pragma once

#include <QMap>
#include "Handler.h"


struct File
{
  QString    Type;
  QString    Name;
  QByteArray Data;
};

typedef QMap<QByteArray, QByteArray> HeaderMap;
typedef QMap<QByteArray, QByteArray> CookiesMap;
typedef QSharedPointer<QIODevice> QIODeviceS;

class HttpHandler : public Handler
{
  SyncSocket*       mSocket;
  bool              mLogRequest;
  bool              mLogRespond;

  QByteArray        mRequest;
  QByteArray        mCommand;
  QByteArray        mVersion;
  QByteArray        mPath;
  QList<QByteArray> mParams;
  QList<File>       mFiles;
  HeaderMap         mHeaders;
  CookiesMap        mCookies;
  bool              mKeepAlive;

  QByteArray        mAnswer;
  QIODeviceS        mResultFile;
  int               mFileSpeedLimit;

  enum EParseState {
    eNotStart,
    eHeader,
    eMultyPart
  };

  EParseState       mParseState;
  int               mHeaderSize;
  bool              mMultyPart;
  int               mMultyPartPos;
  int               mMultyPartSize;
  QByteArray        mMultyPartBoundary;

protected:
  const QByteArray& Version() const { return mVersion; }
  const QByteArray& Path()    const { return mPath; }
  const HeaderMap&  Headers() const { return mHeaders; }
  const CookiesMap& Cookies() const { return mCookies; }
  const QByteArray& Request() const { return mRequest; }
  const QList<QByteArray>& Params() const { return mParams; }

  QByteArray& Answer()     { return mAnswer; }
  QIODeviceS& ResultFile() { return mResultFile; }

  void SetKeepAlive(bool _KeepAlive) { mKeepAlive = _KeepAlive; }
  void SetFileSpeedLimit(int _FileSpeedLimit) { mFileSpeedLimit = _FileSpeedLimit; }
  void SetLogRequest(bool _LogRequest) { mLogRequest = _LogRequest; }
  void SetLogRespond(bool _LogRespond) { mLogRespond = _LogRespond; }
  void SetLogAll(bool _LogAll) { mLogRequest = _LogAll; mLogRespond = _LogAll; }
  bool IsLogAny() { return mLogRequest || mLogRespond; }

protected:
  /*override */virtual bool Receive(SyncSocket* socket, bool& sendDone) Q_DECL_OVERRIDE;
//  /*override */virtual void OnDisconnected() Q_DECL_OVERRIDE;

protected:
  /*new */virtual const char* Protocol();
  /*new */virtual bool Get(const QString& path, const QList<QByteArray>& params);
  /*new */virtual bool Post(const QString& path, const QList<QByteArray>& params, const QList<File>& files);
  /*new */virtual bool Put(const QString& path, const QList<File>& files);

  /*new */virtual bool Announce(const QString& path, const QList<File>& files);
  /*new */virtual bool Describe(const QString& path);
  /*new */virtual bool GetParameter(const QString& path, const QList<File>& files);
  /*new */virtual bool Options(const QString& path);
  /*new */virtual bool Pause(const QString& path);
  /*new */virtual bool Play(const QString& path);
  /*new */virtual bool Record(const QString& path);
  /*new */virtual bool Redirect(const QString& path);
  /*new */virtual bool SetParameter(const QString& path);
  /*new */virtual bool Setup(const QString& path);
  /*new */virtual bool Teardown(const QString& path);

private:
  bool DefaultHandle(const char* cmd);

protected:
  static const char* GetHtmlHeader(bool done);
  static const char* GetRtspHeader();
  const QByteArray& Cookie(const QByteArray& name);
  const QByteArray& Header(const QByteArray& name);

  bool HttpResult(int code, const QByteArray& text, bool done);
  bool HttpRedirect(const QByteArray& uri);
  bool HttpResultOk(bool done);
  bool HttpResultUnauthorized(const char* realm, bool done);
  bool HttpResultBadRequest(bool done);
  bool HttpResultNotFound(bool done);
  bool HttpResultFail(bool done);
  bool HttpInternalError(bool done);
  bool HttpResultDone(const QByteArray& text);
  bool HttpAddContent(const QByteArray& contentType, const QByteArray& contentData);

  void InitRequest(SyncSocket* _Socket) { mSocket = _Socket; }
  bool ReadRequest(int size);
  bool ReadRequest();
  void EndRequest(int size);
  void EndRequest();
  void ClearRequest();

  void ParamsToLower();
  bool GetKeyValue(const QByteArray& key, QByteArray& value);

public:
  QHostAddress HostAddress();

private:
  bool Parse(bool& needMoreData);
  bool ParseHeader(bool& needMoreData);
  bool ParseMultiPart(bool& needMoreData);
  bool ParseOneFile(bool& done);
  bool DispatchCommand();

public:
  HttpHandler();
  /*override */virtual ~HttpHandler();
};

