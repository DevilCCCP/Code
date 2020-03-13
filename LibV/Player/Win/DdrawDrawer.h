#pragma once

#include <QVector>
#include <QRect>

#include <LibV/Include/Frame.h>

#include "../Drawer.h"
#include "DirectDraw.h"
#include "../Icons.h"

DefineClassS(DdrawDrawer);

class DdrawDrawer: public DeviceDrawer
{
  HWND                  mMainWindow;
  DirectDrawS           mDirectDraw;
  DdrawSurface          mBackFrame;
  DdrawSurface          mStatusFrame;
  DdrawSurface          mBackScene;
  EDrawStatus           mCurrentStatus;
  QVector<DdrawSurface> mStatusIcons;
  QRect                 mLastDestRect;

  int                   mZoomScale;
  QPointF               mZoomPos;
  bool                  mHasStatusFrame;
  bool                  mError;

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
  bool PrepareScene(const RECT& destRect);
  DdrawSurface* GetIcon(int ind);
  bool LoadIcons();
  bool SurfaceFromImage(const QString& path, DdrawSurface& surface);

public:
  DdrawDrawer(WndProcAS& _MainWindow, const char** _StatusIcons, bool _ScaleBest);
};
