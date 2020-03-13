#include "VirtualSink.h"


namespace rtsp_cctv
{

VirtualSink::VirtualSink(UsageEnvironment& usageEnvironment)
	: MediaSink(usageEnvironment)
	, frameProcessor(NULL)
{
	if (bufferSize <= 0)
	{
		throw "invalid buffer size";
	}
	bufferPtr = new unsigned char[bufferSize];
	this->bufferSize = bufferSize;
}

VirtualSink::~VirtualSink()
{
	if(frameProcessor)
	{
		frameProcessor->Dispose();
		frameProcessor = NULL;
	}

	if(bufferPtr != NULL)
	{
		delete []bufferPtr;
		bufferPtr = NULL;
		bufferSize = 0;
	}
}

VirtualSink* VirtualSink::CreateNew(UsageEnvironment& usageEnvironment, unsigned bufferSize)
{
	try
	{
		return new VirtualSink(usageEnvironment, bufferSize);
	}
	catch(void*)
	{
		return nullptr;
	}
}

void VirtualSink::AddFrameProcessor(IFrameProcessor* fp)
{
	if(frameProcessor)
	{
		frameProcessor->Dispose();
		frameProcessor = NULL;
	}
	if(fp != nullptr)
	{
		frameProcessor = fp;
	}
}

void VirtualSink::ConnectFrameProcessor(FrameProcessorS &fp)
{
}

Boolean VirtualSink::continuePlaying()
{
	if (fSource == NULL)
	{
		//has been stopped
		return False;
	}

	struct proxy
	{
		static void AfterGettingFrame(void* clientData, unsigned frameSize, unsigned truncatedBytesCount, struct timeval presentationTime, unsigned durationInMicroseconds)
		{
			auto sink = static_cast<VirtualSink*>(clientData);
			sink->AfterGettingFrame(frameSize, truncatedBytesCount, presentationTime, durationInMicroseconds);
		}
	};

	fSource->getNextFrame(bufferPtr, bufferSize, proxy::AfterGettingFrame, this, onSourceClosure, this);
	return True;
}

void VirtualSink::AfterGettingFrame(int frameSize, int truncatedBytesCount, struct timeval presentationTime, int durationInMicroseconds)
{
	if(frameProcessor)
	{
		frameProcessor->ProcessFrame(bufferPtr, frameSize, presentationTime, durationInMicroseconds, true);
	}
	if(!continuePlaying())
	{
		onSourceClosure(this);
	}
}

}
