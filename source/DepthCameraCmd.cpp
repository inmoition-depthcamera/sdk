#include "DepthCameraCmd.h"
#include "Common.h"

DepthCameraCmd::DepthCameraCmd()
{
	IsStartFlag = 0;
	IsBusyFlag = 0;
	DepthCameraCmdCallBack = NULL;
	RxBufferPtr = NULL;
}

DepthCameraCmd::~DepthCameraCmd()
{
	IsStartFlag = 0;
	IsBusyFlag = 0;
	DepthCameraCmdCallBack = NULL;
}

int DepthCameraCmd::startCmd(char* DepthCameraName)
{
	int Res = getVideoCorrespondSerialPortName(DepthCameraName, CmdPortName);
	if (Res != 0)
		return Res;
	else
	{
		if (0 == DepthCameraCmdSpi.Open(CmdPortName, 108, 115200))
		{
			DepthCameraCmdSpi.AddRxCallBack(PT_BUFFER, onRxSerailData, this);
			hEventRx = CreateEvent(NULL, 1, 0, NULL);
			IsStartFlag = 1;
			IsBusyFlag = 0;
			IsUpdatingFlag = 0;
			ResetEvent(hEventRx);
		}
		else
			return -2;
	}
	return 0;
}


void DepthCameraCmd::stopCmd()
{
	DepthCameraCmdCallBack = NULL;
	if (DepthCameraCmdSpi.IsOpened())
		DepthCameraCmdSpi.Close();
	IsStartFlag = 0;
	IsBusyFlag = 0;
	CloseHandle(hEventRx);
}

void DepthCameraCmd::onRxSerailData(int32_t id, void *param, const uint8_t *buffer, int32_t len) {

	DepthCameraCmd *Orsd = (DepthCameraCmd*)param;
	
	if (Orsd->RxBufferPtr != NULL)
	{
		int32_t rlen = (Orsd->ReadCmdOffset + len) >= ACKBUFFER_SIZE ? (ACKBUFFER_SIZE - Orsd->ReadCmdOffset) : len;
		memcpy((Orsd->RxBufferPtr + Orsd->ReadCmdOffset), buffer, rlen);
		Orsd->ReadCmdOffset += rlen;
		if (strstr((char*)buffer, " />") != NULL)
		{
			if (Orsd->ReadLen != NULL)
				*Orsd->ReadLen = Orsd->ReadCmdOffset;
			SetEvent(Orsd->hEventRx);
		}
		else if (Orsd->IsUpdatingFlag == 1 && strstr((char*)buffer, ".\r\n") != NULL)
		{
			if (Orsd->ReadLen != NULL)
				*Orsd->ReadLen = Orsd->ReadCmdOffset;
			SetEvent(Orsd->hEventRx);
		}
	}
   if (Orsd->DepthCameraCmdCallBack)
		Orsd->DepthCameraCmdCallBack((uint8_t*)buffer, len);
}

int DepthCameraCmd::writeCmd(const char * read_buffer, int32_t size)
{
	if (IsBusyFlag == 0)
	  return DepthCameraCmdSpi.WriteData(PT_BUFFER, (uint8_t*)read_buffer, size);
	return 1;
}


int DepthCameraCmd::sendOrWaitCmd(const uint8_t *sned_buffer, int32_t sned_len, int32_t timeout, uint8_t *ack_buffer,int32_t * ack_Len) {

	if (IsStartFlag == 0)
		return -1;
	IsBusyFlag = 1;
	if (ack_buffer != NULL){
		RxBufferPtr = ack_buffer;
		ReadLen = ack_Len;
		ReadCmdOffset = 0;
	}
	if (sned_buffer != NULL)DepthCameraCmdSpi.WriteData(PT_BUFFER, (uint8_t*)sned_buffer, sned_len);

	if (ack_buffer != NULL){
		WaitForSingleObject(hEventRx, timeout);
		ResetEvent(hEventRx);
		RxBufferPtr = NULL;
		ReadLen = NULL;
	}
	IsBusyFlag = 0;
	return 0;
}



int DepthCameraCmd::setIntegrationTime(uint8_t value)
{
	if (IsStartFlag == 0)return -2;
	if (IsBusyFlag == 1)return 1;
	value = value > 55 ? 55 : value;
	int len = sprintf_s((char*)CmdBuffer, sizeof(CmdBuffer), "intg %d\r\n", value);
	int ack_len = 0;
	if (sendOrWaitCmd(CmdBuffer, len, 1000, AckBuffer, &ack_len) == 0 && ack_len > 0){
		if (strstr((char*)AckBuffer, "Ok") != NULL)
			return 0;
	}
	return -1;
}


