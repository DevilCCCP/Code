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
  bool PrepareScene(const RECT& destRect);
  DdrawSurface* GetIcon(int ind);
  bool LoadIcons();
  bool SurfaceFromImage(const QString& path, DdrawSurface& surface);

public:
  DdrawDrawer(WndProcAS& _MainWindow, const char** _StatusIcons, bool _ScaleBest);
};
