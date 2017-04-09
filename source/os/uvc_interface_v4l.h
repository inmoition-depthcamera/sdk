
#ifndef __WIN_SERIAL_PORT_H__
#define __WIN_SERIAL_PORT_H__

#include "../cmd_interface.h"
#include <windows.h>

class CmdInterfaceWin:public CmdInterface
{
public:
	CmdInterfaceWin();
	~CmdInterfaceWin();

	// Inherited via CmdInterface
	virtual int Open(string port_name) override;
	virtual int Close() override;

protected:

	HANDLE mComHandle;

	// Inherited via CmdInterface
	virtual bool ReadFromIO(uint8_t * rx_buf, uint32_t rx_buf_len, uint32_t * rx_len) override;
	virtual bool WriteToIo(const uint8_t * tx_buf, uint32_t tx_buf_len, uint32_t * tx_len) override;

};

#endif