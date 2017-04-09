
#include "DepthCameraInput.h"
#include "Common.h"
#include "stdio.h"


/********************* Capturing video from camera via DirectShow *********************/



int main()
{
	int num = 0;
	char * Name = NULL;
	DepthCameraInput DCI;
	unsigned char Buffer[4096];
	unsigned short PhaseBuffer[80 * 60];
	unsigned short PhaseBuffer1[80 * 60];
	unsigned short ABuffer[80 * 60];
	unsigned char FBuffer[80 * 60];

    float PointCloudBuffer[80 * 60 * 3];
	int32_t Len = 0;
	int size = 0;
	num = DCI.listVideos();
	if (num > 0)
	{
		if (DCI.OpenById(0,true))
		{
			if(DCI.isCmdStart() == 1)
			{
     			printf("--------------------------------\r\n");

				DCI.getSystemStatus((char*)Buffer, sizeof(Buffer), &Len);
				Buffer[Len] = 0;
				printf("GetSystemStatus :\r\n %s\r\n%d\r\n", Buffer, Len);
				Sleep(100);
				printf("SetExLight: %d\r\n", DCI.setExLight(222));
				Sleep(100);
				printf("SetBinning: %d\r\n", DCI.setBinning(1, 1));
				Sleep(100);
				printf("SetInLight: %d\r\n", DCI.setInLight(200));
				Sleep(100);
				printf("SetFrameRate: %d\r\n", DCI.setFrameRate(200));
				Sleep(100);
				printf("SetIntegrationTime: %d\r\n", DCI.setIntegrationTime(50));
				Sleep(100);
				printf("SaveConfig: %d\r\n", DCI.saveConfig());
				Sleep(100);
				printf("SwitchMirror: %d\r\n", DCI.switchMirror());
				Sleep(100);
				
				//DCI.updateFirmwareStart(0,"e:\\test.ifw");
				Sleep(500);
				if (DCI.isUpdating())
				{
					//DCI.updateFirmwareStop(0);

				}
			}
			while (1)
			{
				Sleep(30);
				DCI.getPixels(0,NULL ,PhaseBuffer, ABuffer, NULL, FBuffer);
				DCI.PhaseDenoise(PhaseBuffer, ABuffer, FBuffer, PhaseBuffer1,10);
				DCI.DepthToPointCloud(0,PhaseBuffer1, PointCloudBuffer);
				printf("fps: %d\r\n", DCI.getCurrentFrameRate(0));
			}
		}
	}
	else
		printf("No device\r\n");
	DCI.Close(0);
	return 0;
}
