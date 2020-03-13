#pragma once

#include <QDialog>
#include <QVector>


class QStandardItemModel;
class QStandardItem;

namespace Ui {
class DialogList;
}

class DialogList: public QDialog
{
  Ui::DialogList*         ui;

  bool                    mRestarted;
  QStandardItemModel*     mListModel;
  QVector<QStandardItem*> mDirItems;

  Q_OBJECT

public:
  bool IsRestarted() const { return mRestarted; }

public:
  explicit DialogList(QWidget* parent = 0);
  ~DialogList();

private:
  void LoadList();
  void SetAction(int type);
  void SetTypeChecked(int type);

private slots:
  void on_actionTo0_triggered();
  void on_actionTo1_triggered();
  void on_actionTo2_triggered();
  void on_actionTo3_triggered();
  void on_buttonBox_accepted();
  void on_labelAll_linkActivated(const QString&);
  void on_labelNone_linkActivated(const QString&);
  void on_labelState0_linkActivated(const QString&);
  void on_labelState1_linkActivated(const QString&);
  void on_labelState2_linkActivated(const QString&);
  void on_labelState3_linkActivated(const QString&);
  void on_actionRestart_triggered();
};
