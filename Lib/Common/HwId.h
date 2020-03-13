#pragma once

#include <QString>


class HwId
{
public:
  static QString Get();

public:
  HwId() = delete;
  ~HwId() = default;
};
