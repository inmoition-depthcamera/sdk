
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
	mComHandle = -1;
}

CmdInterfaceLinux::~CmdInterfaceLinux()
{
	Close();
}

bool CmdInterfaceLinux::Open(string & port_name)
{
	int flags = (O_RDWR | O_NOCTTY | O_NONBLOCK);
	mComHandle = open(port_name.c_str(), flags);
	if (-1 == mComHandle) {
		return false;
	}

	// get port options
	struct termios options;
	if (-1 == tcgetattr(mComHandle, &options)) {
		Close();
		return false;
	}

	options.c_cflag |= (tcflag_t)  (CLOCAL | CREAD | CS8 | CRTSCTS);
	options.c_cflag &= (tcflag_t) ~(CSTOPB | PARENB | PARODD);
    options.c_lflag &= (tcflag_t) ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL |
                                       ISIG | IEXTEN); //|ECHOPRT
	options.c_oflag &= (tcflag_t) ~(OPOST);
	options.c_iflag &= (tcflag_t) ~(IXON | IXOFF | INLCR | IGNCR | ICRNL | IGNBRK);

    options.c_cc[VMIN] = 0;
    options.c_cc[VTIME] = 0;

    if(tcsetattr(mComHandle, TCSANOW, &options) < 0){
        Close();
        return false;
    }

	tcflush(mComHandle, TCIFLUSH);

	mRxThreadExitFlag = false;
	mRxThread = new std::thread(mRxThreadProc, this);
	mIsCmdOpened = true;

	return true;
}

bool CmdInterfaceLinux::Close()
{
	if(mIsCmdOpened == false)
		return true;

	mRxThreadExitFlag = true;

    if (mComHandle != -1) {
        close(mComHandle);
        mComHandle = -1;
    }

	if (mRxThread && mRxThread->joinable()) {
		mRxThread->join();
		delete mRxThread;
		mRxThread = NULL;
	}

	mIsCmdOpened = false;

	return true;
}

bool CmdInterfaceLinux::GetCmdDevices(std::vector<std::pair<std::string, std::string>> &device_list)
{
#ifdef USE_UDEV
	struct udev *udev;
	struct udev_enumerate *enumerate;
	struct udev_list_entry *devices, *dev_list_entry;
	struct udev_device *dev;
	udev = udev_new();
	if (!udev) {
		return false;
	}
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
			std::pair<std::string, std::string> p;
			p.first = dev_path;
			p.second = udev_device_get_sysattr_value(dev,"product");
			device_list.push_back(p);
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

bool CmdInterfaceLinux::GetUvcRelatedCmdPort(string & uvc_port_name, string & cmd_port_name)
{
#ifdef USE_UDEV
	string device_name = uvc_port_name.substr(uvc_port_name.rfind("__") + 2);
	std::vector<std::pair<std::string, std::string>> device_list;
	GetCmdDevices(device_list);
	for(auto dev : device_list){
		if(strstr(dev.second.c_str(), device_name.c_str())){
			cmd_port_name = dev.first;
			return true;
		}
	}
#endif
	return false;
}

bool CmdInterfaceLinux::ReadFromIO(uint8_t * rx_buf, uint32_t rx_buf_len, uint32_t * rx_len)
{
	static timespec timeout = {0, (long)(100 * 1e6)};
	int32_t len = -1;
	if (IsOpened()) {
        fd_set read_fds;
        FD_ZERO (&read_fds);
        FD_SET (mComHandle, &read_fds);
        int r = pselect (mComHandle + 1, &read_fds, NULL, NULL, &timeout, NULL);
        if (r < 0) {
            // Select was interrupted
            if (errno == EINTR) {
                return false;
            }
        }else if(r == 0){ // timeout
            return false;
        }

		if(FD_ISSET (mComHandle, &read_fds)){
			len = read(mComHandle, rx_buf, rx_buf_len);
			if(len != -1 && rx_len)
				*rx_len = len;
		}
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
