#ifndef FORMEARTH_H
#define FORMEARTH_H

#include <QWidget>

namespace Ui {
class FormEarth;
}

class FormEarth : public QWidget
{
  Q_OBJECT

public:
  explicit FormEarth(QWidget *parent = 0);
  ~FormEarth();

private:
  Ui::FormEarth *ui;
};

#endif // FORMEARTH_H
