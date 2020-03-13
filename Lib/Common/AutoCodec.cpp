#include <QSet>
#include <QMap>
#include <QFile>
#include <QStringList>
#include <QDataStream>
#include <QTextCodec>

#include "AutoCodec.h"
#include "HystText.h"


const QStringList kDefaultOneByteCodecs = QStringList() << "IBM 866" << "KOI8-R" << "Windows-1251";
const QStringList kDefaultMultiByteCodecs = QStringList() << "UTF-8" << "UTF-16BE" << "UTF-16LE";
const QStringList kDefaultCodecs = QStringList() << kDefaultOneByteCodecs << kDefaultMultiByteCodecs;
const QStringList kDefaultCharSets =  QStringList() << "EN" << "DE" << "RU";
const QStringList kDefaultLangs =  QStringList() << "EN" << "DE" << "RU";
const QString kDefaultNeytralCharSet =  "Ascii";

bool AutoCodec::Create(const QString& dictsPath)
{
  QString asciiList = LoadCharset(QString("%1/%2 chars.txt").arg(dictsPath).arg(mNeytralCharSet));
  if (asciiList.isEmpty()) {
    return false;
  }

  QSet<QChar> allCharSet;
  mNeytralCharsMap.resize(0x100);
  mNeytralCharsMap.fill(0);
  foreach (const QChar& ch, asciiList) {
    mNeytralCharsMap[(int)(uchar)ch.toLatin1()] = 1;
    allCharSet.insert(ch);
  }

  for (int j = 0; j < mCharSetList.size(); j++) {
    QString charsList = LoadCharset(QString("%1/%2 chars.txt").arg(dictsPath).arg(mCharSetList.at(j)));
    if (charsList.isEmpty()) {
      return false;
    }
    foreach (const QChar& ch, charsList) {
      allCharSet.insert(ch);
    }
  }
  QString allCharPairs;
  for (auto itr = allCharSet.constBegin(); itr != allCharSet.constEnd(); itr++) {
    const QChar& ch1 = *itr;
    for (auto itr = allCharSet.constBegin(); itr != allCharSet.constEnd(); itr++) {
      const QChar& ch2 = *itr;
      allCharPairs.append(ch1);
      allCharPairs.append(ch2);
    }
  }

  mCodecInfoList.resize(mCodecList.size());
  for (int i = 0; i < mCodecInfoList.size(); i++) {
    CodecInfo* codecInfo = &mCodecInfoList[i];
    codecInfo->CodecName = mCodecList.at(i);
    codecInfo->LangInfoList.resize(mLangList.size());

    codecInfo->Codec       = QTextCodec::codecForName(codecInfo->CodecName.toLatin1());
    codecInfo->Encoder     = codecInfo->Codec? codecInfo->Codec->makeEncoder(QTextCodec::IgnoreHeader): nullptr;
    codecInfo->Decoder     = codecInfo->Codec? codecInfo->Codec->makeDecoder(QTextCodec::IgnoreHeader): nullptr;
    codecInfo->RangesHyst.reset(new HystText(2));
    for (int j = 0; j < codecInfo->LangInfoList.size(); j++) {
      LangInfo* langInfo = &codecInfo->LangInfoList[j];
      langInfo->LangName = mLangList.at(j);
      langInfo->LangHyst.reset(new HystText(2));
    }

    QByteArray allCharPairsCoded = codecInfo->Encoder->fromUnicode(allCharPairs + " " + allCharPairs);
    codecInfo->RangesHyst->AddTwoByte(allCharPairsCoded);
  }

  for (int j = 0; j < mLangList.size(); j++) {
    QFile dictFile(QString("%1/%2 words.txt").arg(dictsPath).arg(mLangList.at(j)));
    if (!dictFile.open(QFile::ReadOnly)) {
      printf("Loading %s fail\n", dictFile.fileName().toUtf8().constData());
      return false;
    }
    printf("Loading %s ...\n", dictFile.fileName().toUtf8().constData());
    while (!dictFile.atEnd()) {
      QByteArray lineData = dictFile.readLine();
      QString line = QString::fromUtf8(lineData);
      if (!line.isEmpty()) {
        int space = line.indexOf(' ');
        if (space > 0) {
          QString word = line.mid(0, space);
          int count = line.mid(space + 1).toInt();
          if (count) {
            AddLangWord(j, word, count);
            if (count >= 10) {
              QString wordCap = word.mid(0, 1).toUpper() + word.mid(1);
              AddLangWord(j, wordCap, count/10);
              if (count >= 100) {
                QString wordBig = word.toUpper();
                AddLangWord(j, wordBig, count/100);
              }
            }
          }
        }
      }
    }
  }
  return true;
}

