
#ifndef __UVC_INTERFACE_DIRECT_SHOW_H__
#define __UVC_INTERFACE_DIRECT_SHOW_H__

#include "../uvc_interface.h"
#include <windows.h>

class UvcInterfaceDirectShow:public UvcInterface
{
public:
	UvcInterfaceDirectShow();
	~UvcInterfaceDirectShow();

	// Inherited via UvcInterface
	virtual bool GetUvcCameraList(std::vector<std::string> &camera_list) override;
	virtual bool Open(std::string camera_name) override;
	virtual bool Close() override;
	virtual bool GetFrame(uint8_t * buf, uint32_t buf_len) override;

};

#endif