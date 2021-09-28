#include <Lib/Log/Log.h>

#include "HttpHandler.h"


const char kHtmlHeader[] = "HTTP/1.1 200 Ok\r\n""Content-Type: text/html; charset=\"utf-8\"\r\n";
const char kHtmlHeaderDone[] = "HTTP/1.1 200 Ok\r\n""Content-Type: text/html; charset=\"utf-8\"\r\n""\r\n";
const char kRtspHeader[] = "RTSP/1.0 200 OK\r\n";

bool HttpHandler::Receive(SyncSocket* socket, bool& sendDone)
{
  InitRequest(socket);

  bool needMoreData = false;
  if (Parse(needMoreData)) {
    if (mLogRequest) {
      Log.Info(QString("--- Request: ------\n") + QString::fromUtf8(mRequest));
    }
    EndRequest();
    mParseState = eNotStart;
    if (DispatchCommand()) {
      sendDone = !mKeepAlive;
      ClearRequest();
      return true;
    }
  } else if (needMoreData) {
    return true;
  } else {
    EndRequest();
  }
  return false;
}

const char* HttpHandler::Protocol()
{
  return "HTTP";
}

bool HttpHandler::Get(const QString &path, const QList<QByteArray> &params)
{
  Q_UNUSED(path);
  Q_UNUSED(params);
  return DefaultHandle("GET");
}

bool HttpHandler::Post(const QString &path, const QList<QByteArray> &params, const QList<File> &files)
{
  Q_UNUSED(path);
  Q_UNUSED(params);
  Q_UNUSED(files);
  return DefaultHandle("POST");
}

bool HttpHandler::Put(const QString& path, const QList<File>& files)
{
  Q_UNUSED(path);
  Q_UNUSED(files);
  return DefaultHandle("PUT");
}

bool HttpHandler::Announce(const QString& path, const QList<File>& files)
{
  Q_UNUSED(path);
  Q_UNUSED(files);
  return DefaultHandle("ANNOUNCE");
}

bool HttpHandler::Describe(const QString& path)
{
  Q_UNUSED(path);
  return DefaultHandle("DESCRIBE");
}

bool HttpHandler::GetParameter(const QString& path, const QList<File>& files)
{
  Q_UNUSED(path);
  Q_UNUSED(files);
  return DefaultHandle("GET_PARAMETER");
}

bool HttpHandler::Options(const QString &path)
{
  Q_UNUSED(path);
  return DefaultHandle("OPTIONS");
}

bool HttpHandler::Pause(const QString& path)
{
  Q_UNUSED(path);
  return DefaultHandle("PAUSE");
}

bool HttpHandler::Play(const QString& path)
{
  Q_UNUSED(path);
  return DefaultHandle("PLAY");
}

bool HttpHandler::Record(const QString& path)
{
  Q_UNUSED(path);
  return DefaultHandle("RECORD");
}

bool HttpHandler::Redirect(const QString& path)
{
  Q_UNUSED(path);
  return DefaultHandle("REDIRECT");
}

bool HttpHandler::SetParameter(const QString& path)
{
  Q_UNUSED(path);
  return DefaultHandle("SET_PARAMETER");
}

bool HttpHandler::Setup(const QString& path)
{
  Q_UNUSED(path);
  return DefaultHandle("SETUP");
}

bool HttpHandler::Teardown(const QString& path)
{
  Q_UNUSED(path);
  return DefaultHandle("TEARDOWN");
}

bool HttpHandler::DefaultHandle(const char* cmd)
{
  Log.Info(QString("Receive %1 command, it's not implemented").arg(cmd));

  return HttpResult(501, "Not Implemented", true);
}

const char* HttpHandler::GetHtmlHeader(bool done)
{
  return done? kHtmlHeaderDone: kHtmlHeader;
}

const char* HttpHandler::GetRtspHeader()
{
  return kRtspHeader;
}

const QByteArray& HttpHandler::Cookie(const QByteArray& name)
{
  auto itr = mCookies.find(name);
  if (itr != mCookies.end()) {
    return itr.value();
  }
  static QByteArray gEmpty;
  return gEmpty;
}

