#pragma once
#include <QtGlobal>

inline void UpdateStatValuesTop(int& statValue, int& statDisp, const int& value)
{
  int inc = statValue / 10 + 10;
  int d = qAbs(value - statValue);
  if (d <= inc) {
    statValue = value;
  } else if (value > statValue) {
    statValue += inc;
  } else {
    statValue -= inc;
  }

  if (d < statDisp / 4) {
    statDisp = (d + 31 * statDisp) / 32;
  } else if (d > statDisp) {
    statDisp = (3 * d + statDisp) / 4;
  }
}