bool AutoCodec::Save(QByteArray& data) const
{
  QDataStream stream(&data, QIODevice::WriteOnly);
  stream << mNeytralCharsMap.size();
  for (int i = 0; i < mNeytralCharsMap.size(); i++) {
    if (mNeytralCharsMap.at(i)) {
      stream << i;
    }
  }
  stream << -1;

  stream << mCodecInfoList.size();
  for (int i = 0; i < mCodecInfoList.size(); i++) {
    const CodecInfo& codecInfo = mCodecInfoList.at(i);
    stream << codecInfo.CodecName;
    if (!codecInfo.RangesHyst->Save(stream)) {
      return false;
    }
    stream << codecInfo.LangInfoList.size();
    for (int j = 0; j < codecInfo.LangInfoList.size(); j++) {
      const LangInfo& langInfo = codecInfo.LangInfoList.at(j);
      stream << langInfo.LangName;
      if (!langInfo.LangHyst->Save(stream)) {
        return false;
      }
    }
  }
  return true;
}

bool AutoCodec::Load(const QByteArray& data)
{
  QDataStream stream(data);
  int size;
  stream >> size;
  mNeytralCharsMap.fill(0, size);
  forever {
    int i;
    stream >> i;
    if (i < 0) {
      break;
    }
    mNeytralCharsMap[i] = 1;
  }

  int sz;
  stream >> sz;
  mCodecInfoList.resize(sz);
  for (int i = 0; i < mCodecInfoList.size(); i++) {
    CodecInfo* codecInfo = &mCodecInfoList[i];
    stream >> codecInfo->CodecName;
    codecInfo->RangesHyst.reset(new HystText());
    if (!codecInfo->RangesHyst->Load(stream)) {
      return false;
    }
    stream >> sz;
    codecInfo->LangInfoList.resize(sz);
    for (int j = 0; j < codecInfo->LangInfoList.size(); j++) {
      LangInfo* langInfo = &codecInfo->LangInfoList[j];
      stream >> langInfo->LangName;
      langInfo->LangHyst.reset(new HystText());
      if (!langInfo->LangHyst->Load(stream)) {
        return false;
      }
    }

    codecInfo->Codec     = QTextCodec::codecForName(codecInfo->CodecName.toLatin1());
    codecInfo->Encoder   = codecInfo->Codec? codecInfo->Codec->makeEncoder(QTextCodec::IgnoreHeader): nullptr;
    codecInfo->Decoder   = codecInfo->Codec? codecInfo->Codec->makeDecoder(QTextCodec::IgnoreHeader): nullptr;
  }
  return true;
}

bool AutoCodec::DefineCodecLang(const QByteArray& text)
{
  QMap<qreal, int> codecFitMap;
  for (int i = 0; i < mCodecInfoList.size(); i++) {
    const CodecInfo& codecInfo = mCodecInfoList[i];
    qreal fit = codecInfo.RangesHyst->CalcTwoByteFitRange(text);
    codecFitMap.insertMulti(fit, i);
  }
  qreal codecPrecisionThreshold = codecFitMap.lastKey() - 0.01;
  if (codecPrecisionThreshold < 0.1) {
    return false;
  }

  HystText hyst(2);
  hyst.AddTwoByte(text);
  mPrecision = -1;
  for (auto itr = codecFitMap.lowerBound(codecPrecisionThreshold); itr != codecFitMap.end(); itr++) {
    int i = itr.value();
    const CodecInfo& codecInfo = mCodecInfoList[i];
    for (int j = 0; j < codecInfo.LangInfoList.size(); j++) {
      const LangInfo& langInfo = codecInfo.LangInfoList.at(j);
      qreal fit = langInfo.LangHyst->CalcFitHyst(hyst);
      if (fit > mPrecision) {
        mPrecision = fit;
        mCurrentCodec = const_cast<CodecInfo*>(&codecInfo);
        mCurrentLang  = const_cast<LangInfo*>(&langInfo);
      }
    }
  }
  return mPrecision > 0;
}

bool AutoCodec::DefineCodec(const QByteArray& text, const QString& lang)
{
  mHintCodec = QString();
  mHintLang  = lang;
  return DefineCurrent(text);
}

bool AutoCodec::DefineLang(const QByteArray& text, const QString& codec)
{
  mHintCodec = codec;
  mHintLang  = QString();
  return DefineCurrent(text);
}