const QByteArray& HttpHandler::Header(const QByteArray& name)
{
  auto itr = mHeaders.find(name);
  if (itr != mHeaders.end()) {
    return itr.value();
  }
  static QByteArray gEmpty;
  return gEmpty;
}

bool HttpHandler::HttpResult(int code, const QByteArray& text, bool done)
{
  mAnswer = QByteArray("HTTP/1.1 " + QByteArray::number(code) + " " + text + "\r\n");
  if (mKeepAlive) {
    Answer().append("Connection: keep-alive\r\n");
  }
  if (done) {
    Answer().append(QByteArray("Content-Type: text/plain; charset=\"utf-8\"\r\n"));
    Answer().append(QByteArray("Content-Length: ") + QByteArray::number(text.size()) + "\r\n");
    mAnswer.append(QByteArray("\r\n"));
    mAnswer.append(text);
  }
  return true;
}

bool HttpHandler::HttpRedirect(const QByteArray& uri)
{
  HttpResult(301, "Redirect", false);
  Answer().append("Location: " + uri + "\r\n");
  return HttpResultDone("Redirect");
}

bool HttpHandler::HttpResultOk(bool done)
{
  return HttpResult(200, "OK", done);
}

bool HttpHandler::HttpResultUnauthorized(const char* realm, bool done)
{
  HttpResult(401, "Unauthorized", false);
  Answer().append("WWW-Authenticate: Basic realm=\"");
  Answer().append(realm);
  Answer().append("\"\r\n");
  if (done) {
    HttpResultDone("Unauthorized");
  }
  return true;
}

bool HttpHandler::HttpResultBadRequest(bool done)
{
  return HttpResult(400, "Bad Request", done);
}

bool HttpHandler::HttpResultNotFound(bool done)
{
  return HttpResult(404, "Not found", done);
}

bool HttpHandler::HttpResultFail(bool done)
{
  return HttpResult(400, "Bad Request", done);
}

bool HttpHandler::HttpInternalError(bool done)
{
  return HttpResult(500, "Internal Server Error", done);
}

bool HttpHandler::HttpResultDone(const QByteArray& text)
{
  Answer().append(QByteArray("Content-Type: text/plain; charset=utf-8\r\n"));
  Answer().append(QByteArray("Content-Length: ") + QByteArray::number(text.size()) + "\r\n");
  mAnswer.append(QByteArray("\r\n"));
  mAnswer.append(text);
  return true;
}

bool HttpHandler::HttpAddContent(const QByteArray& contentType, const QByteArray& contentData)
{
  Answer().append(QByteArray("Content-Type: ") + contentType
                  + "\r\nContent-Length: " + QByteArray::number(contentData.size()) + "\r\n"
                  + "\r\n");
  Answer().append(contentData);
  return true;
}

bool HttpHandler::ReadRequest(int size)
{
  return mSocket->ReadData(mRequest, size);
}

bool HttpHandler::ReadRequest()
{
  return mSocket->ReadData(mRequest);
}

void HttpHandler::EndRequest(int size)
{
  mRequest.remove(0, size);
}

void HttpHandler::EndRequest()
{
  mRequest.clear();
}

void HttpHandler::ClearRequest()
{
  mCommand.clear();
  mVersion.clear();
  mPath.clear();
  mParams.clear();
  mFiles.clear();
  mHeaders.clear();
  mCookies.clear();
  mKeepAlive = true;
}

void HttpHandler::ParamsToLower()
{
  for (auto itr = mParams.begin(); itr != mParams.end(); itr++) {
    QByteArray& param = *itr;
    param = param.toLower();
  }
}

namespace priv {
enum KeyValueState {
  ePreEqual  = 0,
  ePostEqual = 1,
  eValue     = 2,
  eIllegal   = 3
};
}