int DepthCameraCmd::setExLight(uint8_t value)
{
	if (IsStartFlag == 0)return -2;
	if (IsBusyFlag == 1)return 1;
	int len = sprintf_s((char*)CmdBuffer, sizeof(CmdBuffer), "isl cd 0x%x\r\n", value);
	int ack_len = 0;
	if (sendOrWaitCmd(CmdBuffer, len, 1000, AckBuffer, &ack_len) == 0 && ack_len > 0) {
		if (strstr((char*)AckBuffer, "Ok") != NULL)
			return 0;
	}
	return -1;
}

int DepthCameraCmd::setInLight(uint8_t value)
{
	if (IsStartFlag == 0)return -2;
	if (IsBusyFlag == 1)return 1;
	value = value > 230 ? 230 : value;
	int len = sprintf_s((char*)CmdBuffer, sizeof(CmdBuffer), "inled %d\r\n", value);
	int ack_len = 0;
	if (sendOrWaitCmd(CmdBuffer, len, 1000, AckBuffer, &ack_len) == 0 && ack_len > 0) {
		if (strstr((char*)AckBuffer, "Ok") != NULL)
			return 0;
	}
	return -1;
}


int DepthCameraCmd::setFrameRate(uint16_t value)
{
	if (IsStartFlag == 0)return -2;
	if (IsBusyFlag == 1)return 1;
	value = value > 500 ? 500 : value;
	int len = sprintf_s((char*)CmdBuffer, sizeof(CmdBuffer), "fps %d\r\n", value);
	int ack_len = 0;
	if (sendOrWaitCmd(CmdBuffer, len, 1000, AckBuffer, &ack_len) == 0 && ack_len > 0) {
		if (strstr((char*)AckBuffer, "Ok") != NULL)
			return 0;
	}
	return -1;
}

int DepthCameraCmd::setMotorSpeed(uint8_t value)
{
	if (IsStartFlag == 0)return -2;
	if (IsBusyFlag == 1)return 1;
	value = value > 500 ? 500 : value;
	int len = sprintf_s((char*)CmdBuffer, sizeof(CmdBuffer), "rev %d\r\n", value);
	int ack_len = 0;
	if (sendOrWaitCmd(CmdBuffer, len, 1000, AckBuffer, &ack_len) == 0 && ack_len > 0) {
		if (strstr((char*)AckBuffer, "Ok") != NULL)
			return 0;
	}
	return -1;
}

int DepthCameraCmd::switchMirror()
{
	if (IsStartFlag == 0)return -2;
	if (IsBusyFlag == 1)return 1;
	int ack_len = 0;
	if (sendOrWaitCmd((uint8_t *)"mirror\r\n", 7, 1000, AckBuffer, &ack_len) == 0 && ack_len > 0) {
		if (strstr((char*)AckBuffer, "Ok") != NULL)
			return 0;
	}
	return -1;
}


int DepthCameraCmd::setBinning(uint8_t rows, uint8_t columns)
{
	if (IsStartFlag == 0)return -2;
	if (IsBusyFlag == 1)return 1;
	rows = rows > 3 ? 3 : rows;
	columns = columns > 3 ? 3 : columns;
	int len = sprintf_s((char*)CmdBuffer, sizeof(CmdBuffer), "binning %d %d\r\n", rows, columns);
	int ack_len = 0;
	if (sendOrWaitCmd(CmdBuffer, len, 1000, AckBuffer, &ack_len) == 0 && ack_len > 0) {
		if (strstr((char*)AckBuffer, "Ok") != NULL)
			return 0;
	}
	return -1;
}

int DepthCameraCmd::restoreFactorySettings()
{
	if (IsStartFlag == 0)return -2;
	if (IsBusyFlag == 1)return 1;
	int ack_len = 0;
    if (sendOrWaitCmd((uint8_t *)"rfs\r\n", 5, 1000, AckBuffer, &ack_len) == 0 && ack_len > 0) {
		if (strstr((char*)AckBuffer, "Ok") != NULL)
			return 0;
	}
	return -1;
}

