
#ifndef __UVC_INTERFACE_LIBUSB_H__
#define __UVC_INTERFACE_LIBUSB_H__

#include "uvc_interface.h"
#include "libuvc/libuvc.h"

class UvcInterfaceLibUvc:public UvcInterface
{
public:
	UvcInterfaceLibUvc();
	~UvcInterfaceLibUvc();


	// Inherited via UvcInterface
	virtual bool GetUvcCameraList(std::vector<std::string>& camera_list, const char * filter) override;

	virtual bool Open(std::string & camera_name) override;

	virtual bool Close() override;
	uvc_context_t *mUvcContext;
	uvc_device_handle_t *mUvcDeviceHandle;
};

#endif