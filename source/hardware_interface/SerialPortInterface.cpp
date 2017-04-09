
#include "SerialPortInterface.h"

SerialPortInterface::SerialPortInterface()
{
	mComHandle = INVALID_HANDLE_VALUE;
	memset(&mOverlappedSend, 0, sizeof(OVERLAPPED));
	memset(&mOverlappedRecv, 0, sizeof(OVERLAPPED));
	mOverlappedSend.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	mOverlappedRecv.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}


SerialPortInterface::~SerialPortInterface()
{
	if (mOverlappedRecv.hEvent != INVALID_HANDLE_VALUE)
	{
		CloseHandle(mOverlappedRecv.hEvent);
		mOverlappedRecv.hEvent = INVALID_HANDLE_VALUE;
	}
	if (mOverlappedSend.hEvent != INVALID_HANDLE_VALUE)
	{
		CloseHandle(mOverlappedSend.hEvent);
		mOverlappedSend.hEvent = INVALID_HANDLE_VALUE;
	}
}

#ifdef _MSC_VER
int SerialPortInterface::Open(char *file_name, uint32_t param1, uint32_t param2)
{
	uint32_t port_no = param1;
	uint32_t baud_rate = param2;

	strcpy_s(mFileName, file_name);

#ifdef UNICODE
	int needlen = MultiByteToWideChar(936, 0, file_name, -1, 0, 0);
	TCHAR *file_name_t = new TCHAR[needlen];
	if (MultiByteToWideChar(936, 0, file_name, -1, file_name_t, needlen) != needlen)
	{
		delete[]file_name_t;
		return -1;
	}	
#else
	TCHAR *file_name_t = file_name;
#endif

	if (mIsOpened)
	{
		if ((port_no == mPortNo) && (baud_rate == mDCB.BaudRate))
			return 0;
		else
			Close();
	}

	mPortNo = port_no;
	mBaud = baud_rate;
	mComHandle = ::CreateFile(file_name_t,
		GENERIC_READ | GENERIC_WRITE, 0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
		NULL);

#ifdef UNICODE
	delete[] file_name_t;
#endif

	if (mComHandle == INVALID_HANDLE_VALUE)
	{
		CloseHandle(mComHandle);
		return -1;
	}

	PurgeComm(mComHandle, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
	if (SetupComm(mComHandle, 4096, 4096) == 0)
	{
		CloseHandle(mComHandle);
		return -1;
	}
	mDCB.DCBlength = sizeof(DCB);
	if (GetCommState(mComHandle, &mDCB) == 0)
	{
		CloseHandle(mComHandle);
		return -1;
	}
	else
	{
		mDCB.BaudRate = baud_rate;
		mDCB.ByteSize = 8;
		mDCB.Parity = NOPARITY;
		mDCB.StopBits = ONESTOPBIT;
		if (SetCommState(mComHandle, &mDCB) == 0)
		{
			CloseHandle(mComHandle);
			return -1;
		}
	}

	COMMTIMEOUTS TimeOuts;

	TimeOuts.ReadIntervalTimeout = 1;
	TimeOuts.ReadTotalTimeoutMultiplier = 1;
	TimeOuts.ReadTotalTimeoutConstant = 1;

	TimeOuts.WriteTotalTimeoutMultiplier = 1000;
	TimeOuts.WriteTotalTimeoutConstant = 50000;
	SetCommTimeouts(mComHandle, &TimeOuts);
	mIsOpened = 1;

	mRxThread.Start();
	return 0;
}

int SerialPortInterface::Close()
{
	mRxThread.Exit();

	if (mComHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(mComHandle);		
		mComHandle = INVALID_HANDLE_VALUE;
	}
	mIsOpened = 0;
	return TRUE;
}

int32_t SerialPortInterface::ReadBufferFromInterface(uint8_t * byBuf, int32_t dwLen, int32_t dwTimeOut)
{
	uint32_t readed = 0;
	uint32_t res;

	ResetEvent(mOverlappedRecv.hEvent);

	res = ReadFile(mComHandle, byBuf, dwLen, (LPDWORD)&readed, &mOverlappedRecv);
	if (res == FALSE)
	{
		mSysErrCode = GetLastError();
		if (mSysErrCode == ERROR_IO_PENDING)
		{
			WaitForSingleObject(mOverlappedRecv.hEvent, INFINITE);
			GetOverlappedResult(mComHandle, &mOverlappedRecv, (LPDWORD)&readed, FALSE);
		}
	}

	return readed;
}

int32_t SerialPortInterface::WriteBufferToInterface(const uint8_t * byBuf, int32_t dwLen, int32_t dwTimeOut)
{
	uint32_t writed = 0;
	ResetEvent(mOverlappedSend.hEvent);
	uint32_t res = WriteFile(mComHandle, byBuf, dwLen, (LPDWORD)&writed, &mOverlappedSend);
	if (!res)
	{
		mSysErrCode = GetLastError();
		if (mSysErrCode == ERROR_IO_PENDING)
		{
			WaitForSingleObject(mOverlappedSend.hEvent, dwTimeOut);
			GetOverlappedResult(mComHandle, &mOverlappedSend, (LPDWORD)&writed, FALSE);
			return writed;
		}
		else
			return 0;
	}
	return writed;
}

#else

bool CRawSerial::Open(void * dwParams)
{
	uint32_t *params = (uint32_t *)dwParams;
	uint32_t port_no = params[0];
	uint32_t baud_rate = params[1];
	
	if (mIsOpened)
	{
		if ((port_no == mPortNo) && (baud_rate == mBaud))
			return 1;
		else
			Close();
	}

	mPortNo = port_no;
	mBaud = baud_rate;

	struct termios newtio, oldtio;

	mComHandle = open(com_name_, O_RDWR | O_NOCTTY | O_NDELAY);
	
	if (mComHandle == -1)
		return 0;

	if (fcntl(mComHandle, F_SETFL, 0)<0)
		return 0;

	if (isatty(STDIN_FILENO) == 0)
		return 0;
	if (tcgetattr(mComHandle, &oldtio) != 0) {
		return 0;
	}

	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag |= CLOCAL | CREAD;   //enable read and write
	newtio.c_cflag &= ~CSIZE;
	newtio.c_cflag |= CS8;          //8 data bit
	newtio.c_cflag &= ~PARENB;      //disable parity
	newtio.c_cflag &= ~CSTOPB;      //1 stop bit

	cfsetispeed(&newtio, mComHandle);
	cfsetospeed(&newtio, mComHandle);

	newtio.c_cc[VTIME] = 1;     //while reading data count less than VMIN, after 100ms can be read
	newtio.c_cc[VMIN] = 1;
	tcflush(mComHandle, TCIFLUSH);
	
	if ((tcsetattr(mComHandle, TCSANOW, &newtio)) != 0)
		return 0;

	// play with DTR
	int iFlags;
	// turn on DTR
	iFlags = TIOCM_DTR;
	ioctl(mComHandle, TIOCMBIS, &iFlags);

	//step3: create receive thread
	int ret = pthread_create(&rx_thread_id_, NULL, ReceiveThread, (void*)this);
	if (ret != 0) {
		return 0;
	}
	m_bIsOpened = 1;
	return 1;
}

bool CRawSerial::Close()
{
	void* status;

	mRxThread.Exit();
	//close serial communication
	if (mComHandle != 0)
	{
		close(mComHandle);
		mComHandle = 0;
	}

	mIsOpened = 0;

	return 1;
}

#endif

int SerialPortInterface::ReOpen()
{
	Close();
	return Open(mFileName, mPortNo, mBaud);
}