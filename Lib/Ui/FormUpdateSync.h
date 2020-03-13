#pragma once

#include <QWidget>

#include <Lib/Include/Common.h>


namespace Ui {
class FormUpdateSync;
}

class FormUpdateSync: public QWidget
{
  Ui::FormUpdateSync* ui;

  QString             mSecsFormat;

  Q_OBJECT

public:
  explicit FormUpdateSync(QWidget* parent = 0);
  ~FormUpdateSync();

signals:
  void SelectUserUpStart(int timeoutMs);

public:
  void OnUpdateSecs(int secs);

private slots:
  void on_commandLinkButtonNow_clicked();
  void on_commandLinkButtonWaitMin_clicked();
  void on_commandLinkButtonWaitMax_clicked();
  void on_commandLinkButtonWaitCustom_clicked();
};
