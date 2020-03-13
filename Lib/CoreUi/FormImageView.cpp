#include <QScrollBar>
#include <QMouseEvent>
#include <QWheelEvent>

#include "FormImageView.h"
#include "ui_FormImageView.h"


FormImageView::FormImageView(QWidget* parent)
  : QWidget(parent), ui(new Ui::FormImageView)
  , mScale(1), mMousePressPos(-1, -1), mManual(false)
{
  ui->setupUi(this);

  mImageRegion = ui->scrollAreaWidgetContents;

  ui->toolButtonScaleHome->setDefaultAction(ui->actionScaleHome);
  ui->toolButtonScaleIn->setDefaultAction(ui->actionScaleIn);
  ui->toolButtonScaleOut->setDefaultAction(ui->actionScaleOut);

  connect(ui->scrollAreaMain->horizontalScrollBar(), &QScrollBar::valueChanged, this, &FormImageView::OnHorizontalMoved);
  connect(ui->scrollAreaMain->verticalScrollBar(), &QScrollBar::valueChanged, this, &FormImageView::OnVerticalMoved);
}

FormImageView::~FormImageView()
{
  delete ui;
}


void FormImageView::mouseMoveEvent(QMouseEvent* event)
{
  if (event->buttons() == Qt::LeftButton && mMousePressPos.x() > -1) {
    QPoint pos = mMousePressCenter - (event->pos() - mMousePressPos);
    ui->scrollAreaMain->horizontalScrollBar()->setValue(pos.x());
    ui->scrollAreaMain->verticalScrollBar()->setValue(pos.y());
//    OnHorizontalMoved(pos.x());
//    OnVerticalMoved(pos.y());
  }

  QWidget::mouseMoveEvent(event);
}

void FormImageView::mousePressEvent(QMouseEvent* event)
{
  if (event->buttons() == Qt::LeftButton && mImageRegion->rect().contains(event->pos())) {
    mMousePressPos = event->pos();
    mMousePressCenter = QPoint(ui->scrollAreaMain->horizontalScrollBar()->value(), ui->scrollAreaMain->verticalScrollBar()->value());
  } else {
    mMousePressPos = QPoint(-1, -1);
  }

  QWidget::mousePressEvent(event);
}

void FormImageView::mouseReleaseEvent(QMouseEvent* event)
{
  Q_UNUSED(event);

  mMousePressPos = QPoint(-1, -1);

  QWidget::mouseReleaseEvent(event);
}

void FormImageView::wheelEvent(QWheelEvent* event)
{
  if (event->modifiers().testFlag(Qt::ControlModifier)) {
    QPointF deltaScaled = (QPointF)event->pos() - 0.5 * QPointF(width(), height());
    QPointF deltaReal = deltaScaled / mScale;
    int scale = ui->horizontalSliderScale->value() + event->delta() / 120;
    ui->horizontalSliderScale->setValue(scale);
    on_horizontalSliderScale_sliderMoved(ui->horizontalSliderScale->value());
    QPointF deltaScaled2 = deltaReal * mScale;
    QPoint move = (deltaScaled2 - deltaScaled + QPointF(0.5, 0.5)).toPoint();
    QPoint center = QPoint(ui->scrollAreaMain->horizontalScrollBar()->value(), ui->scrollAreaMain->verticalScrollBar()->value());
    QPoint newCenter = center + move;
    ui->scrollAreaMain->horizontalScrollBar()->setValue(newCenter.x());
    ui->scrollAreaMain->verticalScrollBar()->setValue(newCenter.y());
    OnHorizontalMoved(newCenter.x());
    OnVerticalMoved(newCenter.y());
    event->accept();
    return;
  }

  QWidget::wheelEvent(event);
}

void FormImageView::SetImageRegion(FormImageRegion* _ImageRegion)
{
  mImageRegion = _ImageRegion;

  ui->scrollAreaMain->setWidget(mImageRegion);
}

//void FormImageView::resizeEvent(QResizeEvent* event)
//{
//  QWidget::resizeEvent(event);
//}

void FormImageView::SetScale(int value)
{
  mScale = (value >= 0)
      ? 1.0 + 0.1 * value * value
      : 1.0 + 0.02 * value;

  mManual = true;
  mImageRegion->SetScale(mScale);
  int perc = (int)(mScale * 100 + 0.5);
  ui->actionScaleHome->setText(QString("%1%").arg(perc));
  int x = 0.5 * ui->scrollAreaMain->horizontalScrollBar()->maximum() + mScaleCenter.x() * mScale;
  int y = 0.5 * ui->scrollAreaMain->verticalScrollBar()->maximum() + mScaleCenter.y() * mScale;
  ui->scrollAreaMain->horizontalScrollBar()->setValue(x);
  ui->scrollAreaMain->verticalScrollBar()->setValue(y);
  mManual = false;
}

void FormImageView::OnHorizontalMoved(int value)
{
  if (mManual) {
    return;
  }

  qreal x = (value - 0.5 * ui->scrollAreaMain->horizontalScrollBar()->maximum()) / mScale;
  mScaleCenter.setX(x);
}

void FormImageView::OnVerticalMoved(int value)
{
  if (mManual) {
    return;
  }

  qreal y = (value - 0.5 * ui->scrollAreaMain->verticalScrollBar()->maximum()) / mScale;
  mScaleCenter.setY(y);
}

void FormImageView::SetImage(const QImage& image)
{
  mImageRegion->SetImage(image);
}

void FormImageView::on_horizontalSliderScale_sliderMoved(int value)
{
  SetScale(value);
}

void FormImageView::on_actionScaleIn_triggered()
{
  int value = ui->horizontalSliderScale->value() + 1;
  ui->horizontalSliderScale->setValue(value);
  SetScale(value);
}

void FormImageView::on_actionScaleOut_triggered()
{
  int value = ui->horizontalSliderScale->value() - 1;
  ui->horizontalSliderScale->setValue(value);
  SetScale(value);
}

void FormImageView::on_actionScaleHome_triggered()
{
  ui->horizontalSliderScale->setValue(0);
  SetScale(0);
}