bool AutoCodec::DefineCodec(const QByteArray& text)
{
  mCurrentCodec = nullptr;
  mPrecision = -1;
  for (int i = 0; i < mCodecInfoList.size(); i++) {
    const CodecInfo& codecInfo = mCodecInfoList[i];
    qreal fit = codecInfo.RangesHyst->CalcTwoByteFitRange(text);
    if (fit > mPrecision) {
      mCurrentCodec = const_cast<CodecInfo*>(&codecInfo);
      mPrecision = fit;
    }
  }
  return mCurrentCodec != nullptr;
}

bool AutoCodec::CheckCodec(const QByteArray& text, const QString& codecName)
{
  mCurrentCodec = nullptr;
  mPrecision = -1;
  for (int i = 0; i < mCodecInfoList.size(); i++) {
    const CodecInfo& codecInfo = mCodecInfoList[i];
    if (codecInfo.CodecName == codecName) {
      mCurrentCodec = const_cast<CodecInfo*>(&codecInfo);
      mPrecision = codecInfo.RangesHyst->CalcTwoByteFitRange(text);
    }
  }
  if (mPrecision > 0.99) {
    return true;
  }

  if (!mCurrentCodec) {
    return false;
  }

  for (int i = 0; i < mCodecInfoList.size(); i++) {
    const CodecInfo& codecInfo = mCodecInfoList[i];
    if (codecInfo.CodecName != codecName) {
      qreal fit = codecInfo.RangesHyst->CalcTwoByteFitRange(text);
      if (fit > mPrecision + 0.01) {
        return false;
      }
    }
  }
  return mCurrentCodec != nullptr;
}

QString AutoCodec::Dump() const
{
  QString info;
  for (int i = 0; i < mCodecInfoList.size(); i++) {
    const CodecInfo& codecInfo = mCodecInfoList[i];
    info.append(QString("Codec: %1\n").arg(codecInfo.CodecName));
    for (int j = 0; j < codecInfo.LangInfoList.size(); j++) {
      const LangInfo& langInfo = codecInfo.LangInfoList.at(j);
      info.append(QString("Lang: %1\n%2\n")
                  .arg(langInfo.LangName, langInfo.LangHyst->Dump()));
    }
  }
  return info;
}

QString AutoCodec::LoadCharset(const QString& charSetName)
{
  QFile charsFile(charSetName);
  if (!charsFile.open(QFile::ReadOnly)) {
    return QString();
  }
  QByteArray charsDataList = charsFile.readAll();
  return QString::fromUtf8(charsDataList);
}

void AutoCodec::AddLangWord(int langIndex, const QString& word, int count)
{
  for (int i = 0; i < mCodecInfoList.size(); i++) {
    CodecInfo* codecInfo = &mCodecInfoList[i];
    if (codecInfo->Encoder) {
      QByteArray data = codecInfo->Encoder->fromUnicode(word);
      LangInfo* langInfo = &codecInfo->LangInfoList[langIndex];
      langInfo->LangHyst->AddTwoByteCount(data, count);
    }
  }
}

bool AutoCodec::DefineCurrent(const QByteArray& text)
{
  HystText hyst;
  hyst.AddOneByte(text);
  mPrecision = -1;
  for (int i = 0; i < mCodecInfoList.size(); i++) {
    const CodecInfo& codecInfo = mCodecInfoList[i];
    if (!mHintCodec.isEmpty() && mHintCodec != codecInfo.CodecName) {
      continue;
    }
    for (int j = 0; j < codecInfo.LangInfoList.size(); j++) {
      const LangInfo& langInfo = codecInfo.LangInfoList.at(j);
      if (!mHintLang.isEmpty() && mHintLang != langInfo.LangName) {
        continue;
      }
      qreal fit = hyst.CalcFitHyst(*langInfo.LangHyst);
      if (fit > mPrecision) {
        mPrecision = fit;
        mCurrentCodec = const_cast<CodecInfo*>(&codecInfo);
        mCurrentLang  = const_cast<LangInfo*>(&langInfo);
      }
    }
  }
  return mPrecision > 0;
}


AutoCodec::AutoCodec()
  : mCurrentCodec(nullptr), mCurrentLang(nullptr), mPrecision(-1)
{
  mCodecList   = kDefaultCodecs;
  mCharSetList = kDefaultCharSets;
  mLangList    = kDefaultLangs;
  mNeytralCharSet = kDefaultNeytralCharSet;
}

