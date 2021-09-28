#include <QProcess>
#include <QFileDialog>
#include <QSettings>
#include <QStandardItemModel>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QDirIterator>
#include <QTemporaryFile>

#include <Ui/Icon.h>

#include "MainWindow.h"
#include "ui_MainWindow.h"


MainWindow::MainWindow(QWidget* parent)
  : MainWindow2(parent), ui(new Ui::MainWindow)
  , mFilesListModel(new QStandardItemModel(this))
  , mProcess(nullptr)
{
  ui->setupUi(this);

  setAcceptDrops(true);
  ui->lineEditFfmpeg->setText(GetSettings()->value("Ffmpeg").toString());

  ui->actionAddFiles->setIcon(IconFromImage(":/Icons/File.png", ":/Icons/Add.png"));
  ui->actionAddFolder->setIcon(IconFromImage(":/Icons/Folder.png", ":/Icons/Add.png"));
  ui->actionRemoveFiles->setIcon(IconFromImage(":/Icons/File.png", ":/Icons/Remove.png"));

  ui->toolButtonAddFile->setDefaultAction(ui->actionAddFiles);
  ui->toolButtonAddFolder->setDefaultAction(ui->actionAddFolder);
  ui->toolButtonRemove->setDefaultAction(ui->actionRemoveFiles);

  ui->listViewFiles->addActions(QList<QAction*>() << ui->actionAddFiles << ui->actionAddFolder << ui->actionRemoveFiles);
  ui->listViewFiles->setContextMenuPolicy(Qt::ActionsContextMenu);
  ui->listViewFiles->setModel(mFilesListModel);
  if (auto sm = ui->listViewFiles->selectionModel()) {
    connect(sm, &QItemSelectionModel::selectionChanged, this, &MainWindow::OnFilesListSelectionChanged);
  }
  OnFilesListSelectionChanged();

  ui->radioButtonVideoCopy->setChecked(true);
  ui->radioButtonAudioNone->setChecked(true);
  ui->widgetCmdLog->setVisible(false);
}

MainWindow::~MainWindow()
{
  if (mProcess) {
    mProcess->disconnect();
  }

  delete ui;
}


void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
  event->setDropAction(Qt::CopyAction);
  event->accept();
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent* event)
{
  event->accept();
}

void MainWindow::dragMoveEvent(QDragMoveEvent* event)
{
  event->setDropAction(Qt::CopyAction);
  event->accept();
}

void MainWindow::dropEvent(QDropEvent* event)
{
  const QMimeData* mimeData = event->mimeData();
  if (mimeData->hasUrls()) {
    foreach (const QUrl& url, mimeData->urls()) {
      AddPath(url.toLocalFile());
    }
  } else if (mimeData->hasText()) {
    AddPath(mimeData->text());
  }

  UpdateGo();
}

void MainWindow::AddPath(const QString& path)
{
  QFileInfo info(path);
  if (info.isDir()) {
    AddFolder(path);
  } else if (info.isFile()) {
    AddFile(path);
  }
}

void MainWindow::AddFolder(const QString& path)
{
  QDirIterator itr(path, QDir::NoDotAndDotDot | QDir::Files, QDirIterator::Subdirectories);
  while (itr.hasNext()) {
    AddFile(itr.next());
  }
}

void MainWindow::AddFile(const QString& path)
{
  QStandardItem* item = new QStandardItem(path);
  mFilesListModel->appendRow(item);
}

void MainWindow::BuildCmd()
{
  QStringList fullCmd;
  fullCmd << QString("-i %INPUT%");
  fullCmd << ui->lineEditInputCmd->text();
  fullCmd << ui->lineEditVideoCmd->text();
  fullCmd << ui->lineEditAudioCmd->text();
  fullCmd << ui->lineEditExtraCmd->text();
  fullCmd << QString("%OUTPUT%");
  fullCmd << ui->lineEditOutputCmd->text();
  fullCmd.removeAll(QString(""));

  mFfmpeg = ui->lineEditFfmpeg->text();
  mCmd = fullCmd.join(" ");
  ui->lineEditFinalCmd->setText(mCmd);

  UpdateGo();
}

void MainWindow::UpdateGo()
{
  mCmd = ui->lineEditFinalCmd->text();
  if (mFfmpeg.isEmpty()) {
    ui->labelInfo->setText("<html><head/><body><p><span style=\" color:red;\">* Setup ffmpeg binaries path</span></p></body></html>");
  } else if (mCmd.isEmpty()) {
    ui->labelInfo->setText("* Input all command line parameters");
  } else {
    ui->labelInfo->setText("* You can use drag and drop to add files");
  }
  ui->pushButtonGo->setEnabled(!mFfmpeg.isEmpty() && !mCmd.isEmpty() && mFilesListModel->rowCount() > 0);
}

void MainWindow::StartNextCmd()
{
  if (mCmdStack.isEmpty()) {
    return;
  }

  mProcess = new QProcess(this);
  mProcess->setProcessChannelMode(QProcess::MergedChannels);
  QString nextCmd = mCmdStack.takeFirst();
  ui->progressBarCommands->setValue(ui->progressBarCommands->maximum() - mCmdStack.size());
  ui->textEditCmdOutput->setText("");

  connect(mProcess, &QProcess::readyRead, this, &MainWindow::OnProcessReadyRead);
  connect(mProcess, static_cast<void (QProcess::*)(int)>(&QProcess::finished), this, &MainWindow::OnProcessFinished);
  mProcess->start(nextCmd);
  mProcess->write("y\n");

  ui->widgetCmdLog->setVisible(true);
}

