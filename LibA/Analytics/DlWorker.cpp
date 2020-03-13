#include "DlWorker.h"


void DlWorker::run()
{
  mParent->WaitWorkerStart();
  while (mParent->DoWorkerJob()) {
  }// while (mParent->WaitWorkerJob());
  mParent->DoWorkerJob();
}


DlWorker::DlWorker(DiffLayers* _Parent)
  : mParent(_Parent)
{
}

DlWorker::~DlWorker()
{
}

