#include "CRC32.h"


#define POLY  0x04C11DB7 

CRC32::CRC32()
{

}

CRC32::~CRC32()
{

}

uint8_t *CRC32::IntArrayToByte(uint32_t *iArray, uint32_t Size)
{
	uint8_t *bArray = new uint8_t[Size * 4];
	uint32_t pos = 0;
	for (uint32_t i = 0; i < Size; i++)
	{
		uint16_t v = iArray[i];
		bArray[pos++] = (uint8_t)((v & 0xFF000000) >> 24);
		bArray[pos++] = (uint8_t)((v & 0xFF0000) >> 16);
		bArray[pos++] = (uint8_t)((v & 0xFF00) >> 8);
		bArray[pos++] = (uint8_t)(v & 0xFF);
	}
	return bArray;
}

//主流计算方法
uint32_t CRC32::CalcCRC(uint8_t* buffer, uint32_t bufLen, uint32_t crc)
{
	if (buffer == NULL)
		return crc;
	crc = ~crc;
	for (uint32_t i = 0; i < bufLen; i++)
		crc = crcTable[(crc ^ buffer[i]) & 0xFF] ^ (crc >> 8);
	return ~crc;
}

//STM32 硬件兼容的 CRC 计算

uint32_t CRC32::CalcStm32CRCSlow(uint32_t* ptr,uint32_t ptrLen)
{
	uint32_t xbit;
	uint32_t data;
	uint32_t CRC = 0xFFFFFFFF;    // init
	uint32_t len = ptrLen;
	uint32_t pos = 0;
	while (len-- > 0)
	{
		xbit = (uint32_t)0x01 << 31;

		data = ptr[pos++];
		for (int bits = 0; bits < 32; bits++)
		{
			if ((CRC & 0x80000000) != 0)
			{
				CRC <<= 1;
				CRC ^= POLY;
			}
			else
				CRC <<= 1;
			if ((data & xbit) != 0)
				CRC ^= POLY;

			xbit >>= 1;
		}
	}
	return CRC;
}


uint32_t CRC32::CalcStm32CRC(uint8_t* p, uint32_t offset, uint32_t len)
{
	uint32_t pos = offset;
	uint32_t u32len = len / 4;
	uint32_t crc = 0xFFFFFFFF;
	while (u32len-- > 0)
	{
		crc = (crc << 8) ^ crcStm32Table[((crc >> 24) & 0xff) ^ p[pos + 3]];
		crc = (crc << 8) ^ crcStm32Table[((crc >> 24) & 0xff) ^ p[pos + 2]];
		crc = (crc << 8) ^ crcStm32Table[((crc >> 24) & 0xff) ^ p[pos + 1]];
		crc = (crc << 8) ^ crcStm32Table[((crc >> 24) & 0xff) ^ p[pos]];
		pos += 4;
	}
	crc ^= 0x00000000;
	crc &= 0xFFFFFFFF;
	return (crc);

}