void MainWindow::OnFilesListSelectionChanged()
{
  bool hasSelection = ui->listViewFiles->selectionModel()->hasSelection();
  ui->actionRemoveFiles->setEnabled(hasSelection);
}

void MainWindow::OnProcessReadyRead()
{
  QString newText = QString::fromLocal8Bit(mProcess->readAll());
  ui->textEditCmdOutput->setText(ui->textEditCmdOutput->toPlainText() + newText);

  QTextCursor cursor = ui->textEditCmdOutput->textCursor();
  cursor.movePosition(QTextCursor::End);
  ui->textEditCmdOutput->setTextCursor(cursor);
}

void MainWindow::OnProcessFinished(int code)
{
  Q_UNUSED(code);

  QString newText = QString::fromLocal8Bit(mProcess->readAll());
  ui->textEditCmdOutput->setText(ui->textEditCmdOutput->toPlainText() + newText);
  mProcess->deleteLater();
  mProcess = nullptr;
  ui->widgetCmdLog->setVisible(false);

  StartNextCmd();
}

void MainWindow::on_toolButtonFfmpegBrowse_clicked()
{
  QString path = QFileDialog::getOpenFileName(this);
  if (!path.isEmpty()) {
    ui->lineEditFfmpeg->setText(path);
  }
}

void MainWindow::on_lineEditFfmpeg_textChanged(const QString& text)
{
  GetSettings()->setValue("Ffmpeg", text);
}

void MainWindow::on_actionAddFiles_triggered()
{
  QStringList pathList = QFileDialog::getOpenFileNames(this);
  foreach (const QString& path, pathList) {
    AddPath(path);
  }

  UpdateGo();
}

void MainWindow::on_actionAddFolder_triggered()
{
  QString path = QFileDialog::getExistingDirectory(this);
  AddPath(path);

  UpdateGo();
}

void MainWindow::on_actionRemoveFiles_triggered()
{
  QModelIndexList indexList = ui->listViewFiles->selectionModel()->selectedRows();
  QVector<int> rows;
  rows.reserve(indexList.size());
  foreach (const QModelIndex& index, indexList) {
    rows.append(index.row());
  }
  qSort(rows);
  for (int i = rows.size() - 1; i >= 0; i--) {
    mFilesListModel->removeRow(rows.at(i));
  }
}

void MainWindow::on_radioButtonVideoCopy_toggled(bool checked)
{
  if (checked) {
    ui->lineEditVideoCmd->setText("-vcodec copy");
    BuildCmd();
  }
}

void MainWindow::on_radioButtonVideoConvert_toggled(bool checked)
{
  if (checked) {
    ui->lineEditVideoCmd->setText("");
    BuildCmd();
  }
}

void MainWindow::on_radioButtonVideoNone_toggled(bool checked)
{
  if (checked) {
    ui->lineEditVideoCmd->setText("-vn");
    BuildCmd();
  }
}

void MainWindow::on_radioButtonAudioCopy_toggled(bool checked)
{
  if (checked) {
    ui->lineEditAudioCmd->setText("-acodec copy");
    BuildCmd();
  }
}

void MainWindow::on_radioButtonAudioConvert_toggled(bool checked)
{
  if (checked) {
    ui->lineEditAudioCmd->setText("");
    BuildCmd();
  }
}

void MainWindow::on_radioButtonAudioNone_toggled(bool checked)
{
  if (checked) {
    ui->lineEditAudioCmd->setText("-an");
    BuildCmd();
  }
}

void MainWindow::on_pushButtonGo_clicked()
{
  QTemporaryFile* cmdBatch = new QTemporaryFile(this);
  if (!cmdBatch->open()) {

  }
  for (int i = 0; i < mFilesListModel->rowCount(); i++) {
    QString inputPath = mFilesListModel->data(mFilesListModel->index(i, 0)).toString();
    QFileInfo info(inputPath);
    QString name = info.completeBaseName();
    QString ext = ui->lineEditOut->text();
    QString outputFile;
    if (ext.isEmpty() && info.suffix().isEmpty()) {
      outputFile = name + ".avi";
    } else if (ext.isEmpty() || info.suffix() == ext) {
      ext = info.suffix();
      QString tmp = name + "_." + ext;
      QDir dir = info.dir();
      for (int k = 1; dir.exists(tmp); k++) {
        tmp = name + QString("_%1.").arg(k, 3, 10, QChar('0')) + ext;
      }
      outputFile = tmp;
    } else {
      outputFile = name + "." + ext;
    }
    QString outputPath = info.dir().absoluteFilePath(outputFile);
    QString cmd = mCmd;
    cmd.replace("%INPUT%", QString("\"%1\"").arg(inputPath));
    cmd.replace("%OUTPUT%", QString("\"%1\"").arg(outputPath));

    QString fullCmd = QString("\"%1\" %2").arg(mFfmpeg, cmd);
    mCmdStack.append(fullCmd);
  }
  ui->progressBarCommands->setMaximum(mCmdStack.size());
  if (!mProcess) {
    StartNextCmd();
  }
}

void MainWindow::on_lineEditInputCmd_textChanged(const QString&)
{
  BuildCmd();
}

void MainWindow::on_lineEditExtraCmd_textChanged(const QString&)
{
  BuildCmd();
}

void MainWindow::on_lineEditOutputCmd_textChanged(const QString&)
{
  BuildCmd();
}
