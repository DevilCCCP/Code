#pragma once

#include <QString>


const extern QStringList kBytesSuffixes;
const extern QStringList kTimeSuffixes;

void FormatBytes(qreal bytes, qreal& value, int& suffix);
QString FormatBytes(qreal bytes);
QString FormatBytesRu(qreal bytes);

QString FormatTimeDelta(qint64 ms);
QString FormatTime(qint64 ms);

void FormatTime(quint32 ms, qreal& value, int& suffix);
QString FormatTimeDeltaRu(qint64 ms);
QString FormatTimeRu(qint64 ms);