int DepthCameraCmd::saveConfig()
{
	if (IsStartFlag == 0)return -2;
	if (IsBusyFlag == 1)return 1;
	int ack_len = 0;
    if (sendOrWaitCmd((uint8_t *)"save\r\n", 6, 1000, AckBuffer, &ack_len) == 0 && ack_len > 0) {
		if (strstr((char*)AckBuffer, "save") != NULL)
			return 0;
	}
	return -1;
}

int DepthCameraCmd::getSystemStatus(char * read_buffer, int32_t buffer_size, int32_t * read_len)
{
	if (IsStartFlag == 0)return -2;
	if (IsBusyFlag == 1)return 1;
	int ack_len = 0;
	if (sendOrWaitCmd((uint8_t *)"show\r\n", 6, 1000, AckBuffer, &ack_len) == 0 && ack_len > 0) {
		int len = 0;
		char * str = strstr((char*)AckBuffer, "show");
		char * endstr = strstr((char*)AckBuffer, "\r\nINMOTION");
		len = (int)(endstr - str) - 6;
		if (str != NULL && len > 0)
		{
			if (read_buffer == NULL)return -3;
			if (buffer_size < len)
				len = buffer_size -1;
			*read_len = len;
			memcpy(read_buffer, str + 6, len );
			read_buffer[len] = '\0';
			return 0;
		}
	}
	return -1;
}


int DepthCameraCmd::getCameraConfig(char * Config_buffer)
{
	if (IsStartFlag == 0)return -2;
	if (IsBusyFlag == 1)return 1;
	int ack_len = 0;
	if (sendOrWaitCmd((uint8_t *)"camera\r\n", 8, 10000, AckBuffer, &ack_len) == 0 && ack_len > 0) {
		int len = 0;
		char * str = strstr((char*)AckBuffer, "Send start:\r\n<");
		char * endstr = strstr((char*)AckBuffer, ">\r\nSend end.");
		len = (int)(endstr - str) - 14;
		if (str != NULL && len > 0)
		{
			if (Config_buffer == NULL)return 0;
			memcpy(Config_buffer, str + 14, len);
			Config_buffer[len] = '\0';
			return len;
		}
	}
	return 0;
}



