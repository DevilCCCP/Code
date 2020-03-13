#pragma once

#include "Lib\Source\Source.h"
#include "Lib\include\VideoFrame.h"

#pragma warning(push)
#pragma warning(disable: 4005)
#include "live555inc.h"
#pragma warning(pop)

class Playback;

class RtspFrameProcessor : public rtsp_cctv::IFrameProcessor
{
public:
	RtspFrameProcessor(ISourceOwner* pOwner, CodecID codec, const SIZE& szImage, Playback* pParent);
	/*virtual*/ ~RtspFrameProcessor(void);

	virtual void Dispose();
	virtual void ProcessFrame(unsigned char* framePtr, int frameSize, struct timeval presentationTime, unsigned duration, bool key);

private:
	ISourceOwner* m_pOwner;
	CodecID       m_Codec;
	SIZE          m_szImage;
	Playback*     m_pParent;

	VideoFrame m_frame;
	DateTime   mPrevFrameTimeStamp;
};