bool HttpHandler::GetKeyValue(const QByteArray& key, QByteArray& value)
{
  for (auto itr = mParams.begin(); itr != mParams.end(); itr++) {
    const QByteArray& param = *itr;
    if (param.startsWith(key)) {
      int i = key.size();
      priv::KeyValueState state = priv::ePreEqual;
      for (; i < param.size(); i++) {
        switch (param.at(i)) {
        case '\t': case '\n': case '\v': case '\f': case '\r': case ' ':
          break;
        case '=':
          state = priv::ePostEqual;
          break;
        default:
          if (state == priv::ePostEqual) {
            state = priv::eValue;
          } else {
            state = priv::eIllegal;
          }
          break;
        }
        if (state >= priv::eValue) {
          break;
        }
      }
      if (state == priv::eValue) {
        value = param.mid(i).trimmed();
        return true;
      }
    }
  }
  return false;
}

QHostAddress HttpHandler::HostAddress()
{
  return mSocket->PeerAddress();
}

bool HttpHandler::Parse(bool &needMoreData)
{
  ReadRequest();

  if (mParseState == eNotStart) {
    if (!ParseHeader(needMoreData)) {
      return false;
    }
  }

  if (mParseState == eHeader) {
    if (mMultyPart) {
      if (mMultyPartSize > 0 && mMultyPartBoundary.size() > 0) {
        return ParseMultiPart(needMoreData);
      } else {
        return false;
      }
    } else if (mMultyPartSize > 0) {
      if (mRequest.size() < mMultyPartPos + mMultyPartSize) {
        needMoreData = true;
        return false;
      } else {
        File file;
        file.Data = mRequest.mid(mMultyPartPos);
        mFiles.append(file);
      }
    }
  }
  return true;
}

bool ParseHeaderLineHeaderName(const QByteArray& header, int& pos, QByteArray& headerName)
{
  while (pos < header.size()) {
    char ch = header[pos];
    switch (ch) {
    case ':':
      pos++;
      return true;
    case '\n': case '\r':
      return false;
    default:
      headerName.append(ch);
      break;
    }
    pos++;
  }
  return false;
}

bool ParseHeaderLineHeaderValue(const QByteArray& header, int& pos, QByteArray& headerValue)
{
  while (pos < header.size() && header[pos] == ' ') {
    pos++;
  }
  while (pos < header.size()) {
    char ch = header[pos];
    switch (ch) {
    case '\n': case '\r':
      return true;
    default:
      headerValue.append(ch);
      break;
    }
    pos++;
  }
  return true;
}

