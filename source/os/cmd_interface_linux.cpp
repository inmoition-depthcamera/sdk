
#include "cmd_interface_linux.h"

#include <sys/file.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <string.h>
#include <memory.h>

#ifdef USE_UDEV
	#include <libudev.h>
#endif

CmdInterfaceLinux::CmdInterfaceLinux()
{
}

CmdInterfaceLinux::~CmdInterfaceLinux()
{
}

bool CmdInterfaceLinux::Open(string & port_name)
{
	int flags = (O_RDWR | O_NOCTTY | O_CLOEXEC | O_SYNC);
	mComHandle = open(port_name.c_str(), flags);
	if (-1 == mComHandle) {
		return false;
	}

	if (-1 == fcntl(mComHandle, F_SETFD, FD_CLOEXEC)) {
		Close();
		return false;
	}

	// get port options
	struct termios options;
	if (-1 == tcgetattr(mComHandle, &options)) {
		Close();
		return false;
	}

	// Get port configuration for modification
	tcgetattr(mComHandle, &options);
	options.c_iflag = IGNPAR;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;
	options.c_cflag |= CRTSCTS;
	options.c_cflag &= ~PARENB;	//disable parity
	options.c_cflag &= ~CSTOPB;	//1 stop bit
	options.c_cflag |= CLOCAL;  // ignore status lines
	options.c_cflag |= CREAD;   // enable receiver
	options.c_oflag = 0;
	options.c_lflag = 0;  // ICANON;
	options.c_cc[VMIN] = 24;
	options.c_cc[VTIME] = 1;

	tcflush(mComHandle, TCIFLUSH);

	tcsetattr(mComHandle, TCSANOW, &options);

	if (-1 == flock(mComHandle, LOCK_EX | LOCK_NB)) {
		Close();
		return false;
	}

	mIsOpened = true;

	mRxThreadExitFlag = false;
	mRxThread = new std::thread(mRxThreadProc, this);
	
	return true;
}

bool CmdInterfaceLinux::Close()
{
	if (mComHandle != -1) {
		close(mComHandle);
		mComHandle = -1;
	}

	if (mRxThread->joinable())
		mRxThread->join();

	delete mRxThread;
	mRxThread = NULL;
	return true;
}

bool CmdInterfaceLinux::GetUvcRelatedCmdPort(string & uvc_port_name, string & cmd_port_name)
{
#ifdef USE_UDEV
	struct udev *udev;
	struct udev_enumerate *enumerate;
	struct udev_list_entry *devices, *dev_list_entry;
	struct udev_device *dev;
	udev = udev_new();
	if (!udev) {
		printf("Can't create udev\n");
		exit(1);
	}
	string device_name = uvc_port_name.substr(uvc_port_name.rfind("__") + 2);
	enumerate = udev_enumerate_new(udev);
	udev_enumerate_add_match_subsystem(enumerate, "tty");
	udev_enumerate_scan_devices(enumerate);
	devices = udev_enumerate_get_list_entry(enumerate);
	udev_list_entry_foreach(dev_list_entry, devices) {
		const char *path;
		path = udev_list_entry_get_name(dev_list_entry);
		dev = udev_device_new_from_syspath(udev, path);
		string dev_path = string(udev_device_get_devnode(dev));
		dev = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");
		if(dev){
			if(strstr(udev_device_get_sysattr_value(dev,"product"), device_name.c_str())){
				cmd_port_name = dev_path;
				break;
			}
			udev_device_unref(dev);
		}else
			break;	
	}
	udev_enumerate_unref(enumerate);
	udev_unref(udev);
	return true;
#else
	return false;
#endif
}

bool CmdInterfaceLinux::ReadFromIO(uint8_t * rx_buf, uint32_t rx_buf_len, uint32_t * rx_len)
{
	int32_t len = -1;
	if (IsOpened()) {
		len = read(mComHandle, rx_buf, rx_buf_len);
		if(len != -1 && rx_len)
			*rx_len = len;
	}	
	return len == -1 ? false : true;
}

bool CmdInterfaceLinux::WriteToIo(const uint8_t * tx_buf, uint32_t tx_buf_len, uint32_t * tx_len)
{
	int32_t len = -1;
	if (IsOpened()) {
		len = write(mComHandle, tx_buf, tx_buf_len);
		if(len != -1 && tx_len)
			*tx_len = len;
	}
	return len == -1 ? false : true;
}
