#pragma once
#include <string>
#include <vector>
#include <Lib/include/Common.h>
#include "include/codec.h"

namespace rtsp_cctv
{

	////forward declaration of classes
	//class Live555;
	class VirtualSink;
	//class VideoRenderer;
	//class VideoDecoder;

	struct VideoBuffer
	{
		int         width;
		int         height;
		PixelFormat pixelFormat;
		int         stride[4];
		byte*       scan0[4];
		byte*       signal;
	};

	enum StreamTransport
	{
		Udp,
		Tcp,
		Http
	};

	struct MediaStreamInfo
	{
		std::string      Url;
		std::string      Username;
		std::string      Password;
		StreamTransport  Transport;

    std::vector<std::string> ProfilesUri;
    size_t                   NextProfile;

    MediaStreamInfo(): NextProfile(0) { }
	};
	
	enum VideoPlaybackMode
	{
		//normal playback, decoding and render to video buffer
		Rendering,

		//decoded frames are not copied to video buffer
		//this mode intended to be used as fast variant of pause
		DecodingOnly
	};

	//interfaces

	///<summary></summary>
	class IDisposable
	{
	public:
		///<summary></summary>
		///<param name=""></param>
		///<returns></returns>
		virtual void Dispose() = 0;
	};

	///<summary></summary>
	class ILive555Unit: public IDisposable
	{
	public:
		///<summary></summary>
		///<returns></returns>
		//virtual UsageEnvironment& GetUsageEnvironment();
	};

	///<summary></summary>
	class IFrameProcessor: public ILive555Unit
	{
	public:
		///<summary></summary>
		///<param name="frame">pointer to frame</param>
		///<param name="frameSize">size of frame in bytes</param>
		///<param name="presentationTime">time of frame presentation</param>
		///<param name="duration">duration of frame presentation in microseconds</param>
		virtual void ProcessFrame(unsigned char* framePtr, int frameSize, struct timeval presentationTime, unsigned duration, bool key) = 0;
		
		///<summary></summary>
		///<param name="reason"></param>
		//virtual void Shutdown(int reason) = 0;
	};

	//class IVideoRenderer: public ILive555Unit
	//{
	//public:
	//	//virtual void RenderFrame(AVCodecContext* avCodecContext, AVFrame* avFrame)=0;
	//};

	//class IPlaybackSession: public IDisposable
	//{
	//	//StartRecord
	//	//StopRecord
	//	//SetPlaybackMode
	//};

	//class IPlaybackController
	//{
	//public:
	//	virtual bool Initialized(IPlaybackSession* session)=0;
	//	virtual void Shutdown()=0;
	//};

	//typedef std::function<std::shared_ptr<VirtualSink*> (UsageEnvironment*)> VirtualSinkFactory;
	//typedef std::function<std::shared_ptr<IFrameProcessor> (VirtualSink* sink)> IFrameProcessorFactory;
	//typedef function<shared_ptr<IVideoRenderer> (VideoDecoder* videoDecoder)> IVideoRendererFactory;
	
}
