#ifndef FORMMAPPREVIEW_H
#define FORMMAPPREVIEW_H

#include <QWidget>

namespace Ui {
class FormMapPreview;
}

class FormMapPreview : public QWidget
{
  Q_OBJECT

public:
  explicit FormMapPreview(QWidget *parent = 0);
  ~FormMapPreview();

private:
  Ui::FormMapPreview *ui;
};

#endif // FORMMAPPREVIEW_H
