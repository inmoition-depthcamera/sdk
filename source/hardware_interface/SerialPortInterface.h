
#ifndef __SERIAL_PORT_INTERFACE_H__
#define __SERIAL_PORT_INTERFACE_H__

#include "HardwareInterface.h"

#ifdef _MSC_VER
#include "windows.h"
#include "tchar.h"
#endif

class SerialPortInterface : public HardwareInterface
{
public:
	SerialPortInterface();
	~SerialPortInterface();

	// Inherited via HardwareInterface
	// file_name is the serial port path(\\\\.\\COM? or /dev/tty? or /dev/acm? or /dev/usbtty?),
	// param1 is port num, witch is not used.
	// param2 is baud rate
	virtual int Open(char *file_name, uint32_t param1, uint32_t param2) override;
	virtual int Close() override;
	virtual int ReOpen() override;
	virtual InterfaceType GetInterfaceType() { return SERIAL_PORT; }

private:

	virtual int32_t ReadBufferFromInterface(uint8_t * byBuf, int32_t dwLen, int32_t dwTimeOut = -1) override;
	virtual int32_t WriteBufferToInterface(const uint8_t * byBuf, int32_t dwLen, int32_t dwTimeOut = -1) override;

	uint32_t mPortNo;
	uint32_t mBaud;

#ifdef _MSC_VER
	HANDLE mComHandle;

	DCB mDCB;	

	OVERLAPPED mOverlappedSend;
	OVERLAPPED mOverlappedRecv;
#else

#endif

};


#endif // !__SERIAL_PORT_INTERFACE_H__