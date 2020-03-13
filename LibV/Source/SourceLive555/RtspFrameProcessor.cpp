#include "RtspFrameProcessor.h"
#include "Playback.h"
#include "JpegResolution.h"
#include "H264VirtualSink.h"

#include "Lib/include/SyncTime.h"


RtspFrameProcessor::RtspFrameProcessor(ISourceOwner* pOwner, CodecID codec, const SIZE& szImage, Playback* pParent)
	: m_pOwner(pOwner)
	, m_Codec(codec)
	, m_szImage(szImage)
	, m_pParent(pParent)
	, mPrevFrameTimeStamp(0)
{
	BITMAPINFOHEADER& bmih = m_frame.Header().bmih;

	if(m_Codec == CODEC_ID_MJPEG)
		bmih.biCompression = BI_JPEG;
	else if(m_Codec == CODEC_ID_H264)
	{
		// comment because GetCamModel() was always return 0
		//if(pParent->GetCamModel() == cammodAxis_RTSP)
		//	bmih.biCompression = VideoFrame::BI_AXIS_H264;
		//else
			bmih.biCompression = MAKEFOURCC('H', '2', '6', '4');
	}
	else if(m_Codec == CODEC_ID_MP4ALS)
		bmih.biCompression = MAKEFOURCC('M', 'P', '4', 'V');

	bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmih.biPlanes = 1;
	bmih.biClrImportant = 0;

	m_frame.Header().bmih.biWidth = m_szImage.cx;
	m_frame.Header().bmih.biHeight = m_szImage.cy;
	bmih.biBitCount = 24;
}

RtspFrameProcessor::~RtspFrameProcessor(void)
{
}

void RtspFrameProcessor::Dispose()
{
	delete this;
}

void RtspFrameProcessor::ProcessFrame(unsigned char* framePtr, int frameSize, struct timeval presentationTime, unsigned duration, bool key)
{
	m_pParent->SetCadrExist();
  if (!framePtr) {
    m_pOwner->OnCamState(camstateOk);
    return;
  }

	m_frame.TimeStamp() = GetCurTimeMs();
	if(m_frame.TimeStamp() <= mPrevFrameTimeStamp)
		m_frame.TimeStamp() = ++mPrevFrameTimeStamp;
	else
		mPrevFrameTimeStamp = m_frame.TimeStamp();

	m_frame.Header().keyFrame = key;
	m_frame.Header().bmih.biSizeImage = frameSize;

	if(m_szImage.cx == 0)
	{
		if(m_frame.Header().bmih.biCompression == BI_JPEG)
		{
			if(JpegResolution::GetResolution(framePtr, frameSize, m_szImage))
			{
				m_frame.Header().bmih.biWidth = m_szImage.cx;
				m_frame.Header().bmih.biHeight = m_szImage.cy;
			}
		}
		else if(m_frame.Header().bmih.biCompression == VideoFrame::BI_H264)
		{
			if(!rtsp_cctv::H264VirtualSink::GetFrameResolution(framePtr, frameSize, m_szImage))
				return;

			m_frame.Header().bmih.biWidth = m_szImage.cx;
			m_frame.Header().bmih.biHeight = m_szImage.cy;
		}
	}

  m_frame.SetPtr(framePtr, frameSize);
	m_pOwner->OnImage(m_frame, std::string());
}
