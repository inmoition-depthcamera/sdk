#include "cmd_interface_win.h"

#include <comdef.h>
#include <Wbemidl.h>
#include <comutil.h>
#include <cctype>
#include <algorithm>
#include <SetupAPI.h>
#include <Cfgmgr32.h>
#include "tchar.h"
#pragma comment(lib, "comsupp.lib")
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "SetupAPI.lib")
#pragma comment(lib, "Cfgmgr32.lib")
#include "iostream"
using namespace std;

CmdInterfaceWin::CmdInterfaceWin()
{
	mComHandle = INVALID_HANDLE_VALUE;
	memset(&mOverlappedSend, 0, sizeof(OVERLAPPED));
	memset(&mOverlappedRecv, 0, sizeof(OVERLAPPED));
	mOverlappedSend.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	mOverlappedRecv.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}


CmdInterfaceWin::~CmdInterfaceWin()
{
	if (mOverlappedRecv.hEvent != INVALID_HANDLE_VALUE){
		CloseHandle(mOverlappedRecv.hEvent);
		mOverlappedRecv.hEvent = INVALID_HANDLE_VALUE;
	}
	if (mOverlappedSend.hEvent != INVALID_HANDLE_VALUE){
		CloseHandle(mOverlappedSend.hEvent);
		mOverlappedSend.hEvent = INVALID_HANDLE_VALUE;
	}
}

