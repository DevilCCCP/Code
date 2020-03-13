#pragma once

#include <QWidget>
#include <QIcon>

#include <Lib/Include/Common.h>


namespace Ui {
class FormSection;
}

class FormSection : public QWidget
{
  Ui::FormSection* ui;

  QIcon            mShownIcon;
  QIcon            mHiddenIcon;
  bool             mShow;

  Q_OBJECT

public:
  explicit FormSection(bool _Show = true, QWidget* parent = 0);
  ~FormSection();

public:
  bool IsShow() { return mShow; }
  void SetShow(bool _Show);
  void SetWidget(QWidget* _MainWidget, const QString& _Tytle, QWidget* _CollapseWidget = nullptr);

signals:
  void OnShowChanging(bool show);
  void OnShowChanged(bool show);

private slots:
  void on_toolButtonShow_clicked();
};
