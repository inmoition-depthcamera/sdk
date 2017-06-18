# Inmotion Depth Camera SDK [(Chinese)](https://github.com/inmoition-depthcamera/sdk/blob/master/README_cn.md)

Linux/Window SDK for Inmotion's Depth Cameras
Inmotion depth camera's external interface is a USB composite device that contains a common UVC camera interface and a USB virtual serial port(**Some low resolution device only has a USB virtual serial port**).
This SDK provides a library interface for accessing those two interfaces to obtaining data, and configuring the depth camera.

## Supported Platform
- Windows (Tested in Windows 10)
- Linux like platform(Tested in Ubuntu 16.04)
- Not supported in Virtual Mathine(VMware or VirtualBox)

## Dependences
### SDK Dependences
#### Windows

The SDK using the Windows API directly and `Direct Show` for interface access and control and does not depend on other libraries

#### Linux

- **V4L** Already included in most Linux distributions.
- **libudev** Used to manage USB device information.
- **cmake** Used to build and compile projects.

### Examples Dependences
- OpenGL
- glfw3

## Buld

### Windows
- Make sure that all dependent libraries are already installed.
- Install Visual Studio 2017.
- Enter the `msvc` directory, open the `inmotion-depth-camera-sdk.sln file`, complete the compilation.

> Note that you need to set the location of the corresponding dependency library when compiling the example program.

### Linux
- Install Dependences

````
sudo apt-get install libglfw3-dev libudev-dev
````
- Get SDK source
````
git clone git@github.com:inmoition-depthcamera/sdk.git
````
- Make SDK
````
cd root_of_sdk_path
mkdir build
cmake ..
make
````
- The target files are under `root_of_sdk_path/bin`.

- Notes：
> The compilation toolchain needs to support `c++ 11`

> The camera has two nodes in /dev which are: /dev/ttyACMx and /dev/videox. Make sure your user has right to access those devices before run the example or sdk. Use following instruction to get rights:
````
sudo usermod -a -G video,dialout $(whoami)
````


## Usage
### Using Cmd and Uvc Port

> For most depth camera devices. Using `DepthCameraCmdVideo` to manage and config device, and `DepthCameraUvcPort` to get frame from device. 

- Contains the corresponding header file `depth_camera_uvc.h` and` depth_camera_cmd.h`.
- Call `DepthCameraUvcPort::GetDepthCameraList` to get the depth cameras connected to the system.
- Call `DepthCameraCmdPort::GetUvcRelatedCmdPort` to get the name of the command port for given depth camera name.
- Call `DepthCameraCmdPort::Open` to open the command port. **Note: The command port needs to be opened first than the UVC port**.
- Call `DepthCameraUvcPort::SetDepthFrameCallback` to set the frame callback function. Or directly call `DepthCameraUvcPort::GetDepthFrame` to get the latest depth of the camera frame after opened the uvc port.
- Call `DepthCameraUvcPort::Open` to open the UVC data port. **Note: The command port needs to be opened first than the UVC port.**
- Call `DepthCameraCmdPort::GetDepthScale` to get the depth data and distance conversion factor.
- Call `DepthCameraUvcPort::DepthToPointCloud` to convert deep data frames to 3D point cloud data.
- Perform user operations.
- Call `DepthCameraUvcPort::Close` to close the UVC data flow interface.
- Call `DepthCameraCmdPort::Close` to close the command interface.

### Using cmd_video Port

> For some low resolution devices. Using `DepthCameraCmdVideo`(inherited from `DepthCameraCmdPort` and `DepthVideoInterface`) to config the depth camera and grabber frame from the depth camera.

- Contains the corresponding header file `depth_camera_cmd_video.h`.
- Call `DepthCameraCmdVideo::GetDepthCameraList` to get the depth cameras connected to the system.
- Call `DepthCameraCmdVideo::Open` to open the cmd video port. 
- Call `DepthCameraCmdVideo::SetDepthFrameCallback` to set the frame callback function. Or directly call `DepthCameraCmdVideo::GetDepthFrame` to get the latest depth of the camera frame after opened the uvc port.
- Call `DepthCameraCmdVideo::GetDepthScale` to get the depth data and distance conversion factor.
- Call `DepthCameraCmdVideo::DepthToPointCloud` to convert deep data frames to 3D point cloud data.
- Perform user operations.
- Call `DepthCameraCmdVideo::Close` to close the cmd video port

