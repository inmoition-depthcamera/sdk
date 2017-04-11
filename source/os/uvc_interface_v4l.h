
#ifndef __UVC_INTERFACE_V4L_H__
#define __UVC_INTERFACE_V4L_H__

#include "uvc_interface.h"

class UvcInterfaceV4L:public UvcInterface
{
public:
	UvcInterfaceV4L();
	~UvcInterfaceV4L();

	// Inherited via UvcInterface
	virtual bool GetUvcCameraList(std::vector<std::string>& camera_list, const char * filter) override;

	virtual bool Open(std::string & camera_name) override;

	virtual bool Close() override;

};

#endif