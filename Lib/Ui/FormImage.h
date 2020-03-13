#pragma once

#include <QWidget>
#include <QImage>


namespace Ui {
class FormImage;
}

class FormImage: public QWidget
{
  Ui::FormImage* ui;
  QImage         mImage;
  bool           mShrink;

  Q_OBJECT

public:
  explicit FormImage(QWidget* parent = 0);
  ~FormImage();

protected:
  /*override */virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

public:
  void SetImage(const QImage& image);

private:
  void SetShrink(bool shrink);
  void DrawImage();

private slots:
  void on_actionShrink_triggered(bool checked);
};
