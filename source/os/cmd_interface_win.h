
#ifndef __CMD_INTERFACE_WIN_H__
#define __CMD_INTERFACE_WIN_H__

#include "cmd_interface.h"
#include <windows.h>

class CmdInterfaceWin:public CmdInterface
{
public:
	CmdInterfaceWin();
	~CmdInterfaceWin();

	/// @brief Open cmd port with given name
	/// 
	/// The port name can be found by GetDepthCameraList and GetUvcRelatedCmdPort function.
	///
	/// @param port_name The cmd port name to be opend
	/// @return Return ture if successed
	virtual bool Open(string &port_name) override;

	/// @brief Close the uvc port
	/// @return Return ture if successed
	virtual bool Close() override;

	/// @brief Get the cmd port name by uvc port name
	/// @param uvc_port_name The uvc port name, which could be found by GetDepthCameraList function
	/// @param cmd_port_name The output cmd port name, which could be used by Open function
	/// @return Return ture if successed
	virtual bool GetUvcRelatedCmdPort(string & uvc_port_name, string & cmd_port_name) override;

protected:

	// Inherited via CmdInterface
	virtual bool ReadFromIO(uint8_t * rx_buf, uint32_t rx_buf_len, uint32_t * rx_len) override;
	virtual bool WriteToIo(const uint8_t * tx_buf, uint32_t tx_buf_len, uint32_t * tx_len) override;
	virtual bool GetCmdDevices(std::vector<std::pair<std::string, std::string>>& device_list) override;

	HANDLE mComHandle;

	OVERLAPPED mOverlappedSend;
	OVERLAPPED mOverlappedRecv;

};

#endif