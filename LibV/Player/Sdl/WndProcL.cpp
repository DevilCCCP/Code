#include <QEventLoop>
#include <SDL.h>

#include <Lib/Log/Log.h>
#include <Lib/Dispatcher/Overseer.h>
#include <LibV/Include/ModuleNames.h>

#include "WndProcL.h"
#include "Drawer.h"
#include "Render.h"


bool WndProcL::DoInit()
{
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    return false;
  }

  mHidden = false;
  mMainWindow = SDL_CreateWindow("Sdl Player", 0, 0, 10, 10, SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS | SDL_WINDOW_HIDDEN);
  if (!mMainWindow) {
    Log.Fatal(QString("SDL_CreateWindow fail"), true);
  }
//  GuiThread* t = new GuiThread();
//  t->start();
//  msleep(200);

//  mMainWindow.reset(gImageWithPoints);
//  mMainWindow->setGeometry(getSceneRect());
//  mEventLoop.reset(new QEventLoop());
  return true;
}

void WndProcL::DoRelease()
{
  WndProcA::DoRelease();

  SDL_DestroyWindow(mMainWindow);

  SDL_Quit();
}

void WndProcL::Show()
{
  OnChangedBox();

  SDL_ShowWindow(mMainWindow);
}

void WndProcL::Hide()
{
  SDL_HideWindow(mMainWindow);
}

void WndProcL::ConnectBackWnd()
{
}

void WndProcL::ProcessMsgQueue()
{
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    /* handle the events */
  }
}

void WndProcL::OnChangedTime(const QDateTime& changeTime)
{
}

void WndProcL::OnChangedFps(const qreal& changeFps)
{
}

void WndProcL::OnChangedInfo(const QString& changeInfo)
{
}

void WndProcL::OnChangedBox()
{
  SDL_SetWindowPosition(mMainWindow, getSceneRect().x(), getSceneRect().y());
  SDL_SetWindowSize(mMainWindow, getSceneRect().width(), getSceneRect().height());

  if (!mHidden) {
    SDL_ShowWindow(mMainWindow);
  }
}


WndProcL::WndProcL(Overseer* _Overseer, Render *_Render, Drawer *_Drawer, const QRect &_SceneRect
                 , bool _PrimeWindow, bool _ShowMouse, bool _AutoHideMouse, bool _AlwaysOnTop, bool _PlaySound, EStyleType _Style)
  : WndProcA(_Overseer, _Render, _Drawer, _SceneRect, _PrimeWindow, _ShowMouse, _AutoHideMouse, _AlwaysOnTop, _PlaySound, _Style)
  , mMainWindow(nullptr)
{
}

WndProcL::~WndProcL()
{
}


