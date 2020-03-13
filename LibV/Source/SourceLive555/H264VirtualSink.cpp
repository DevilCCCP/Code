//#pragma warning (push, 0)
#include "H264VirtualSink.h"
#include "H264SPropParameterSetParser.h"

#include "atlenc.h"
//#pragma warning pop

namespace rtsp_cctv
{

H264VirtualSink* H264VirtualSink::CreateNew(UsageEnvironment& usageEnvironment, char const* sPropParameterSetsStr, unsigned bufferSize)
{
	try
	{
		return new H264VirtualSink(usageEnvironment, bufferSize,sPropParameterSetsStr);
	}
	catch(void*)
	{
		return nullptr;
	}
}

H264VirtualSink::H264VirtualSink(UsageEnvironment& evn, unsigned bufSize, char const* sPropParameterSetsStr)
	: VirtualSink(evn, bufSize + sizeof(StartCode4_t) + fMaxExHeaderSize)
	, fSPropParameterSetsStr(sPropParameterSetsStr)
	, fHaveWrittenFirstFrame(false)
	, fUse3ByteStartCode(true) // always use 3ByteStartCode
	, m_bFrameSPS(false)
	, m_bFramePPS(false)
{
	static const StartCode4_t startCode = {0x00, 0x00, 0x00, 0x01};

	memcpy(this->bufferPtr + fMaxExHeaderSize, (void*)startCode, sizeof(StartCode4_t));
	this->bufferPtr += (sizeof(StartCode4_t) + fMaxExHeaderSize);
	this->bufferSize -= (sizeof(StartCode4_t) + fMaxExHeaderSize);
}

H264VirtualSink::~H264VirtualSink()
{
	bufferPtr -= (sizeof(StartCode4_t) + fMaxExHeaderSize);
	this->bufferSize += (sizeof(StartCode4_t) + fMaxExHeaderSize);
}

//bool H264VirtualSink::IsKeyFrame(unsigned char* frame, int len)
//{
//	static const uint8_t startCode[] = {0x00, 0x00, 0x01};
//
//	static const int iCheckHeadLen = fMaxExHeaderSize;
//	static const int iCorrectHeadLen = sizeof(startCode) + 2;
//
//	if(len <= iCorrectHeadLen)
//		return false;
//
//	int to = std::min(len, iCheckHeadLen) - iCorrectHeadLen;
//
//	for(int i = 0; i != to; i++)
//	{
//		if(!memcmp(frame + i, startCode, sizeof(startCode)))
//		{
//			int bn = i + sizeof(startCode);
//
//			int fragment_type = frame[bn] & 0x1F; // nal_unit_type
//			int nal_type      = frame[bn + 1] & 0x1F; // nal_unit_type if fragment_type == 28 || 29
//			int start_bit     = frame[bn + 1] & 0x80;
//			int end_bit       = frame[bn + 1] & 0x40;
//
//			if(((fragment_type == 28 || fragment_type == 29) && nal_type == 5 && start_bit == 128) || fragment_type == 5)
//				return true;
//		}
//	}
//
//	return false;
//}
//
//bool H264VirtualSink::IsKeyFrameHaveSProps(unsigned char* frame, int len, int* pos)
//{
//	static const uint8_t startCode[] = {0x00, 0x00, 0x01};
//
//	static const int iCheckHeadLen = fMaxExHeaderSize;
//	static const int iCorrectHeadLen = sizeof(startCode) + 1;
//
//	if(len <= iCorrectHeadLen)
//		return false;
//
//	int to = std::min(len, iCheckHeadLen) - iCorrectHeadLen;
//
//	for(int i = 0; i != to; i++)
//	{
//		if(!memcmp(frame + i, startCode, sizeof(startCode)))
//		{
//			if((frame[i + sizeof(startCode)] & 0x1f) == 7/*0x67*/)
//			{
//				if(pos)
//					*pos = i + sizeof(startCode);
//				return true;
//			}
//		}
//	}
//
//	return false;
//}

void H264VirtualSink::ParseFrame(unsigned char* frame, int len, ParseData& pd)
{
	memset(&pd, 0, sizeof(pd));

	static const uint8_t startCode[] = {0x00, 0x00, 0x01};

	static const int iCheckHeadLen = fMaxExHeaderSize;
	static const int iCorrectHeadLen = sizeof(startCode) + 2;

	if(len <= iCorrectHeadLen)
		return;

	int to = std::min(len, iCheckHeadLen) - iCorrectHeadLen;

	static const int chTotal = 4;
	int chCount = 0;

	ParseData::PS* ps = NULL;

	for(int i = 0; i != to; i++)
	{
		if(!memcmp(frame + i, startCode, sizeof(startCode)))
		{
			if(ps)
			{
				ps->len = i - ps->pos;
				ps = NULL;

				chCount++;
			}

			int bn = i + sizeof(startCode);

			int fragment_type = frame[bn] & 0x1F; // nal_unit_type
			int nal_type      = frame[bn + 1] & 0x1F; // nal_unit_type if fragment_type == 28 || 29
			int start_bit     = frame[bn + 1] & 0x80;
			int end_bit       = frame[bn + 1] & 0x40;

			if(((fragment_type == 28 || fragment_type == 29) && nal_type == 5 && start_bit == 128) || fragment_type == 5)
			{
				pd.key = true;
				chCount++;

				if(!pd.data)
				{
					pd.data = true;
					chCount++;
				}
			}
			else
			{
				nal_type = fragment_type;

				if(nal_type == 7) // SPS
				{
					ps = &pd.sps;
					ps->is = true;
					ps->pos = i + sizeof(startCode);
				}
				else if(nal_type == 8) // PPS
				{
					ps = &pd.pps;
					ps->is = true;
					ps->pos = i + sizeof(startCode);
				}
				else if(nal_type < 6)
				{
					if(!pd.data)
					{
						pd.data = true;
						chCount++;
					}
				}
			}
		}

		if(chCount >= chTotal)
			break;
	}

	if(ps)
		ps->len = to - ps->pos + iCorrectHeadLen;
}

bool H264VirtualSink::GetFrameResolution(unsigned char* frame, int len, SIZE& res)
{
	//if(!IsKeyFrame(frame, len))
	//	return false;

	//int pos;
	//if(!IsKeyFrameHaveSProps(frame, len, &pos))
	//	return false;

	//H264SPropParameterSetParser parser(&frame[pos]);

	ParseData pd;
	ParseFrame(frame, len, pd);

	if(!pd.sps.is)
		return false;

	H264SPropParameterSetParser parser(&frame[pd.sps.pos]);

	res.cx = parser.Width();
	res.cy = parser.Height();

	return true;
}

//void H264VirtualSink::AfterGettingFrame(unsigned frameSize, unsigned truncatedBytesCount, struct timeval presentationTime, unsigned durationInMicroseconds)
//{
//	static const uint8_t startCode3[] = {0x00, 0x00, 0x01};
//	static const uint8_t startCode4[] = {0x00, 0x00, 0x00, 0x01};
//
//	if(frameProcessor)
//	{
//		auto correctedFrameSize = frameSize;
//		auto correctedBufferPtr = bufferPtr;
//			
//		int iByteNum = sizeof(StartCode4_t);
//
//		if(frameSize < sizeof(startCode4) || memcmp(startCode4, bufferPtr, sizeof(startCode4)) != 0)
//		{
//			if(frameSize < sizeof(startCode3) || memcmp(startCode3, bufferPtr, sizeof(startCode3)) != 0)
//			{
//				if(fUse3ByteStartCode)
//				{
//					iByteNum = sizeof(startCode3);
//
//					correctedFrameSize += sizeof(startCode3);
//					correctedBufferPtr -= sizeof(startCode3);
//				}
//				else
//				{
//					correctedFrameSize += sizeof(StartCode4_t);
//					correctedBufferPtr -= sizeof(StartCode4_t);
//				}
//			}
//		}
//
//		ParseData pd;
//		ParseFrame(correctedBufferPtr, correctedFrameSize, pd);
//		char ccc[1000];
//		sprintf_s(ccc, "size: %d, key: %d, data: %d, SPS: %d, PPS: %d\n", correctedFrameSize, (int)pd.key, (int)pd.data, (int)pd.sps.is, (int)pd.pps.is);
//		OutputDebugStringA(ccc);
//
//
//		//// check I-Frame
//		//int fragment_type = correctedBufferPtr[iByteNum] & 0x1F;
//		//int nal_type      = correctedBufferPtr[iByteNum + 1] & 0x1F;
//		//int start_bit     = correctedBufferPtr[iByteNum + 1] & 0x80;
//		//int end_bit       = correctedBufferPtr[iByteNum + 1] & 0x40;
//
//		//bool bKey = false;
//		//if(((fragment_type == 28 || fragment_type == 29) && nal_type == 5 && start_bit == 128) || fragment_type == 5)
//		//	bKey = true;
//		
//		bool bKey = IsKeyFrame(correctedBufferPtr, correctedFrameSize);
//
//		if(!fHaveWrittenFirstFrame)
//		{
//			if(!bKey)
//			{
//				if(!continuePlaying())
//					onSourceClosure(this);
//				return;
//			}
//
//			fHaveWrittenFirstFrame = true;
//
//			// 
//			if(fSPropParameterSetsStr && !IsKeyFrameHaveSProps(bufferPtr, frameSize))
//			{
//				std::vector<char> sProp;
//				sProp.resize(strlen(fSPropParameterSetsStr) + 3);
//				strcpy_s(sProp.data(), sProp.size(), fSPropParameterSetsStr);
//				strcat_s(sProp.data(), sProp.size(), ",");
//
//				std::vector<std::vector<unsigned char> > heads;
//				int heads_len = 0;
//
//				char* spStr = sProp.data();
//				char* sEnd = strchr(spStr, ',');
//				while(sEnd)
//				{
//					sEnd[0] = 0;
//
//					int h_len = Base64DecodeGetRequiredLength(strlen(spStr));
//					std::vector<unsigned char> h;
//					h.resize(h_len);
//
//					Base64Decode(spStr, strlen(spStr), h.data(), &h_len);
//
//					h.resize(h_len);
//					heads_len += h_len;
//
//					heads.push_back(h);
//
//					spStr = sEnd + 1;
//					sEnd = strchr(spStr, ',');
//				}
//
//				if(heads_len)
//				{
//					fExHeader.resize(heads_len + (fUse3ByteStartCode ? sizeof(startCode3): sizeof(startCode4))*heads.size());
//					if(fExHeader.size() <= fMaxExHeaderSize)
//					{
//						unsigned char* fExHeaderData = fExHeader.data();
//
//						for(auto it = heads.begin(); it != heads.end(); it++)
//						{
//							memcpy(fExHeaderData, fUse3ByteStartCode ? startCode3: startCode4, fUse3ByteStartCode ? sizeof(startCode3): sizeof(startCode4));
//							fExHeaderData += (fUse3ByteStartCode ? sizeof(startCode3): sizeof(startCode4));
//
//							memcpy(fExHeaderData, it->data(), it->size());
//							fExHeaderData += it->size();
//						}
//					}
//					else
//						fExHeader.clear();
//				}
//			}
//		}
//
//		//TCHAR c[100];
//		//wsprintf(c, TEXT("sb: %d, eb: %d, key: %d, nal_type: %d, len: %d\n"), start_bit, end_bit, (int)bKey, nal_type, correctedFrameSize);
//		//OutputDebugString(c);
//
//		if(bKey && !fExHeader.empty())
//		{ // 
//			correctedFrameSize += fExHeader.size();
//			correctedBufferPtr -= fExHeader.size();
//
//			memcpy(correctedBufferPtr, fExHeader.data(), fExHeader.size());
//		}
//
//
//		//if(correctedFrameSize != 53)
//		frameProcessor->ProcessFrame(correctedBufferPtr, correctedFrameSize, presentationTime, durationInMicroseconds, bKey);
//	}
//
//	if(!continuePlaying())
//	{
//		onSourceClosure(this);
//	}
//}

void H264VirtualSink::AfterGettingFrame(unsigned frameSize, unsigned truncatedBytesCount, struct timeval presentationTime, unsigned durationInMicroseconds)
{
	static const uint8_t startCode3[] = {0x00, 0x00, 0x01};
	static const uint8_t startCode4[] = {0x00, 0x00, 0x00, 0x01};

	if(frameProcessor)
	{
		auto correctedFrameSize = frameSize;
		auto correctedBufferPtr = bufferPtr;
			
		int iByteNum = sizeof(StartCode4_t);

		if(frameSize < sizeof(startCode4) || memcmp(startCode4, bufferPtr, sizeof(startCode4)) != 0)
		{
			if(frameSize < sizeof(startCode3) || memcmp(startCode3, bufferPtr, sizeof(startCode3)) != 0)
			{
				if(fUse3ByteStartCode)
				{
					iByteNum = sizeof(startCode3);

					correctedFrameSize += sizeof(startCode3);
					correctedBufferPtr -= sizeof(startCode3);
				}
				else
				{
					correctedFrameSize += sizeof(StartCode4_t);
					correctedBufferPtr -= sizeof(StartCode4_t);
				}
			}
		}

		ParseData pd;
		ParseFrame(correctedBufferPtr, correctedFrameSize, pd);

		//char ccc[1000];
		//sprintf_s(ccc, "size: %d, key: %d, data: %d, SPS: %d, PPS: %d\n", correctedFrameSize, (int)pd.key, (int)pd.data, (int)pd.sps.is, (int)pd.pps.is);
		//OutputDebugStringA(ccc);

		if(!pd.data)
		{
			if(!fSPropParameterSetsStr)
			{
				if(pd.sps.is || pd.pps.is)
				{
					if(pd.sps.is && !m_bFrameSPS)
					{
						int st = fExHeader.size();
						fExHeader.resize(st + pd.sps.len + sizeof(startCode3));

						memcpy(fExHeader.data() + st, startCode3, sizeof(startCode3));
						st += sizeof(startCode3);

						memcpy(fExHeader.data() + st, correctedBufferPtr + pd.sps.pos, pd.sps.len);
						m_bFrameSPS = true;
					}
					if(pd.pps.is && !m_bFramePPS)
					{
						int st = fExHeader.size();
						fExHeader.resize(st + pd.pps.len + sizeof(startCode3));

						memcpy(fExHeader.data() + st, startCode3, sizeof(startCode3));
						st += sizeof(startCode3);

						memcpy(fExHeader.data() + st, correctedBufferPtr + pd.pps.pos, pd.pps.len);
						m_bFramePPS = true;
					}
				}
			}

			if(!continuePlaying())
				onSourceClosure(this);
			return;
		}

		if(!fHaveWrittenFirstFrame)
		{
			if(!pd.key)
			{
        frameProcessor->ProcessFrame(nullptr, 0, presentationTime, durationInMicroseconds, pd.key);
				if(!continuePlaying())
					onSourceClosure(this);
				return;
			}

			fHaveWrittenFirstFrame = true;

			// 
			if(fSPropParameterSetsStr && !pd.sps.is)
			{
				std::vector<char> sProp;
				sProp.resize(strlen(fSPropParameterSetsStr) + 3);
				strcpy_s(sProp.data(), sProp.size(), fSPropParameterSetsStr);
				strcat_s(sProp.data(), sProp.size(), ",");

				std::vector<std::vector<unsigned char> > heads;
				int heads_len = 0;

				char* spStr = sProp.data();
				char* sEnd = strchr(spStr, ',');
				while(sEnd)
				{
					sEnd[0] = 0;

					int h_len = Base64DecodeGetRequiredLength(strlen(spStr));
					std::vector<unsigned char> h;
					h.resize(h_len);

					Base64Decode(spStr, strlen(spStr), h.data(), &h_len);

					h.resize(h_len);
					heads_len += h_len;

					heads.push_back(h);

					spStr = sEnd + 1;
					sEnd = strchr(spStr, ',');
				}

				if(heads_len)
				{
					fExHeader.resize(heads_len + (fUse3ByteStartCode ? sizeof(startCode3): sizeof(startCode4))*heads.size());
					if(fExHeader.size() <= fMaxExHeaderSize)
					{
						unsigned char* fExHeaderData = fExHeader.data();

						for(auto it = heads.begin(); it != heads.end(); it++)
						{
							memcpy(fExHeaderData, fUse3ByteStartCode ? startCode3: startCode4, fUse3ByteStartCode ? sizeof(startCode3): sizeof(startCode4));
							fExHeaderData += (fUse3ByteStartCode ? sizeof(startCode3): sizeof(startCode4));

							memcpy(fExHeaderData, it->data(), it->size());
							fExHeaderData += it->size();
						}
					}
					else
						fExHeader.clear();
				}
			}
		}

		if(pd.key && !pd.sps.is && !fExHeader.empty())
		{ // 
			correctedFrameSize += fExHeader.size();
			correctedBufferPtr -= fExHeader.size();

			memcpy(correctedBufferPtr, fExHeader.data(), fExHeader.size());
		}


		frameProcessor->ProcessFrame(correctedBufferPtr, correctedFrameSize, presentationTime, durationInMicroseconds, pd.key);
	}

	if(!continuePlaying())
	{
		onSourceClosure(this);
	}
}

}
