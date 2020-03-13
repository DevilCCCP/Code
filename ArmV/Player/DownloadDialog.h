#pragma once

#include <QDialog>
#include <QDateTime>

namespace Ui {
class DownloadDialog;
}

class DownloadDialog : public QDialog
{
  Ui::DownloadDialog *ui;

  QList<QPair<int, QString> >& mCameras;
  int                          mSpeed;
  QDateTime                    mStartTime;
  QDateTime                    mEndTime;
  QString                      mPath;
  bool                         mPathAsDefault;

  Q_OBJECT

public:
  void SetCameraId(int id);
  int GetCameraId();
  void SetSpeed(int speed);
  int GetSpeed();
  void SetTimeStart(const QDateTime& timeStart);
  QDateTime GetTimeStart();
  void SetTimeEnd(const QDateTime& timeEnd);
  QDateTime GetTimeEnd();
  void SetPath(const QString& path);
  QString GetPath();
  bool GetPathAsDefault();


public slots:
  virtual void accept();

private slots:
  void on_pushButtonBrowse_clicked();

public:
  explicit DownloadDialog(QList<QPair<int, QString> >& _Cameras, QWidget *parent = 0);
  ~DownloadDialog();
};
