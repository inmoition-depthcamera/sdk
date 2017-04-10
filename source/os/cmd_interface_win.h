
#ifndef __CMD_INTERFACE_WIN_H__
#define __CMD_INTERFACE_WIN_H__

#include "cmd_interface.h"
#include <windows.h>

class CmdInterfaceWin:public CmdInterface
{
public:
	CmdInterfaceWin();
	~CmdInterfaceWin();

	// Inherited via CmdInterface
	virtual bool Open(string &port_name) override;
	virtual bool Close() override;
	virtual bool GetUvcRelatedCmdPort(string & uvc_port_name, string & cmd_port_name) override;

protected:

	// Inherited via CmdInterface
	virtual bool ReadFromIO(uint8_t * rx_buf, uint32_t rx_buf_len, uint32_t * rx_len) override;
	virtual bool WriteToIo(const uint8_t * tx_buf, uint32_t tx_buf_len, uint32_t * tx_len) override;


	HANDLE mComHandle;

	OVERLAPPED mOverlappedSend;
	OVERLAPPED mOverlappedRecv;



};

#endif