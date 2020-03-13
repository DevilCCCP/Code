#include "JpegResolution.h"


long Mult(BYTE lsb, BYTE msb)
{
	return(lsb + (msb * 256));
}

bool JpegResolution::GetResolution(const unsigned char* buf, int len, SIZE& sz)
{
	sz.cx = sz.cy = 0;

	int lPos = 0;
	int b_pos = 0;
	const unsigned char* bBuf;
	bool bMarkerFound=false;
	bool bExitFlag=false;

	//#define INC_BPOS(count) { b_pos += count; if(b_pos >= len) return false; }
	#define SET_POS(p) { lPos = p; if(lPos >= len) return false; }

	while(true)
	{
		bBuf = (const unsigned char*)&buf[lPos];
		//INC_BPOS(3);

		if(bBuf[0] == 0xFF && bBuf[1] == 0xD8 && bBuf[2] == 0xFF)
		{
			bMarkerFound = true; // 
			break;
		}

		SET_POS(lPos + 1);
	}

	if(!bMarkerFound) // 
    return false;

	SET_POS(lPos + 1);

	while(true)
	{
		while(1)
		{
			bBuf = (const unsigned char*)&buf[lPos];
			//INC_POS(2);

			if(bBuf[0] == 0xFF && bBuf[1] != 0xFF)
				break;

			SET_POS(lPos + 1);
		} //while

		SET_POS(lPos + 1);

		bBuf = (const unsigned char*)&buf[lPos];
		//INC_POS(3);

		switch (bBuf[0])
		{
			case 0xC0:
			case 0xC1:
			case 0xC2:
			case 0xC3:
			case 0xC5:
			case 0xC6:
			case 0xC7:
			case 0xC9:
			case 0xCA:
			case 0xCB:
			case 0xCD:
			case 0xCE:
			case 0xCF:
			{
				if(bBuf[1] == 0 && bBuf[2] == 17)
					bExitFlag=true;
				break;
			}
		}

		if(bExitFlag)
			break;

		SET_POS(lPos + Mult(bBuf[2], bBuf[1]));

	} //while

	SET_POS(lPos + 4);

	bBuf = (const unsigned char*)&buf[lPos];

	//
	sz.cy = Mult(bBuf[1], bBuf[0]);

	SET_POS(lPos + 2);
	bBuf = (const unsigned char*)&buf[lPos];

	//
	sz.cx = Mult(bBuf[1], bBuf[0]);

	return true;
}