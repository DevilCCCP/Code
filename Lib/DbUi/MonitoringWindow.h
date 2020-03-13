#pragma once

#include <QMainWindow>
#include <QStandardItemModel>
#include <QTimer>
#include <QSet>

#include <Lib/Db/Db.h>
#include <Lib/Ui/MainWindow2.h>


namespace Ui {
class MonitoringWindow;
}

typedef QStandardItem* StateMl;
typedef QPair<QBrush, QBrush> ItemView;

class MonitoringWindow: public MainWindow2
{
  Ui::MonitoringWindow*    ui;
  Db&                      mDb;
  QStringList              mServerTypeNames;
  ObjectTableS             mObjectTable;
  ObjectTypeTableS         mObjectTypeTable;
  ObjectStateS             mObjectState;
  ObjectStateValuesTableS  mObjectStateValuesTable;
  EventTypeTableS          mEventTypeTable;
  EventTableS              mEventTable;
  QSet<int>                mServerTypeIds;
  QString                  mServerTypeIdsText;
  int                      mScheduleTypeId;

  bool                     mLoadSchema;
  bool                     mLoadError;
  QTimer*                  mRefreshTimer;
  QAction*                 mRefreshLast;

  QStandardItemModel*      mStateModel;
  QStandardItemModel*      mLogModel;
  QStandardItemModel*      mEventModel;
  QMap<int, StateMl>       mStateMap;
  ItemView                 mItemEnabled;
  ItemView                 mItemDisabled;
  ItemView                 mItemNotExists;
  int                      mHourEvents;
  bool                     mHaveObject;

  Q_OBJECT

private:
  void ChangeRefreshRate(int rateMs, QAction* refreshNew);

private slots:
  void on_actionExit_triggered();

  void on_actionRefresh_triggered();
  void on_actionRefreshRate0_5_triggered();
  void on_actionRefreshRate1_triggered();
  void on_actionRefreshRate2_triggered();
  void on_actionRefreshRate5_triggered();
  void on_actionRefreshRate30_triggered();
  void on_actionRefreshPause_triggered();
  void on_tabWidgetMain_currentChanged(int index);
  void on_pushButtonLogFilterApply_clicked();
  void on_pushButtonEventFilterApply_clicked();
  void on_comboBoxEventFilterObject_currentIndexChanged(int index);
  void on_checkBoxEventFilterPeriod_clicked(bool checked);
  void on_pushButtonEventExport_clicked();
  void on_pushButtonEventImport_clicked();

public slots:
  void onUpdate();

private:
  bool ReloadSchema();
  bool GetTypeId(const QString& abbr, int& id, QString* name = nullptr);
  void ReloadState();
  QStandardItem* CreateBadStateItem();
  void ReloadEvents();

  bool UpdateState();
  QueryS RetriveLogFilter();
  bool UpdateLog(QueryS& q);
  void AddLogRecord(int objectStateId, int stateValue, const QDateTime& fromTime, const QDateTime& toTime);
  QueryS RetriveEventFilter(bool forceAll = false);
  bool UpdateEvent(QueryS& q);
  bool ExportEvent(const QString& path, QueryS& q);
  bool ImportEvent(const QString& path);
  bool ImportEventPack(int lineBegin, int lineEnd, const QVariantList& eventIds, const QVariantList& timestamps, const QVariantList& values);
//  bool CancelEvents(const QList<qint64>& ids);

public:
  explicit MonitoringWindow(Db& _Db, const QStringList& _ServerTypeNames, QWidget *parent = 0);
  ~MonitoringWindow();
};