int DepthCameraCmd::getVideoCorrespondSerialPortName(char* device_name, char* serialPort_name) {
	HRESULT hres;
	// Step 1: --------------------------------------------------
	// Initialize COM. ------------------------------------------
#ifdef VI_COM_MULTI_THREADED
	hres = CoInitializeEx(NULL, COINIT_MULTITHREADED);
#else
	hres = CoInitialize(NULL);
#endif
	if (FAILED(hres))
	{
#ifdef DEBUG
		printf("Failed to initialize COM library. Error code = 0x%X\r\n", hres);
#endif // DEBUG
		return 1;                  // Program has failed.
	}
	//编写基于DLL的COM应用程序时，不能调用CoInitializeSecurity,因为代理会调用CoInitializeSecurity.
	// Step 2: --------------------------------------------------
	// Set general COM security levels --------------------------
#ifdef DEBUG

	hres = CoInitializeSecurity(
		NULL,
		-1,                          // COM authentication
		NULL,                        // Authentication services
		NULL,                        // Reserved
		RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
		RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
		NULL,                        // Authentication info
		EOAC_NONE,                   // Additional capabilities 
		NULL                         // Reserved
		);
	if (FAILED(hres))
	{
		printf("Failed to initialize security. Error code = 0x%X\r\n", hres);
		CoUninitialize();
		return 2;                    // Program has failed.
	}
#endif // DEBUG
	// Step 3: ---------------------------------------------------
	// Obtain the initial locator to WMI -------------------------

	IWbemLocator *pLoc = NULL;

	hres = CoCreateInstance(
		CLSID_WbemLocator,
		0,
		CLSCTX_INPROC_SERVER,
		IID_IWbemLocator, (LPVOID *)&pLoc);

	if (FAILED(hres))
	{
#ifdef DEBUG
		printf("Failed to create IWbemLocator object. Error code = 0x%X\r\n", hres);
#endif // DEBUG
		CoUninitialize();
		return 3;                 // Program has failed.
	}

	// Step 4: -----------------------------------------------------
	// Connect to WMI through the IWbemLocator::ConnectServer method

	IWbemServices *pSvc = NULL;

	// Connect to the root\cimv2 namespace with
	// the current user and obtain pointer pSvc
	// to make IWbemServices calls.
	hres = pLoc->ConnectServer(
		_bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
		NULL,                    // User name. NULL = current user
		NULL,                    // User password. NULL = current
		0,                       // Locale. NULL indicates current
		NULL,                    // Security flags.
		0,                       // Authority (for example, Kerberos)
		0,                       // Context object 
		&pSvc                    // pointer to IWbemServices proxy
		);

	if (FAILED(hres))
	{
#ifdef DEBUG
		printf("Could not connect. Error code = 0x%X\r\n", hres);
#endif // DEBUG
		pLoc->Release();
		CoUninitialize();
		return 4;                // Program has failed.
	}
#ifdef DEBUG
	printf("Connected to ROOT\\CIMV2 WMI namespace.\r\n");
#endif // DEBUG
	// Step 5: --------------------------------------------------
	// Set security levels on the proxy -------------------------

	hres = CoSetProxyBlanket(
		pSvc,                        // Indicates the proxy to set
		RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
		RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
		NULL,                        // Server principal name 
		RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
		RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
		NULL,                        // client identity
		EOAC_NONE                    // proxy capabilities 
		);

	if (FAILED(hres))
	{
#ifdef DEBUG
		printf("Could not set proxy blanket.Error code = 0x%X\r\n", hres);
#endif // DEBUG
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return 6;               // Program has failed.
	}

	// Step 6: --------------------------------------------------
	// Use the IWbemServices pointer to make requests of WMI ----

	// For example, get the name of the operating system
	IEnumWbemClassObject* pEnumerator = NULL;
	hres = pSvc->ExecQuery(
		bstr_t("WQL"),
		bstr_t("select * from Win32_PnPEntity where PNPDeviceID  like '%VID_0483&PID_5760&%' "),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		NULL,
		&pEnumerator);

	if (FAILED(hres))
	{
#ifdef DEBUG
		printf("Query for operating system name failed.Error code = 0x%X\r\n", hres);
#endif // DEBUG
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return 6;               // Program has failed.
	}

	// Step 7: -------------------------------------------------
	// Get the data from the query in step 6 -------------------

	IWbemClassObject *pclsObj = NULL;
	ULONG uReturn = 0;
	bool nameOkFlag = 0;
	char * CpnpStr = NULL;
	char * VpnpStr = NULL;
	char * VnameStr = NULL;
	char * CnameStr = NULL;
	char * nameStr = NULL;
	char * pnpStr = NULL;

	while (pEnumerator)
	{
		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
			&pclsObj, &uReturn);

		if (0 == uReturn)
		{
			nameOkFlag = 0;
			break;
		}

		VARIANT vtProp;
		VARIANT pnpProp;
		// Get the value of the Name property
		hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
		hr = pclsObj->Get(L"PNPDeviceID", 0, &pnpProp, 0, 0);
		nameStr = _com_util::ConvertBSTRToString(vtProp.bstrVal);
		pnpStr = _com_util::ConvertBSTRToString(pnpProp.bstrVal);
		if (strstr(nameStr, "COM") != NULL)  //看是COM还是其他
		{
			CnameStr = nameStr;
			CpnpStr = strrchr(pnpStr, '\\');
		}
		else
		{
			VnameStr = nameStr;
			VpnpStr = strrchr(pnpStr, '\\');
		}
		if (nameOkFlag == 0 && VnameStr != NULL)
		{
			if (strstr(VnameStr, device_name) != NULL)
				nameOkFlag = 1;
		}
		if (nameOkFlag)
		{
			if (CpnpStr != NULL && memcmp(CpnpStr, VpnpStr, 10) == 0)
			{ 

				char * comName0 = strrchr(CnameStr, '(');
				char * comName1 = strrchr(CnameStr, ')');
				int size = (int)(comName1 - comName0 - 1);
				memcpy(serialPort_name, "\\\\.\\", 4);
				memcpy(serialPort_name + 4, (char*)(comName0 + 1), size);
				serialPort_name[4 + size] = '\0';
#ifdef DEBUG
				printf("\r\n%s --> %s\r\n", device_name, serialPort_name + 4);
#endif // DEBUG
				break;
			}
		}

		VariantClear(&vtProp);
		VariantClear(&pnpProp);
		pclsObj->Release();
	}

	// Cleanup
	// ========
	pSvc->Release();
	pLoc->Release();
	pEnumerator->Release();
	CoUninitialize();
	if (nameOkFlag)
		return 0;   // Program successfully completed.
	else
		return 7;
}