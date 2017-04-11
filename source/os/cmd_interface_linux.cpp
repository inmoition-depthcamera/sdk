
#include "cmd_interface_linux.h"

#include <sys/file.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <string.h>
#include <memory.h>

CmdInterfaceLinux::CmdInterfaceLinux()
{
}

CmdInterfaceLinux::~CmdInterfaceLinux()
{
}

bool CmdInterfaceLinux::Open(string & port_name)
{
	int flags = (O_RDWR | O_NOCTTY | O_NONBLOCK | O_CLOEXEC | O_SYNC);
	mComHandle = open(port_name.c_str(), flags);
	if (-1 == mComHandle) {
		return -1;
	}

	if (-1 == fcntl(mComHandle, F_SETFD, FD_CLOEXEC)) {
		Close();
		return -1;
	}

	// get port options
	struct termios options;
	if (-1 == tcgetattr(mComHandle, &options)) {
		Close();
		return -1;
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
	return false;
}

bool CmdInterfaceLinux::ReadFromIO(uint8_t * rx_buf, uint32_t rx_buf_len, uint32_t * rx_len)
{
	if (IsOpened()) {
		int32_t len = read(mComHandle, rx_buf, rx_buf_len);
		if (rx_len)
			*rx_len = len;
	}	
	return true;
}

bool CmdInterfaceLinux::WriteToIo(const uint8_t * tx_buf, uint32_t tx_buf_len, uint32_t * tx_len)
{
	if (IsOpened()) {
		int32_t len = write(mComHandle, tx_buf, tx_buf_len);
		if (tx_len)
			*tx_len = len;
	}
	return false;
}
