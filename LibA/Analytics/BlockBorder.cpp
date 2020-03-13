#include "BlockBorder.h"


void BlockBorder::Init(const QRect& block)
{
  mLeft.resize(block.height());
  mRight.resize(0);
  mRight.fill(-1, block.height());

  mTop.resize(block.width());
  mBottom.resize(0);
  mBottom.fill(-1, block.width());
}


BlockBorder::BlockBorder()
{
}

