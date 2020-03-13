#pragma once

#include <QWidget>
#include <QStandardItemModel>
#include <QDateTime>

#include "Core.h"


DefineClassS(Core);

namespace Ui {
class FormEvents;
}

class FormEvents: public QWidget
{
  Ui::FormEvents*     ui;

  Core*               mCore;
  bool                mShowEvent;

  QStandardItemModel* mEventModel;
  int                 mHourEvents;
  bool                mHaveObject;

  struct EventLogInfo {
    int       EventId;
    QDateTime Timestamp;
    int       Count;
    qint64    FileId;
  };
  typedef QMap<qint64, EventLogInfo> EventLogInfoMap;
  EventLogInfoMap     mEventLogInfoMap;
  const EventLogInfo* mCurrentEventLog;

  Q_OBJECT

public:
  explicit FormEvents(QWidget *parent = 0);
  ~FormEvents();

public:
  QAction* PreviewAction();

public:
  void Init(Core* _Core, bool _ShowEvent = false);
  void ReloadEvents();

private:
  QueryS RetriveEventFilter(bool forceAll = false);
  bool UpdateEvent(QueryS& q);
  bool ExportEvent(const QString& path, QueryS& q);
  bool ImportEvent(const QString& path);
  bool ImportEventPack(int lineBegin, int lineEnd, const QVariantList& eventIds, const QVariantList& timestamps, const QVariantList& values);
//  bool CancelEvents(const QList<qint64>& ids);
  bool LoadEvent(const QModelIndex& index);
  void ShowEvent();

private:
  void OnSelectEvent(const QModelIndex &current, const QModelIndex &previous);

signals:
  void Info(QString info);
  void Preview(qint64 fileId);
  void Show(int objectId, qint64 timestamp);
  void ShowClose();

private slots:
  void on_pushButtonEventFilterApply_clicked();
  void on_comboBoxEventFilterObject_currentIndexChanged(int index);
  void on_checkBoxEventFilterPeriod_clicked(bool checked);
  void on_pushButtonEventExport_clicked();
  void on_pushButtonEventImport_clicked();
  void on_treeViewEvents_activated(const QModelIndex& index);
  void on_actionShowEvent_triggered();
  void on_actionShowClose_triggered();
};

