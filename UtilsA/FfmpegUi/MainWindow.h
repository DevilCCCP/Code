#pragma once

#include <Ui/MainWindow2.h>


class QStandardItemModel;
class QProcess;

namespace Ui {
class MainWindow;
}

class MainWindow: public MainWindow2
{
  Ui::MainWindow*     ui;

  QStandardItemModel* mFilesListModel;
  QString             mFfmpeg;
  QString             mCmd;

  QProcess*           mProcess;
  QStringList         mCmdStack;

  Q_OBJECT

public:
  explicit MainWindow(QWidget* parent = 0);
  ~MainWindow();

protected:
  /*override */virtual void dragEnterEvent(QDragEnterEvent* event) Q_DECL_OVERRIDE;
  /*override */virtual void dragLeaveEvent(QDragLeaveEvent* event) Q_DECL_OVERRIDE;
  /*override */virtual void dragMoveEvent(QDragMoveEvent* event) Q_DECL_OVERRIDE;
  /*override */virtual void dropEvent(QDropEvent* event) Q_DECL_OVERRIDE;

private:
  void AddPath(const QString& path);
  void AddFolder(const QString& path);
  void AddFile(const QString& path);

  void BuildCmd();
  void UpdateGo();

  void StartNextCmd();

private:
  void OnFilesListSelectionChanged();
  void OnProcessReadyRead();
  void OnProcessFinished(int code);

private slots:
  void on_toolButtonFfmpegBrowse_clicked();
  void on_lineEditFfmpeg_textChanged(const QString& text);
  void on_actionAddFiles_triggered();
  void on_actionAddFolder_triggered();
  void on_actionRemoveFiles_triggered();
  void on_radioButtonVideoCopy_toggled(bool checked);
  void on_radioButtonVideoConvert_toggled(bool checked);
  void on_radioButtonVideoNone_toggled(bool checked);
  void on_radioButtonAudioCopy_toggled(bool checked);
  void on_radioButtonAudioConvert_toggled(bool checked);
  void on_radioButtonAudioNone_toggled(bool checked);
  void on_pushButtonGo_clicked();
  void on_lineEditInputCmd_textChanged(const QString&);
  void on_lineEditExtraCmd_textChanged(const QString&);
  void on_lineEditOutputCmd_textChanged(const QString&);
};
