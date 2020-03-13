#pragma once

#include <QWidget>
#include <QStandardItemModel>

#include "Core.h"


DefineClassS(Core);

namespace Ui {
class FormSysLog;
}

class FormSysLog: public QWidget
{
  Ui::FormSysLog*     ui;

  Core*               mCore;

  QStandardItemModel* mLogModel;

  Q_OBJECT

public:
  void Init(Core* _Core);
  void ReloadObjects();

private:
  QueryS RetriveLogFilter();
  bool UpdateLog(QueryS& q);
  void AddLogRecord(int objectStateId, int stateValue, const QDateTime& fromTime, const QDateTime& toTime);

private slots:
  void on_pushButtonLogFilterApply_clicked();

signals:
  void Info(QString info);

public:
  explicit FormSysLog(QWidget *parent = 0);
  ~FormSysLog();
};

