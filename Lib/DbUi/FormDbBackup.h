#pragma once

#include <QWidget>
#include <QMutex>
#include <QStandardItemModel>

#include <Lib/Db/Db.h>


namespace Ui {
class FormDbBackup;
}

DefineClassS(DbBackupThread);
DefineClassS(QFileDialog);

class FormDbBackup: public QWidget
{
  Ui::FormDbBackup*   ui;

  DbBackupThreadS        mDbBackupThread;

  typedef QMultiMap<QString, QStandardItem*> ModelItemMap;
  QFileDialog*           mFileDialog;
  QStandardItemModel*    mModel;
  ModelItemMap           mModelUseMap;
  QMap<QString, QString> mTableNames;
  QIcon                  mTableIcon;
  QStringList            mTableOrder;

  QMutex                 mPrivate;
  int                    mBackupPercent;
  int                    mBackupIndex;
  QString                mLogText;
  bool                   mCancel;
  bool                   mDone;

  bool                   mManual;

  Q_OBJECT

public:
  explicit FormDbBackup(QWidget* parent = 0);
  ~FormDbBackup();

  friend class DbBackupThread;
  friend class DbBackupTc;

protected:
  virtual void closeEvent(QCloseEvent* event) override;

public:
  void AddTable(const QString& tableInfo, const QIcon& tableIcon, const QStringList& tableRef);

private:
  void Prepare();
  QStandardItem* CloneItemTree(const QString& table);
  void UpdateLog();
  void UpdateSelectedChecked();
  void UpdateAllChecked();
  void CheckTree(QStandardItem* root, bool checked);
  void CheckSelected(bool checked);
  void CheckAll(bool checked);

private: /*internal*/
  bool OnQueryContinue(); /*external thread*/
  void OnPercent(int perc); /*external thread*/
  void OnTable(int index); /*external thread*/
  void OnError(const QString& text); /*external thread*/
  void OnInfo(const QString& text); /*external thread*/
  void Done(); /*external thread*/

private:
  void OnItemChanged(QStandardItem* item);
  void OnSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
  void OnUpdate();

signals:
  void Update();

private slots:
  void on_pushButtonSelectFile_clicked();
  void on_pushButtonStart_clicked();
  void on_pushButtonStop_clicked();
  void on_actionExpand_triggered();
  void on_actionCollapse_triggered();
  void on_checkBoxSelected_toggled(bool checked);
  void on_checkBoxAll_toggled(bool checked);
};
