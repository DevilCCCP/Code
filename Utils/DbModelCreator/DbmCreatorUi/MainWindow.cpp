#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QFileDialog>
#include <QDirIterator>

#include "MainWindow.h"
#include "ui_MainWindow.h"


MainWindow::MainWindow(QWidget* parent)
  : MainWindow2(parent), ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  ui->toolButtonGenerate->setDefaultAction(ui->actionGenerate);
  ui->toolButtonSave->setDefaultAction(ui->actionSave);
  ui->toolButtonSaveModel->setDefaultAction(ui->actionSaveModel);
  ui->toolButtonPath->setDefaultAction(ui->actionPath);
  ui->toolButtonPathModel->setDefaultAction(ui->actionPathModel);
  ui->toolButtonBatch->setDefaultAction(ui->actionBatch);
  ui->toolButtonBatchModel->setDefaultAction(ui->actionBatchModel);

  Restore();
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::on_actionGenerate_triggered()
{
  QString sqlScript = ui->plainTextEditDbScript->toPlainText();
  QString prefix = ui->lineEditPrefix->text();
  if (sqlScript.isEmpty()) {
    sqlScript = QApplication::clipboard()->text();
    if (sqlScript.isEmpty()) {
      QMessageBox::warning(this, this->windowTitle(), QString::fromUtf8("Provide source SQL script"));
      return;
    } else {
      ui->plainTextEditDbScript->setPlainText(sqlScript);
    }
  }

  if (mCore.Generate(sqlScript, prefix)) {
    ui->plainTextEditClasses->setPlainText(mCore.GetClasses());
    ui->plainTextEditHeader->setPlainText(mCore.GetHeader());
    ui->plainTextEditSource->setPlainText(mCore.GetSource());
    ui->plainTextEditModelHeader->setPlainText(mCore.GetModelHeader());
    ui->plainTextEditModelSource->setPlainText(mCore.GetModelSource());
    ui->statusBar->showMessage("Generate done");
  } else {
    ui->plainTextEditClasses->setPlainText("");
    ui->plainTextEditHeader->setPlainText("");
    ui->plainTextEditSource->setPlainText("");
    ui->plainTextEditModelHeader->setPlainText("");
    ui->plainTextEditModelSource->setPlainText("");
    ui->statusBar->showMessage("Generate fail");
  }
}

void MainWindow::on_toolButtonSourcePaste_clicked()
{
  ui->plainTextEditDbScript->setPlainText(QApplication::clipboard()->text());
  ui->actionGenerate->trigger();
}

void MainWindow::on_toolButtonHeaderCopy_clicked()
{
  QApplication::clipboard()->setText(ui->plainTextEditHeader->toPlainText());
}

void MainWindow::on_toolButtonSourceCopy_clicked()
{
  QApplication::clipboard()->setText(ui->plainTextEditSource->toPlainText());
}

void MainWindow::on_toolButtonModelHeaderCopy_clicked()
{
  QApplication::clipboard()->setText(ui->plainTextEditModelHeader->toPlainText());
}

void MainWindow::on_toolButtonModelSourceCopy_clicked()
{
  QApplication::clipboard()->setText(ui->plainTextEditModelSource->toPlainText());
}

void MainWindow::on_actionSave_triggered()
{
  if (mSourceDirectory.isEmpty()) {
    on_actionPath_triggered();
  }

  QDir destDir(mSourceDirectory);
  QFile cppFile(destDir.absoluteFilePath(QString("%1.cpp").arg(mCore.GetClassName())));
  if (cppFile.open(QFile::WriteOnly)) {
    cppFile.write(mCore.GetSource().toUtf8());
  }
  QFile headerFile(destDir.absoluteFilePath(QString("%1.h").arg(mCore.GetClassName())));
  if (headerFile.open(QFile::WriteOnly)) {
    headerFile.write(mCore.GetHeader().toUtf8());
  }
}

void MainWindow::on_actionSaveModel_triggered()
{
  if (mSourceModelDirectory.isEmpty()) {
    on_actionPathModel_triggered();
  }

  QDir destDir(mSourceModelDirectory);
  QFile cppFile(destDir.absoluteFilePath(QString("%1Model.cpp").arg(mCore.GetClassName())));
  if (cppFile.open(QFile::WriteOnly)) {
    cppFile.write(mCore.GetModelSource().toUtf8());
  }
  QFile headerFile(destDir.absoluteFilePath(QString("%1Model.h").arg(mCore.GetClassName())));
  if (headerFile.open(QFile::WriteOnly)) {
    headerFile.write(mCore.GetModelHeader().toUtf8());
  }
}

void MainWindow::on_actionPath_triggered()
{
  mSourceDirectory = QFileDialog::getExistingDirectory(this, "Select table files directory", qApp->applicationDirPath());
}

void MainWindow::on_actionPathModel_triggered()
{
  mSourceModelDirectory = QFileDialog::getExistingDirectory(this, "Select model files directory", qApp->applicationDirPath());
}

void MainWindow::on_actionBatch_triggered()
{
  if (mSqlDirectory.isEmpty()) {
    mSqlDirectory = QFileDialog::getExistingDirectory(this, "Select sql files directory", qApp->applicationDirPath());
  }

  on_actionPath_triggered();

  Core core;
  QString prefix = ui->lineEditPrefix->text();
  QDirIterator iter(mSqlDirectory);
  while (iter.hasNext()) {
    QString file = iter.next();
    QFile sql(file);
    if (sql.open(QFile::ReadOnly)) {
      QString sqlCode = QString::fromUtf8(sql.readAll());
      if (core.Generate(sqlCode, prefix)) {
        QDir destDir(mSourceDirectory);
        QFile cppFile(destDir.absoluteFilePath(QString("%1.cpp").arg(core.GetClassName())));
        if (cppFile.open(QFile::WriteOnly)) {
          cppFile.write(core.GetSource().toUtf8());
        }
        QFile headerFile(destDir.absoluteFilePath(QString("%1.h").arg(core.GetClassName())));
        if (headerFile.open(QFile::WriteOnly)) {
          headerFile.write(core.GetHeader().toUtf8());
        }
      }
    }
  }
}

void MainWindow::on_actionBatchModel_triggered()
{
  if (mSqlDirectory.isEmpty()) {
    mSqlDirectory = QFileDialog::getExistingDirectory(this, "Select sql files directory", qApp->applicationDirPath());
  }

  on_actionPathModel_triggered();

  Core core;
  QString prefix = ui->lineEditPrefix->text();
  QDirIterator iter(mSqlDirectory, QStringList() << "*.sql", QDir::Files);
  while (iter.hasNext()) {
    QString file = iter.next();
    QFile sql(file);
    if (sql.open(QFile::ReadOnly)) {
      QString sqlCode = QString::fromUtf8(sql.readAll());
      if (core.Generate(sqlCode, prefix)) {
        QDir destDir(mSourceModelDirectory);
        QFile cppFile(destDir.absoluteFilePath(QString("%1Model.cpp").arg(core.GetClassName())));
        if (cppFile.open(QFile::WriteOnly)) {
          cppFile.write(core.GetModelSource().toUtf8());
        }
        QFile headerFile(destDir.absoluteFilePath(QString("%1Model.h").arg(core.GetClassName())));
        if (headerFile.open(QFile::WriteOnly)) {
          headerFile.write(core.GetModelHeader().toUtf8());
        }
      }
    }
  }
}
