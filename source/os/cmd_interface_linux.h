
#ifndef __WIN_SERIAL_PORT_H__
#define __WIN_SERIAL_PORT_H__

#include "cmd_interface.h"

class CmdInterfaceLinux:public CmdInterface
{
public:
	CmdInterfaceLinux();
	~CmdInterfaceLinux();


	// Inherited via CmdInterface
	virtual bool Open(string & port_name) override;
	virtual bool Close() override;
	virtual bool GetUvcRelatedCmdPort(string & uvc_port_name, string & cmd_port_name) override;
	virtual bool ReadFromIO(uint8_t * rx_buf, uint32_t rx_buf_len, uint32_t * rx_len) override;
	virtual bool WriteToIo(const uint8_t * tx_buf, uint32_t tx_buf_len, uint32_t * tx_len) override;
    virtual bool GetCmdDevices(std::vector<std::pair<std::string, std::string>> &device_list) override;

private:

    int32_t mComHandle;
};

#endif