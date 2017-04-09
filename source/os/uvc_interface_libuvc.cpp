#include "cmd_interface_win.h"



CmdInterfaceWin::CmdInterfaceWin()
{
	mComHandle = INVALID_HANDLE_VALUE;
}


CmdInterfaceWin::~CmdInterfaceWin()
{
}

int32_t CmdInterfaceWin::Open(string port_name)
{
	if (IsOpened())
		Close();

	mPortName = port_name;
	mComHandle = ::CreateFile(port_name.c_str(),
		GENERIC_READ | GENERIC_WRITE, 0, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (mComHandle == INVALID_HANDLE_VALUE) {
		CloseHandle(mComHandle);
		return -1;
	}

	PurgeComm(mComHandle, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
	if (SetupComm(mComHandle, 4096, 4096) == 0) {
		CloseHandle(mComHandle);
		return -1;
	}

	DCB dcb;
	dcb.DCBlength = sizeof(DCB);
	if (GetCommState(mComHandle, &dcb) == 0) {
		CloseHandle(mComHandle);
		return -1;
	}
	else {
		dcb.ByteSize = 8;
		dcb.Parity = NOPARITY;
		dcb.StopBits = ONESTOPBIT;
		dcb.fRtsControl = 1; // use rts control to notify usb serial port open and close event
		if (SetCommState(mComHandle, &dcb) == 0) {
			CloseHandle(mComHandle);
			return -1;
		}
	}

	COMMTIMEOUTS timeouts = { 1, 1, 1, 1000, 50000 };
	SetCommTimeouts(mComHandle, &timeouts);

	mRxThreadExitFlag = false;
	mRxThread = std::thread(mRxThreadProc, this);

	mIsOpened = true;
	return 0;
}

int32_t CmdInterfaceWin::Close()
{
	mRxThreadExitFlag = true;
	mIsOpened = false;

	if (mComHandle != INVALID_HANDLE_VALUE) {
		CloseHandle(mComHandle);
		mComHandle = INVALID_HANDLE_VALUE;
	}

	if (mRxThread.joinable())
		mRxThread.join();

	return 0;
}

bool CmdInterfaceWin::ReadFromIO(uint8_t * rx_buf, uint32_t rx_buf_len, uint32_t * rx_len)
{
	if (IsOpened()) {
		BOOL res = ReadFile(mComHandle, rx_buf, rx_buf_len, (LPDWORD)rx_len, NULL);
		return res ? true : false;
	}
	return false;
}

bool CmdInterfaceWin::WriteToIo(const uint8_t * tx_buf, uint32_t tx_buf_len, uint32_t * tx_len)
{
	if (IsOpened()) {
		BOOL res = WriteFile(mComHandle, tx_buf, tx_buf_len, (LPDWORD)tx_len, NULL);
		return res ? true : false;
	}
	return false;
}
