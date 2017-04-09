
#include "uvc_interface_direct_show.h"

UvcInterfaceDirectShow::UvcInterfaceDirectShow()
{
}

UvcInterfaceDirectShow::~UvcInterfaceDirectShow()
{
}

bool UvcInterfaceDirectShow::GetUvcCameraList(std::vector<std::string> &camera_list)
{

	return false;
}

bool UvcInterfaceDirectShow::Open(std::string camera_name)
{
	return false;
}

bool UvcInterfaceDirectShow::Close()
{
	return false;
}

bool UvcInterfaceDirectShow::GetFrame(uint8_t * buf, uint32_t buf_len)
{
	return false;
}
