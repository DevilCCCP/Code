#include <Lib/Log/Log.h>

#include "HandlerManager.h"


HandlerS HandlerManager::CreateHandler()
{
  return NewHandler();
}

HandlerS HandlerManager::NewHandler()
{
  return HandlerS(new Handler());
}


HandlerManagerS HandlerManager::New()
{
  return HandlerManagerS(new HandlerManager());
}

HandlerManager::~HandlerManager()
{
}