bool HttpHandler::ParseHeader(bool& needMoreData)
{
//mRequest = "OPTIONS rtsp://10.232.10.36:8554/59  RTSP/1.0\r\n"
//"CSeq: 1\r\n"
//"User-Agent: IStream_v1.9\r\n";
//
  int spacePos[4] = { 0, 0, 0, 0 };
  int spaceIndex = 0;
  for (int i = 0; i < mRequest.size(); i++) {
    if (mRequest[i] == ' ') {
      spacePos[spaceIndex] = i;
      spaceIndex++;
      i++;
      spacePos[spaceIndex] = i;
      for (; i < mRequest.size(); i++) {
        if (mRequest[i] != ' ') {
          spacePos[spaceIndex] = i;
          break;
        }
      }
      spaceIndex++;
      if (spaceIndex >= 4) {
        break;
      }
    }
  }
  if (spaceIndex < 4) {
    return false;
  }
  int end = mRequest.indexOf('\r', spacePos[3]);
  if (end < 0) {
    needMoreData = true;
    return false;
  }

  needMoreData = false;
  int psize = QString(Protocol()).size();
  if (mRequest.mid(spacePos[3], psize) != Protocol()) {
    Log.Warning(QString("Parse header fail, wrong protocol"));
    return false;
  }
  int posV = spacePos[3] + psize;
  if (mRequest[posV] == '/') {
    posV++;
  }
  mVersion = mRequest.mid(posV, end - posV);
  mCommand = mRequest.mid(0, spacePos[0]);
  QByteArray pathParams = mRequest.mid(spacePos[1], spacePos[2] - spacePos[1]);
  int posD = pathParams.indexOf('?');
  mParams.clear();
  if (posD >= 0) {
    mPath = pathParams.mid(0, posD);
    mParams = pathParams.mid(posD + 1).split('&');
  } else {
    mPath = pathParams;
  }

  mHeaders.clear();
  mCookies.clear();
  mMultyPart = false;
  mMultyPartSize = 0;
  mMultyPartBoundary.clear();
  for (int pos = mRequest.indexOf('\n', spacePos[1] + 5) + 1
       ; pos > 0 && pos < mRequest.size()
       ; pos = mRequest.indexOf('\n', pos) + 1) {
    if (mRequest[pos] == '\r') {
      if (++pos >= mRequest.size()) {
        return false;
      }
    }
    if (mRequest[pos] == '\n') {
      mParseState = eHeader;
      mMultyPartPos = pos + 1;
      mHeaderSize = mMultyPartPos;
      break;
    }

    static QByteArray kConnection("Connection");
    static QByteArray kContentType("Content-Type");
    static QByteArray kContentLen("Content-Length");
    static QByteArray kCoookies("Cookie");

    QByteArray headerName;
    QByteArray headerValue;
    if (!ParseHeaderLineHeaderName(mRequest, pos, headerName)) {
      continue;
    }
    if (!ParseHeaderLineHeaderValue(mRequest, pos, headerValue)) {
      continue;
    }
    mHeaders[headerName] = headerValue;
    if (headerName == kConnection) {
      if (headerValue.toLower() == "keep-alive") {
        SetKeepAlive(true);
      }
    } else if (headerName == kContentType) {
      if (headerValue.startsWith(QByteArray("multipart"))) {
        mMultyPart = true;
        int posa = headerValue.indexOf("boundary=");
        if (posa >= 0) {
          int posd = posa + QByteArray("boundary=").size();
          mMultyPartBoundary = QByteArray("--") + headerValue.mid(posd);
        } else {
          Log.Warning(QString("Content-Type syntax not parsed ('%1')").arg(headerValue.constData()));
        }
      }
    } else if (headerName == kContentLen) {
      bool ok;
      mMultyPartSize = headerValue.toInt(&ok);
      if (!ok) {
        Log.Warning(QString("Content-Length syntax not parsed ('%1')").arg(headerValue.constData()));
      }
    } else if (headerName == kCoookies) {
      QList<QByteArray> cookies = headerValue.split(';');
      for (auto itr = cookies.begin(); itr != cookies.end(); itr++) {
        QList<QByteArray> kv = itr->split('=');
        if (kv.size() == 2) {
          mCookies[kv[0]] = kv[1];
        }
      }
    }
  }

  return true;
}

bool HttpHandler::ParseMultiPart(bool &needMoreData)
{
  bool done;
  while (ParseOneFile(done)) {
    if (done) {
      return true;
    }
  }
  if (done) {
    needMoreData = false;
  } else {
    needMoreData = mRequest.size() < mHeaderSize + mMultyPartSize;
  }
  return false;
}

bool HttpHandler::ParseOneFile(bool &done)
{
  done = false;
  int posa = mRequest.indexOf(mMultyPartBoundary, mMultyPartPos);
  if (posa < 0) {
    return false;
  }
  int posb = posa + mMultyPartBoundary.size();
  if (posb >= mRequest.size()) {
    return false;
  }
  if (mRequest[posb] == '\r') {
    posb++;
  }
  if (posb >= mRequest.size() || mRequest[posb] != '\n') {
    return false;
  }

  File file;
  for (posb++; posb > 0 && posb < mRequest.size(); posb = mRequest.indexOf('\n', posb) + 1) {
    if (mRequest[posb] == '\r') {
      if (++posb >= mRequest.size()) {
        return false;
      }
    }
    if (mRequest[posb] == '\n') {
      break;
    }

    static QString kContentType("Content-Type:");
    static QString kContentDisposition("Content-Disposition:");

    if (mRequest.mid(posb, kContentDisposition.size()) == kContentDisposition) {
      int posc = posb + kContentDisposition.size();
      int posd = mRequest.indexOf('\n', posb);
      QByteArray line = mRequest.mid(posc, posd - posc).trimmed();
      QList<QByteArray> params = line.split(';');
      for (int i = 0; i < params.size(); i++) {
        QList<QByteArray> keyVal = params[i].split('=');
        if (keyVal.size() >= 2) {
          QByteArray key = keyVal[0].trimmed();
          QByteArray val = keyVal[1].trimmed();
          if (key == "name") {
            file.Type = QString(val);
          } else if (key == "filename") {
            file.Name = QString(val);
          }
        }
      }
    } else if (mRequest.mid(posb, kContentType.size()) == kContentType) {
      int posc = posb + kContentType.size();
      int posd = mRequest.indexOf('\n', posb);
      file.Type = mRequest.mid(posc, posd - posc).trimmed();
    } else {
      Log.Warning(QString("Body syntax don't known: \n") + mRequest.mid(posb, 200));
      continue;
    }
  }
  posb++;

  int pose = mRequest.indexOf(mMultyPartBoundary, posb);
  if (pose < 0) {
    return false;
  }
  file.Data = mRequest.mid(posb, pose - posb - 2);
  int posx = pose + mMultyPartBoundary.size();
  if (posx + 2 > mRequest.size()) {
    return false;
  }
  mFiles.append(file);
  if (mRequest[posx] == '-' && mRequest[posx + 1] == '-') {
    done = true;
  } else {
    mMultyPartPos = pose;
  }
  return true;
}

