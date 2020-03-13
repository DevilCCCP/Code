#pragma once

#include "VirtualSink.h"
#include <vector>


namespace rtsp_cctv
{
	///<summary></summary>
	class H264VirtualSink : public VirtualSink
	{
	private:
		///<summary>constructor</summary>
		///<param name="usageEnvironment">usage environment</param>
		///<param name="bufferSize">max packet payload</param>
		H264VirtualSink(UsageEnvironment& evn, unsigned bufSize,char const* sPropParameterSetsStr);

		///<summary>destructor, called by MediaSink::close</summary>
		virtual ~H264VirtualSink();

	public:
		///<summary>
		///create new H264VirtualSink object
		///instance must be released by calling MediaSink::close
		///</summary>
		///<param name="usageEnvironment">usage environment</param>
		///<param name="bufferSize">max packet payload</param>
		///<returns>pointer to created H264VirtualSink object, if successed or nullptr otherwise</returns>
		static H264VirtualSink* CreateNew(UsageEnvironment& usageEnvironment, char const* sPropParameterSetsStr = NULL,
																			unsigned bufferSize = (4*1024*1024));
		
		static bool GetFrameResolution(unsigned char* frame, int len, SIZE& res);

	protected:
		typedef uint8_t StartCode4_t[4];
		unsigned  startCodeSize;

	private:
		char const* fSPropParameterSetsStr;
		bool        fHaveWrittenFirstFrame;
		bool        fUse3ByteStartCode;

		static const int fMaxExHeaderSize = 1024;
		std::vector<unsigned char> fExHeader;

		bool m_bFrameSPS;
		bool m_bFramePPS;

		//static bool IsKeyFrame(unsigned char* frame, int len);
		//static bool IsKeyFrameHaveSProps(unsigned char* frame, int len, int* pos = NULL);

		struct ParseData
		{
			bool key;
			bool data;

			struct PS
			{
				bool is;
				int  pos;
				int  len;
			};

			PS sps;
			PS pps;
		};
		static void ParseFrame(unsigned char* frame, int len, ParseData& pd);

		///<summary></summary>
		///<param name=""></param>
		///<param name=""></param>
		///<returns></returns>
		virtual void AfterGettingFrame(unsigned frameSize, unsigned truncatedBytesCount, struct timeval presentationTime, unsigned durationInMicroseconds);
	};
}
