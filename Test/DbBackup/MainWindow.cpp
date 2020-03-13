#include <Lib/DbUi/FormDbBackup.h>
#include <Lib/Settings/FileSettings.h>

#include "MainWindow.h"
#include "ui_MainWindow.h"


MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent), ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  FileSettings settings;
  if (settings.OpenLocal()) {
    for (int i = 0; ; i++) {
      QStringList tables = settings.GetValue(QString("Table_%1").arg(i, 3, 10, QChar('0')), "").toStringList();
      if (tables.isEmpty() || tables.first().isEmpty()) {
        break;
      }

      QString tableName = tables.takeFirst();
      ui->formDbBackup->AddTable(tableName, QIcon(), tables);
    }
  }
}

MainWindow::~MainWindow()
{
  delete ui;
}
