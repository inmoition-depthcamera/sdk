#include "cmd_interface_win.h"

#include <comdef.h>
#include <Wbemidl.h>
#include "comutil.h"
#pragma comment(lib, "comsupp.lib")
#pragma comment(lib, "wbemuuid.lib")

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
	if (SetupComm(mComHandle, 4096, 4096) == 0) {
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
		dcb.fRtsControl = 1; // use rts control to notify usb serial port open and close event
		if (SetCommState(mComHandle, &dcb) == 0) {
			CloseHandle(mComHandle);
			return false;
		}
	}

	COMMTIMEOUTS timeouts = { 1, 1, 1, 1000, 50000 };
	SetCommTimeouts(mComHandle, &timeouts);

	mIsOpened = true;

	mRxThreadExitFlag = false;
	mRxThread = new std::thread(mRxThreadProc, this);

	
	return true;
}

bool CmdInterfaceWin::Close()
{
	mRxThreadExitFlag = true;
	mIsOpened = false;

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

bool CmdInterfaceWin::GetUvcRelatedCmdPort(string & uvc_port_name, string & cmd_port_name)
{
	HRESULT hres;
#ifdef VI_COM_MULTI_THREADED
	hres = CoInitializeEx(NULL, COINIT_MULTITHREADED);
#else
	hres = CoInitialize(NULL);
#endif
	if (FAILED(hres))
		return false;

	IWbemLocator *pLoc = NULL;
	hres = CoCreateInstance(
		CLSID_WbemLocator,
		0,
		CLSCTX_INPROC_SERVER,
		IID_IWbemLocator, (LPVOID *)&pLoc);

	if (FAILED(hres)) {
		CoUninitialize();
		return false;
	}

	IWbemServices *pSvc = NULL;

	hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc );

	if (FAILED(hres)) {
		pLoc->Release();
		CoUninitialize();
		return false;                // Program has failed.
	}

	hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, 
		RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

	if (FAILED(hres)) {
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return false;
	}

	IEnumWbemClassObject* pEnumerator = NULL;
	hres = pSvc->ExecQuery(bstr_t("WQL"),
		bstr_t("select * from Win32_PnPEntity where PNPDeviceID  like '%VID_0483&PID_5760&%' "),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);

	if (FAILED(hres)) {
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return false;
	}

	IWbemClassObject *pclsObj = NULL;
	ULONG uReturn = 0;
	char * nameStr = NULL;
	char * pnpStr = NULL;

	string video_pnp;
	vector<pair<string, string>> cmd_info;
	const char *video_name = strstr(uvc_port_name.c_str(), "__");
	if (video_name) {
		video_name += 2;
		while (pEnumerator) {
			HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
				&pclsObj, &uReturn);

			if (0 == uReturn)
				break;

			VARIANT vtProp;
			VARIANT pnpProp;

			hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
			hr = pclsObj->Get(L"PNPDeviceID", 0, &pnpProp, 0, 0);
			nameStr = _com_util::ConvertBSTRToString(vtProp.bstrVal);
			pnpStr = _com_util::ConvertBSTRToString(pnpProp.bstrVal);

			if (strstr(nameStr, "COM") != NULL) {
				char *p1 = strrchr(pnpStr, '\\');
				char *p2 = strrchr(p1, '&');

				char * c1 = strrchr(nameStr, '(');
				char * c2 = strrchr(c1, ')');
				*c2 = 0; *p2 = 0;
				pair<string, string> str_pair(c1 + 1, p1);
				cmd_info.push_back(str_pair);
			}
			else {
				if (strcmp(nameStr, video_name) == 0) {
					char *start = strrchr(pnpStr, '\\');
					char *end = strrchr(start, '&');
					*end = 0;
					video_pnp = start;
				}
			}

			delete[] nameStr;
			delete[] pnpStr;

			VariantClear(&vtProp);
			VariantClear(&pnpProp);
			pclsObj->Release();
		}
	}

	pSvc->Release();
	pLoc->Release();
	pEnumerator->Release();
	CoUninitialize();

	for (auto &str_pair : cmd_info) {
		if (str_pair.second == video_pnp) {
			cmd_port_name = "\\\\.\\" + str_pair.first;
			return true;
		}
	}
	return false;
}