bool HttpHandler::DispatchCommand()
{
  bool handled = false;
  mAnswer.clear();
  mResultFile.clear();
  if (mCommand.size() > 2) switch (mCommand[0]) {
  case 'A':
    if (     mCommand == "ANNOUNCE") { handled = Announce(mPath, mFiles); }
    break;
  case 'D':
    if (     mCommand == "DESCRIBE") { handled = Describe(mPath); }
    break;
  case 'G':
    if (     mCommand == "GET"          ) { handled = Get(mPath, mParams); }
    else if (mCommand == "GET_PARAMETER") { handled = GetParameter(mPath, mFiles); }
    break;
  case 'O':
    if (     mCommand == "OPTIONS") { handled = Options(mPath); }
    break;
  case 'P':
    switch (mCommand[1]) {
    case 'A':
      if (   mCommand == "PAUSE") { handled = Pause(mPath); }
      break;
    case 'L':
      if (   mCommand == "PLAY" ) { handled = Play(mPath); }
      break;
    case 'O':
      if (   mCommand == "POST" ) { handled = Post(mPath, mParams, mFiles); }
      break;
    case 'U':
      if (   mCommand == "PUT" ) { handled = Put(mPath, mFiles); }
      break;
    }
    break;
  case 'R':
    if (     mCommand == "RECORD"  ) { handled = Record(mPath); }
    else if (mCommand == "REDIRECT") { handled = Redirect(mPath); }
    break;
  case 'S':
    if (     mCommand == "SETUP") { handled = Setup(mPath); }
    else if (mCommand == "SET_PARAMETER") { handled = SetParameter(mPath); }
    break;
  case 'T':
    if (     mCommand == "TEARDOWN") { handled = Teardown(mPath); }
    break;
  }

  if (handled) {
    if (mLogRespond) {
      int index = mAnswer.indexOf("\r\n\r\n");
      if (index > 0) {
        Log.Info(QString("=== Respond: ======\n") + QString::fromUtf8(mAnswer.left(index + 2)));
      } else {
        Log.Info(QString("=== Respond: ======\n") + QString::fromUtf8(mAnswer));
      }
    }
    if (!mAnswer.isEmpty()) {
      mSocket->WriteData(mAnswer);
      if (mResultFile) {
        SendFile(mResultFile, mFileSpeedLimit);
      }
    }
  } else {
    Log.Warning(QString("Command '%1' not handled").arg(mCommand.constData()));
  }
  return handled;
}


HttpHandler::HttpHandler()
#ifdef QT_NO_DEBUG
  : mLogRequest(false), mLogRespond(false)
#else
  : mLogRequest(true), mLogRespond(true)
#endif
  , mKeepAlive(true)
  , mFileSpeedLimit(100 * 1024*1024)
  , mParseState(eNotStart)
{
#ifndef QT_NO_DEBUG
  setDebug(true);
#endif
}

HttpHandler::~HttpHandler()
{
}


