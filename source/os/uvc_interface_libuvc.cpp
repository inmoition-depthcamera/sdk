#include "uvc_interface_libuvc.h"
#include <sstream>
#include <iomanip>
#include <cstring>

UvcInterfaceLibUvc::UvcInterfaceLibUvc()
{
    mUvcDeviceHandle = NULL;
    uvc_init(&mUvcContext, NULL);
}

UvcInterfaceLibUvc::~UvcInterfaceLibUvc()
{
    if(mUvcContext != NULL){
        uvc_exit(mUvcContext);
        mUvcContext = NULL;
    }
}

bool UvcInterfaceLibUvc::GetUvcCameraList(std::vector<std::string>& camera_list, const char * filter)
{
    int32_t dev_idx = 0;
    uvc_error_t ret;
    uvc_device_t **dev_list = NULL;
    ret = uvc_get_device_list(mUvcContext, &dev_list);
    if (ret != UVC_SUCCESS) {
        return false;
    }
    uvc_device_t *dev;
    while ((dev = dev_list[dev_idx++]) != NULL) {
        uvc_device_descriptor_t *desc;
        if (uvc_get_device_descriptor(dev, &desc) != UVC_SUCCESS)
            continue;
       // if(filter == NULL || strstr(desc->product, filter)){
            std::ostringstream oss;
            oss << std::showbase
                << "vid" << std::hex << desc->idVendor  << "&"
                << "pid" << std::hex << desc->idProduct << "&"
                << "sn:" << desc->serialNumber << "&"
                << desc->manufacturer << "__" << desc->product;
            camera_list.push_back(oss.str());
       // }
        uvc_free_device_descriptor(desc);
    }
    uvc_free_device_list(dev_list, 1);
	return true;
}

bool UvcInterfaceLibUvc::Open(std::string & camera_name)
{
    uvc_device_t *dev;
    uint32_t vid = std::stoi(camera_name.substr(camera_name.find("vid") + 3), nullptr, 16);
    uint32_t pid = std::stoi(camera_name.substr(camera_name.find("pid") + 3), nullptr, 16);
    std::string sn =camera_name.substr(camera_name.find("sn:") + 3);
    sn = sn.substr(0, sn.find('&'));
    uvc_error_t res = uvc_find_device(mUvcContext, &dev, vid, pid, sn.c_str());
    if(res < 0)
        return false;
    res = uvc_open(dev, &mUvcDeviceHandle);
    if(res < 0)
        return false;

	return false;
}

bool UvcInterfaceLibUvc::Close()
{


	return true;
}