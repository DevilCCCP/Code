#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#ifdef __cplusplus
}
#endif

#include <LibV/Include/Frame.h>


DefineClassS(Convert);

class Convert
{
  AVPixelFormat     mPixFormat;
  int               mWidth;
  int               mHeight;
  SwsContext*       mSwsContext;

public:
  bool ConvertFrame(ECompression compression, const FrameS& srcFrame, FrameS& dstFrame);

private:
  bool ConvertToY(const FrameS& srcFrame, FrameS& frame);
  bool ConvertToNv12(const FrameS& srcFrame, FrameS& frame);

  Frame::Header* InitFrame(FrameS& frame, int fullSize, int key);

public:
  Convert();
};
