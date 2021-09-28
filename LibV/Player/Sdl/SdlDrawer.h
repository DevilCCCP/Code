#pragma once

#include <QVector>
#include <QRect>

#include <LibV/Include/Frame.h>

#include "Drawer.h"
#include "Icons.h"


DefineClassS(QtDrawer);
struct SDL_Surface;

class QtDrawer: public DeviceDrawer
{
  EDrawStatus           mCurrentStatus;
  QRect                 mLastDestRect;

  int                   mZoomScale;
  QPointF               mZoomPos;
  bool                  mHasStatusFrame;
  bool                  mError;

  SDL_Surface*          mFrameSurface;

public:
  /*override */virtual void SetFrame(FrameS& frame) override;
  /*override */virtual void SetStatusFrame(FrameS& frame) override;
  /*override */virtual void SetStatus(EDrawStatus status) override;
  /*override */virtual void Redraw() override;
  /*override */virtual void Clear() override;

  /*override */virtual void SetZoom(int scale) override;
  /*override */virtual void MoveZoom(const QPointF& pos) override;

private:
  bool Draw();
  bool PrepareDraw();
  bool PrepareScene(const QRect& destRect);

public:
  QtDrawer(WndProcAS& _MainWindow, const char** _StatusIcons, bool _ScaleBest);
};
