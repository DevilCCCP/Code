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
  /*override */virtual void SetFrame(FrameS& frame) Q_DECL_OVERRIDE;
  /*override */virtual void SetStatusFrame(FrameS& frame) Q_DECL_OVERRIDE;
  /*override */virtual void SetStatus(EDrawStatus status) Q_DECL_OVERRIDE;
  /*override */virtual void Redraw() Q_DECL_OVERRIDE;
  /*override */virtual void Clear() Q_DECL_OVERRIDE;

  /*override */virtual void SetZoom(int scale) Q_DECL_OVERRIDE;
  /*override */virtual void MoveZoom(const QPointF& pos) Q_DECL_OVERRIDE;

private:
  bool Draw();
  bool PrepareDraw();
  bool PrepareScene(const QRect& destRect);

public:
  QtDrawer(WndProcAS& _MainWindow, const char** _StatusIcons, bool _ScaleBest);
};
