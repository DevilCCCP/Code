#pragma once

#include <QThread>

#include "DiffLayers.h"


class DlWorker: public QThread
{
  DiffLayers*    mParent;

protected:
  /*override */virtual void run();

public:
  DlWorker(DiffLayers* _Parent);
  /*override */virtual ~DlWorker();
};