## Depth Frame

In the SDK, a depth frame is represented by the following class:

````
class DepthFrame{
public:
	int32_t w;            /// The Width of the depth frame
	int32_t h;            /// The Heigth of the depth frame 
	uint16_t *phase;      /// The Phase of the depth frame (distance = phase * K). Only low 12 bits has been used.
	uint16_t *amplitude;  /// The amplitude of each pixel. Only low 12 bits is used. (Some camera's low 4 bits is zero).
	uint8_t *ambient;     /// The ambient of each pixel. Only low 3 Bits has been used.
	uint8_t *flags;       /// The over explote flag of each pixel. Only low 1 bit has been used.
	bool amplitude_8bits; /// If the amplitude is only high 8bits.

	DepthFrame(int32_t _w, int32_t _h);
	~DepthFrame();

	/// @brief Copy frame data to given frame
	bool CopyTo(DepthFrame *df);
	/// @brief Copy frame data from given frame
	bool CopyFrom(const DepthFrame *df);
	/// @brief Copy frame data from given frame
	DepthFrame & operator=(const DepthFrame &from) { this->CopyFrom(&from); return *this; }
	/// @brief Calculate SUM of given rect from frame
	uint32_t CalcRectSum(int32_t phase_or_amplitude, int32_t x, int32_t y, int32_t _w, int32_t _h);
	/// @brief Calculate SUM of given center rect from frame
	uint32_t CalcCenterRectSum(int32_t phase_or_amplitude, int32_t _w, int32_t _h);
	/// @brief Convent all depth info to a Gray24 buffer
	bool ToGray24(uint8_t *gray24_buf, int32_t gray24_buf_size);
	/// @brief Convent all depth info to a rgb24 buffer
	bool ToRgb24(uint8_t *rgb24_buf, int32_t rgb24_buf_size);
	/// @brief Calculate Histogram of the frame
	template<typename T>
	inline T CalcHistogram(int32_t phase_or_amplitude, T *histogram_buf, int32_t max);
};
````

