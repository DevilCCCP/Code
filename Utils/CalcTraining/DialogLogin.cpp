#include <QStringListModel>
#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QPushButton>

#include "DialogLogin.h"
#include "ui_DialogLogin.h"


DialogLogin::DialogLogin(QWidget* parent)
  : QDialog(parent), ui(new Ui::DialogLogin)
  , mUserList(new QStringListModel(this))
{
  ui->setupUi(this);

  QDir appDir(QCoreApplication::instance()->applicationDirPath());
  appDir.mkdir("Var");
  appDir.cd("Var");

  QStringList loginList;
  QDirIterator iter(appDir.absolutePath(), QStringList() << "*.ini", QDir::Files, QDirIterator::NoIteratorFlags);
  while (iter.hasNext()) {
    QFileInfo fileIndo(iter.next());
    loginList << fileIndo.completeBaseName();
  }
  mUserList->setStringList(loginList);
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
  ui->comboBoxName->setModel(mUserList);
}

DialogLogin::~DialogLogin()
{
  delete ui;
}


QString DialogLogin::Name() const
{
  return ui->comboBoxName->currentText();
}

void DialogLogin::on_comboBoxName_editTextChanged(const QString& text)
{
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!text.trimmed().isEmpty());
}
