#include <QTimer>

#include <Lib/Updater/UpInfo.h>
#include <Lib/Log/Log.h>

#include "FormUpdateSync.h"
#include "ui_FormUpdateSync.h"


FormUpdateSync::FormUpdateSync(QWidget* parent)
  : QWidget(parent), ui(new Ui::FormUpdateSync)
{
  Q_INIT_RESOURCE(Ui);

  ui->setupUi(this);

#ifdef LANG_EN
  ui->labelUpdateAvailable->setText("New update is available and will be installed in ...");
  ui->commandLinkButtonNow->setText("Install as fast, as possible");
  ui->commandLinkButtonWaitMin->setText("Delay for 2 m");
  ui->commandLinkButtonWaitMax->setText("Delay for 30 m (max)");
  ui->commandLinkButtonWaitCustom->setText("Delay for ...");
  ui->doubleSpinBoxCustom->setSuffix(" m");
  parent->setWindowTitle("Update");
  mSecsFormat = "%1 s";
#else
  ui->labelUpdateAvailable->setText("Загружено обновление, оно будет установлено через ... ");
  ui->commandLinkButtonNow->setText("Установить без задержек");
  ui->commandLinkButtonWaitMin->setText("Отложить на 2 м");
  ui->commandLinkButtonWaitMax->setText("Отложить на 30 м (максимум)");
  ui->commandLinkButtonWaitCustom->setText("Отложить на ...");
  ui->doubleSpinBoxCustom->setSuffix(" м");
  parent->setWindowTitle("Обновление");
  mSecsFormat = "%1 с";
#endif

  QFont baseFont = ui->labelUpdateAvailable->font();
  baseFont.setPixelSize(18);
  ui->labelUpdateAvailable->setFont(baseFont);
  ui->labelSecs->setFont(baseFont);
}

FormUpdateSync::~FormUpdateSync()
{
  delete ui;
}


void FormUpdateSync::OnUpdateSecs(int secs)
{
  ui->labelSecs->setText(mSecsFormat.arg(secs));
}

void FormUpdateSync::on_commandLinkButtonNow_clicked()
{
  emit SelectUserUpStart(0);
  parentWidget()->close();
}

void FormUpdateSync::on_commandLinkButtonWaitMin_clicked()
{
  emit SelectUserUpStart(2 * 60 * 1000);
  parentWidget()->close();
}

void FormUpdateSync::on_commandLinkButtonWaitMax_clicked()
{
  emit SelectUserUpStart(30 * 60 * 1000);
  parentWidget()->close();
}

void FormUpdateSync::on_commandLinkButtonWaitCustom_clicked()
{
  qreal val = qBound(1.0, ui->doubleSpinBoxCustom->value(), 30.0);
  emit SelectUserUpStart((int)(60 * 1000 * val));
  parentWidget()->close();
}