bool CmdInterfaceWin::Open(string &port_name)
{
	if (IsOpened())
		Close();

	mPortName = port_name;
	mComHandle = ::CreateFile(port_name.c_str(),
		GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,		
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);

	if (mComHandle == INVALID_HANDLE_VALUE) {
		CloseHandle(mComHandle);
		return false;
	}

	PurgeComm(mComHandle, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
	if (SetupComm(mComHandle, 102400, 102400) == 0) {
		CloseHandle(mComHandle);
		return false;
	}

	DCB dcb;
	dcb.DCBlength = sizeof(DCB);
	if (GetCommState(mComHandle, &dcb) == 0) {
		CloseHandle(mComHandle);
		return false;
	}
	else {
		dcb.ByteSize = 8;
		dcb.Parity = NOPARITY;
		dcb.StopBits = ONESTOPBIT;
		dcb.fRtsControl = 0; 
		dcb.fDtrControl = 1;// use dtr control to notify usb serial port open and close event
		if (SetCommState(mComHandle, &dcb) == 0) {
			CloseHandle(mComHandle);
			return false;
		}
	}

	COMMTIMEOUTS timeouts;
	timeouts.ReadIntervalTimeout = 2;
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.ReadTotalTimeoutConstant = 5;
	timeouts.WriteTotalTimeoutConstant = 50000;
	timeouts.WriteTotalTimeoutMultiplier = 1000;
	SetCommTimeouts(mComHandle, &timeouts);

	mIsCmdOpened = true;

	mRxThreadExitFlag = false;
	mRxThread = new std::thread(mRxThreadProc, this);

	
	return true;
}

bool CmdInterfaceWin::Close()
{
	if (!IsOpened())
		return true;

	mRxThreadExitFlag = true;
	mIsCmdOpened = false;

	if (mComHandle != INVALID_HANDLE_VALUE) {
		CloseHandle(mComHandle);
		mComHandle = INVALID_HANDLE_VALUE;
	}

	if (mRxThread->joinable())
		mRxThread->join();

	delete mRxThread;
	mRxThread = NULL;

	return true;
}

bool CmdInterfaceWin::ReadFromIO(uint8_t * rx_buf, uint32_t rx_buf_len, uint32_t * rx_len)
{
	if (IsOpened()) {
		uint32_t res;
		ResetEvent(mOverlappedRecv.hEvent);
		res = ReadFile(mComHandle, rx_buf, rx_buf_len, (LPDWORD)rx_len, &mOverlappedRecv);
		if (res == FALSE){
			if (GetLastError() == ERROR_IO_PENDING){				
				if (WaitForSingleObject(mOverlappedRecv.hEvent, INFINITE) == WAIT_OBJECT_0) {
					GetOverlappedResult(mComHandle, &mOverlappedRecv, (LPDWORD)rx_len, FALSE);
					return true;
				}				
			}
		}
	}
	return false;
}

bool CmdInterfaceWin::WriteToIo(const uint8_t * tx_buf, uint32_t tx_buf_len, uint32_t * tx_len)
{
	if (IsOpened()) {
		ResetEvent(mOverlappedSend.hEvent);
		uint32_t res = WriteFile(mComHandle, tx_buf, tx_buf_len, (LPDWORD)tx_len, &mOverlappedSend);
		if (!res){
			if (GetLastError() == ERROR_IO_PENDING){
				WaitForSingleObject(mOverlappedSend.hEvent, INFINITE);
				GetOverlappedResult(mComHandle, &mOverlappedSend, (LPDWORD)tx_len, FALSE);
			}
		}
	}
	return true;
}

bool CmdInterfaceWin::GetCmdDevices(std::vector<std::pair<std::string, std::string>>& device_list)
{
	const TCHAR * vid_pid = _T("VID_0483&PID_5740");
	DWORD dwGuids = 0;
	TCHAR prop_buf[1024];

	SetupDiClassGuidsFromName(_T("Ports"), NULL, 0, &dwGuids);
	if (dwGuids == 0)
		return false;

	GUID *pGuids = new GUID[dwGuids];
	SetupDiClassGuidsFromName(_T("Ports"), pGuids, dwGuids, &dwGuids);

	for (DWORD i = 0; i < dwGuids; i++) {
		HDEVINFO hDevInfo = SetupDiGetClassDevs(&pGuids[i], NULL, NULL, DIGCF_PRESENT);
		if (hDevInfo == INVALID_HANDLE_VALUE)
			break;
		for (int index = 0; ; index++) {
			SP_DEVINFO_DATA devInfo;
			std::pair<std::string, std::string> p;
			devInfo.cbSize = sizeof(SP_DEVINFO_DATA);
			if (!SetupDiEnumDeviceInfo(hDevInfo, index, &devInfo)) { break; }
			prop_buf[0] = 0;
			CM_Get_Device_ID(devInfo.DevInst, prop_buf, 1024, 0);
			if (_tcsstr(prop_buf, vid_pid)) {
				p.second = prop_buf;
				HKEY hDeviceKey = SetupDiOpenDevRegKey(hDevInfo, &devInfo, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_QUERY_VALUE);
				if (hDeviceKey) {
					prop_buf[0] = 0;
					DWORD dw_size = sizeof(prop_buf);
					DWORD dw_type = 0;
					if ((RegQueryValueEx(hDeviceKey, _T("PortName"), NULL, &dw_type, (LPBYTE)prop_buf, &dw_size) == ERROR_SUCCESS) && (dw_type == REG_SZ)) {
						p.first = prop_buf;
						device_list.push_back(p);
					}
				}
			}
			//SetupDiGetDeviceRegistryProperty(hDevInfo, &devInfo, SPDRP_FRIENDLYNAME, 0L, (PBYTE)prop_buf, 1024, &n);
		}
		SetupDiDestroyDeviceInfoList(hDevInfo);
	}
	delete[]pGuids;

	return true;
}

bool CmdInterfaceWin::GetUvcRelatedCmdPort(string & uvc_port_name, string & cmd_port_name)
{
	std::vector<std::pair<std::string, std::string>> device_list;
	bool ret = GetCmdDevices(device_list);

	const char *video_name = strstr(uvc_port_name.c_str(), "__") + 2;
	if (ret && device_list.size() > 0) {
		for (auto dev : device_list) {
			if (strstr(dev.second.c_str(), video_name)) {
				cmd_port_name = "\\\\.\\" + dev.first;
				return true;
			}
		}
	}

	return false;
}