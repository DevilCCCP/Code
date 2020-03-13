#pragma once

#include <QPainter>


class ImagePainter
{
protected:
  /*new */virtual Paint(QPainter* painter)
  { Q_UNUSED(painter); }

public:
  /*new */virtual ~ImagePainter()
  { }
};
