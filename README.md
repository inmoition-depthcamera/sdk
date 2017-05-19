# Inmotion Depth Camera SDK
Linux/Window SDK for Inmotion's Depth Cameras

乐行深度摄像头，对外接口是一个USB复合设备，其包含一个普通的UVC摄像头接口和一个USB虚拟串口。

本SDK提供了类库接口，用于对上述两个接口进行访问，获取数据，以及对深度摄像头进行配置。

Inmotion depth camera's external interface is a USB composite device that contains a common UVC camera interface and a USB virtual serial port.

This SDK provides a library interface for accessing those two interfaces to obtaining data, and configuring the depth camera.

## 支持的平台  Supported Platform
- Windows (Tested in Windows 10)
- Linux like platform(Tested in Ubuntu 16.04)
- Not supported in Virtual Mathine(VMware or VirtualBox)

## 库依赖 Dependences
### SDK库 SDK Dependences
#### Windows
Windows 系统，在SDK中直接使用Windows API 和Direct Show来进行接口访问和控制，不依赖于其他的类库。

The SDK using the Windows API directly and `Direct Show` for interface access and control and does not depend on other libraries

#### Linux
Linux 系统中，依赖如下库：
- **V4L** 在大多数Linux的发行版中已经内置了。 Already included in most Linux distributions.
- **libudev** 用于对USB设备信息进行管理（获取设备名称）。Used to manage USB device information.
- **cmake** 用于构建和编译工程。 Used to build and compile projects.

### 例子程序 Examples Dependences
- OpenGL
- glfw3

## 编译方法 Buld

### Windows
- 确保所有依赖库都已经安装。 Make sure that all dependent libraries are already installed.
- 安装 Visual Studio 2017. Install Visual Studio 2017.
- 进入`msvc`目录，打开`inmotion-depth-camera-sdk.sln`文件，完成编译。 Enter the `msvc` directory, open the `inmotion-depth-camera-sdk.sln file`, complete the compilation.

> 注意，编译例子程序时需要设置对应的依赖库的位置。 Note that you need to set the location of the corresponding dependency library when compiling the example program.

### Linux
- Install Dependences

````
sudo apt-get libglfw3-dev libudev-dev
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
- 生成的目标文件，在 `root_of_sdk_path/bin` 下。 The target files are under `root_of_sdk_path/bin`.

- 注意 Notes：
> 编译工具链需要支持`c++ 11`。 The compilation toolchain needs to support `c++ 11`

> The camera has two nodes in /dev which are: /dev/ttyACMx and /dev/videox. Make sure your user has right to access those devices before run the example or sdk. Use following instruction to get rights:
````
sudo usermod -a -G video,dialout $(whoami)
````


## 用法 Usage
- 包含对应的头文件 `depth_camera_uvc.h` 与 `depth_camera_cmd.h`。 Contains the corresponding header file `depth_camera_uvc.h` and` depth_camera_cmd.h`.
- 调用 `DepthCameraUvcPort::GetDepthCameraList` 来获得系统中所连接的深度摄像头。 Call `DepthCameraUvcPort::GetDepthCameraList` to get the depth cameras connected to the system.
- 调用 `DepthCameraCmdPort::GetUvcRelatedCmdPort` 来获取深度摄像头的命令端口名称。 Call `DepthCameraCmdPort::GetUvcRelatedCmdPort` to get the name of the command port for given depth camera name.
- 调用 `DepthCameraCmdPort::Open` 来打开命令端口。 **注意：命令端口需要比UVC端口先打开**。 Call `DepthCameraCmdPort::Open` to open the command port. **Note: The command port needs to be opened first than the UVC port**.
- 调用 `DepthCameraUvcPort::SetDepthFrameCallback` 来设置帧回调函数。或者不设置回调函数，在摄像头打开后直接调用`DepthCameraUvcPort::GetDepthFrame` 来获取最新的深度摄像头帧。 Call `DepthCameraUvcPort::SetDepthFrameCallback` to set the frame callback function. Or directly call `DepthCameraUvcPort::GetDepthFrame` to get the latest depth of the camera frame after opened the uvc port.
- 调用 `DepthCameraUvcPort::Open` 来打开UVC数据流端口。 **注意：命令端口需要比UVC端口先打开**。 Call `DepthCameraUvcPort::Open` to open the UVC data port. **Note: The command port needs to be opened first than the UVC port.**
- 调用 `DepthCameraCmdPort::GetDepthScale` 来获取深度数据和距离的转换系数。 Call `DepthCameraCmdPort::GetDepthScale` to get the depth data and distance conversion factor.
- 调用 `DepthCameraUvcPort::DepthToPointCloud` 来将深度数据帧，转换为三维点云数据。 Call `DepthCameraUvcPort::DepthToPointCloud` to convert deep data frames to 3D point cloud data.
- 执行用户运算。 Perform user operations.
- 调用 `DepthCameraUvcPort::Close` 来关闭UVC数据流接口。 Call `DepthCameraUvcPort::Close` to close the UVC data flow interface.
- 调用 `DepthCameraCmdPort::Close` 来关闭命令接口。Call `DepthCameraCmdPort::Close` to close the command interface.

## 深度数据帧 Depth Frame

在SDK中，一个深度帧，用如下类表示： In the SDK, a depth frame is represented by the following class:

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
	/// @brief Convent all depth info to a rgb24 buffer
	bool ToRgb24(uint8_t *rgb24_buf, int32_t rgb24_buf_size);
	/// @brief Calculate Histogram of the frame
	template<typename T>
	inline T CalcHistogram(int32_t phase_or_amplitude, T *histogram_buf, int32_t max);
};
````

每个深度帧，都包含如下几个部分： Each depth frame contains the following parts:
- **phase** 相位信息。相位信息和距离信息的换算关系为：distance = phase * K。 K与调制频率有关，可以通过 `GetDepthScale` 函数获得。phase只有低12位有效。 **phase** phase information. The conversion relationship between the phase and the distance is: distance = phase * K. K is related to the modulation frequency and can be obtained by the `GetDepthScale` function. Only low 12 bits are valid.
- **amplitude** 置信度（强度）信息。强度信息，表示每个像素点的置信度，置信度越高，噪声越小，可信度越高。只有低12位有效。（有些摄像头的置信度信息的低4位为0，实际有效的数据为 Bit 4 ~ Bit 11）。**amplitude** confidence level (strength) information. The confidence of each pixel. The higher the confidence, the smaller the noise, the higher the credibility. Only low 12 active. (Some of the camera's amplitude of the lower 4 bits is 0, the actual effective data for the Bit 4 ~ Bit 11).
- **ambient** 环境光信息。表示环境中有效波段光的强度。只有低3位有效。 Ambient light information. Indicates the intensity of the effective band of light in the environment. Only the lower 3 bits is valid.
- **flags**   过曝光信息。表示该像素点是否存在过曝，如果过曝，则该像素点应该被视为无效。只有低1位有效。 **flags** Exposure information. Indicates whether the pixel has been over exposed, and if it is over exposed, the pixel should be considered invalid. Only lower 1 bit is valid.

## 摄像头配置 Camera configuration

本SDK提供了如下接口来进行配置： This SDK provides the following interfaces for configuration:

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

> 上述接口函数的用法，请参考`depth_camera_cmd.h`文件中的注释。 Refer to the comments in the `depth_camera_cmd.h` file for usage of the above interface functions.

## 例子程序 Simple example to grabber frame data from depth camera

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

Email: ggh@imscv.com<br>
Websit: http://robot.imscv.com/
