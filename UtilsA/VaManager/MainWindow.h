#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QProcess>
#include <QStringListModel>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
  Ui::MainWindow*   ui;

  QSettings*        mSettings;
  QProcess*         mBuildProcess;
  QProcess*         mAppProcess;
  QStringList       mListData;
  QStringListModel* mListModel;
  int               mCurrentId;
  bool              mInit;

  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

protected:
  virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;
  virtual void moveEvent(QMoveEvent* event) Q_DECL_OVERRIDE;
  virtual void showEvent(QShowEvent* event) Q_DECL_OVERRIDE;

private:
  void SaveGeometry();
  void SetRunEnable(bool enable);

private:
  void OnBuildFinished(int exitCode, QProcess::ExitStatus exitStatus);
  void OnAppFinished(int exitCode, QProcess::ExitStatus exitStatus);
  void OnListDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles);
  void OnListCurrentChanged(const QModelIndex &current, const QModelIndex &previous);

private slots:
  void on_lineEditProj_textChanged(const QString &text);
  void on_lineEditSourcePath_textChanged(const QString &text);
  void on_lineEditParams_textChanged(const QString &text);
  void on_lineEditMsvsPath_textChanged(const QString &text);
  void on_actionBuild_triggered();
  void on_actionRun_triggered();
  void on_actionStop_triggered();
  void on_actionExit_triggered();
};

#endif // MAINWINDOW_H
