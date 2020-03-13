#pragma once

#include <QIcon>

#include "ToolAction.h"


class ActionIcons
{
  QIcon              mRefreshIcon;
  QIcon              mRebootIcon;
  ToolActionIconMap  mIconsMap;

public:
  void Clear();
  void AddTypeIcon(int typeId, const QString& typeName);

  const QIcon GetIcon(const ToolAction& action) const;

public:
  ActionIcons();
};
