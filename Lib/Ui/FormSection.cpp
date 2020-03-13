#include "FormSection.h"
#include "ui_FormSection.h"


FormSection::FormSection(bool _Show, QWidget* parent)
  : QWidget(parent), ui(new Ui::FormSection)
{
  ui->setupUi(this);

  mShownIcon = QIcon(":/Icons/Section shown.png");
  mHiddenIcon = QIcon(":/Icons/Section hidden.png");

  SetShow(_Show);
}

FormSection::~FormSection()
{
  delete ui;
}


void FormSection::SetShow(bool _Show)
{
  mShow = _Show;
  ui->frameMain->setVisible(mShow);
  ui->toolButtonShow->setIcon(mShow? mShownIcon: mHiddenIcon);
  this->setSizePolicy(mShow
                      ? QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred)
                      : QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));
}

void FormSection::SetWidget(QWidget* _MainWidget, const QString& _Tytle, QWidget* _CollapseWidget)
{
  ui->verticalLayoutMain->addWidget(_MainWidget);
  if (_CollapseWidget) {
    ui->horizontalLayoutHeader->addWidget(_CollapseWidget);
  }
  ui->labelTytle->setText(_Tytle);
}

void FormSection::on_toolButtonShow_clicked()
{
  mShow = !mShow;

  OnShowChanging(mShow);
  SetShow(mShow);
  OnShowChanged(mShow);
}
