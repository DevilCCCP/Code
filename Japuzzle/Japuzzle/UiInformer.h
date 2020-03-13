#pragma once

#include <QString>

#include "CoreInfo.h"


class MainWindow;

class UiInformer: public CoreInfo
{
  MainWindow* mMainWindow;

public:
  /*override */virtual void Info(const QString& text) Q_DECL_OVERRIDE;
  /*override */virtual void Warning(const QString& text) Q_DECL_OVERRIDE;
  /*override */virtual void Error(const QString& text) Q_DECL_OVERRIDE;

public:
  UiInformer(MainWindow* _MainWindow);
  /*override */virtual ~UiInformer();
};