Each depth frame contains the following parts:
- **phase** Phase information. The conversion relationship between the phase and the distance is: distance = phase * K. K is related to the modulation frequency and can be obtained by the `GetDepthScale` function. Only low 12 bits are valid.
- **amplitude** Confidence level (strength) information. The confidence of each pixel. The higher the confidence, the smaller the noise, the higher the credibility. Only low 12 active. (Some of the camera's amplitude of the lower 4 bits is 0, the actual effective data for the Bit 4 ~ Bit 11).
- **ambient** Ambient light information. Indicates the intensity of the effective band of light in the environment. Only the lower 3 bits is valid.
- **flags** Exposure information. Indicates whether the pixel has been over exposed, and if it is over exposed, the pixel should be considered invalid. Only lower 1 bit is valid.

## Camera configuration

This SDK provides the following interfaces for configuration:

- **SetIntegrationTime** Set the integration time of camera.
- **SetExternIlluminatePower** Set the extern illuminate power.
- **SetInternalIlluminatePower** Set the internal illuminate power.
- **SetFrameRate** Set the frame rate of the camera.
- **SwitchMirror** Mirror the output by 180 degree.
- **SetBinning** Set the Binning of the depth camera.
- **SetHdrRatio** Set the camera's hdr ratio.
- **GetDepthScale** Get the depth data to distance scale.
- **Calibration** Calibrate the device with a given distance.
- **SaveConfig** Save modified settings to internal FLASH.
- **GetSystemStatus** Get current status information of depth camera.
- **RestoreFactorySettings** Restore depth camera setting to factory setting.
- **StartUpgrade** Start upgrade the depth camera's firmware.
- **StopUpgrade** Stop the upgrading process.
- **GetUpgradeProgress** Get upgrade progress.
- **IsUpgrading** Get upgrade status.

> Refer to the comments in the `depth_camera_cmd.h` file for usage of the above interface functions.

## Simple example to grabber frame data from depth camera using cmd and uvc port

````
#include <depth_camera_cmd.h>
#include <depth_camera_uvc.h>
#include <iostream>

using namespace std;
using namespace chrono;

// Frame Callback function
void OnDepthFrame(const DepthFrame *df, void*param){

	static auto last_time = system_clock::now();
	static auto last_avg_time = system_clock::now();
	static int32_t dt_avg_v = 0, total_count = 0;
	auto cur_time = system_clock::now();
	auto dt = duration_cast<milliseconds>(cur_time - last_time);

	total_count ++;

	if(total_count % 30 == 0){
		auto cur_avg_time = system_clock::now();
		auto dt_avg = duration_cast<milliseconds>(cur_avg_time - last_avg_time);
		dt_avg_v = (int32_t)dt_avg.count();
		last_avg_time = cur_avg_time;
	}

	last_time = cur_time;
	int32_t dt_v = (int32_t)dt.count();
	printf("frame size: w = %d, h = %d, fps_rt: %0.02f fps: %0.02f (%d)\n",
		   df->w, df->h, 1000.0f / dt_v,
		   dt_avg_v ? 1000.0f * 30 / dt_avg_v : 0, total_count);
}

int main(int argc, char **argv)
{
	DepthCameraCmdPort cmd_port;
	DepthCameraUvcPort uvc_port;

	std::vector<std::string> camera_list;
	string cmd_port_name;

	// get the valid camera names
	uvc_port.GetDepthCameraList(camera_list);
	for(auto name : camera_list){
		cout << name << endl;
	}

    if(camera_list.size() > 0){
		// get uvc relate cmd port(ttyACMx)
		if (cmd_port.GetUvcRelatedCmdPort(camera_list[0], cmd_port_name)) {
			// should open cmd port first
			if (cmd_port.Open(cmd_port_name)) {
				// setup depth data call back
				uvc_port.SetDepthFrameCallback(OnDepthFrame, nullptr);
				// open first camera			
				if (uvc_port.Open(camera_list[0])) {
					// get the camera status information
					std::string status;
					cmd_port.GetSystemStatus(status);
					cout << status << endl;
					// Grabber 10 seconds frame
					this_thread::sleep_for(chrono::seconds(10));
					cout << "close uvc port" << endl;
					uvc_port.Close();
				}
				cout << "close cmd port" << endl;
				cmd_port.Close();
			}
		}		
	}
	cout << "app shutdown" << endl;
	return 0;
}
````

## Simple example to grabber frame data from depth camera using cmd_video port

````
#include <iostream>
#include <stdio.h>
#include <depth_camera_cmd_video.h>

using namespace std;
using namespace chrono;

void OnDepthFrame(const DepthFrame *df, void*param){

	static auto last_time = system_clock::now();
	static auto last_avg_time = system_clock::now();
	static int32_t dt_avg_v = 0, total_count = 0;
	auto cur_time = system_clock::now();
	auto dt = duration_cast<milliseconds>(cur_time - last_time);

	total_count ++;

	if(total_count % 30 == 0){
		auto cur_avg_time = system_clock::now();
		auto dt_avg = duration_cast<milliseconds>(cur_avg_time - last_avg_time);
		dt_avg_v = (int32_t)dt_avg.count();
		last_avg_time = cur_avg_time;
	}

	last_time = cur_time;
	int32_t dt_v = (int32_t)dt.count();
	printf("frame size: w = %d, h = %d, fps_rt: %0.02f fps: %0.02f (%d)\n",
		   df->w, df->h, 1000.0f / dt_v,
		   dt_avg_v ? 1000.0f * 30 / dt_avg_v : 0, total_count);
}

int main(int argc, char **argv)
{
	DepthCameraCmdVideo cmd_video_port;

	std::vector<std::string> camera_list;
	string cmd_port_name;

	// get the valid camera names
	cmd_video_port.GetDepthCameraList(camera_list);
	for(auto name : camera_list){
		std::cout << name << endl;
	}

    if(camera_list.size() > 0){
		cmd_video_port.SetDepthFrameCallback(OnDepthFrame, nullptr);
		
		if (cmd_video_port.Open(camera_list[0])) {
			std::string status;
			cmd_video_port.GetSystemStatus(status);
			std::cout << status << endl;

			// Grabber 10 seconds frame
			this_thread::sleep_for(chrono::seconds(10));

			std::cout << "close uvc port" << endl;
			cmd_video_port.Close();
		}
	}
	std::cout << "app shutdown" << endl;
	return 0;
}
````

Email: ggh@imscv.com<br>
Websit: http://robot.imscv.com/
