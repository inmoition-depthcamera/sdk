﻿#include <iostream>

#include <depth_camera_cmd.h>
#include <depth_camera_uvc.h>

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <GLFW/glfw3.h>

#include <VeraMono_font.h>
#include <sstream>
#include <fstream>  
#include <ctime>
#include <iomanip>

using namespace std;
using namespace chrono;

GLFWwindow* MainWnd;

#define FONT_SIZE 14
#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))

bool ShowPointsCloudFlag = false, ShowStaticsFlag = false;
bool ShowFirmareUpgradeFlag = false, ShowConfigFlag = false;
int32_t SavePointsCount = 0, SavePointsAvgCountSize = 0;


float DepthScale = 7.5f / 3072, MaxRange = 7.5f;
int32_t MainMenuHeight = 20;
string CurrentUvcName;

string TempStrings[3] = { "", "", "" };
bool TemperatureRefreshFlag = true;
int32_t CurrentConfidence;
float CurrentDistance;

bool InitPointsCloudWindow(int32_t w, int32_t h, DepthCameraUvcPort *uvc, float scale, float max_range);
void UpdatePointsCloudWindow(bool *show_hide, DepthFrame *df);

static void ShowHelpMarker(const char* desc)
{
	ImGui::TextDisabled("?");
	if (ImGui::IsItemHovered()){
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(450.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

static void DrawDepthFrame(DepthFrame *df, DepthCameraUvcPort *uvc) {
	int fw, fh;
	static uint8_t * rgb_buf = NULL;
	
	int32_t frame_size = df->w * df->h;

	if (rgb_buf == NULL)
		rgb_buf = new uint8_t[frame_size * 3 * 4];

	glfwGetFramebufferSize(MainWnd, &fw, &fh);
	int x, y, h = fh - MainMenuHeight, w = fw;
	float factor = df->w * 1.0f / df->h;
	if (w > h * factor) {
		w = (int)(h * factor);
		h = fh - MainMenuHeight;
		x = (fw - w) / 2;
		y = 0;
	}else {
		w = fw;
		h = (int)(w / factor);
		y = (fh - MainMenuHeight - h) / 2;
		x = 0;
	}

	// Draw Pixels
	glViewport(x, y, w, h);
	df->ToRgb24(rgb_buf, frame_size * 3 * 4);
	glRasterPos2f(-1, 1);
	glPixelZoom((float)w / (float)(df->w * 2), -(float)(h) / (float)(df->h * 2));
	glDrawPixels(df->w * 2, df->h * 2, GL_RGB, GL_UNSIGNED_BYTE, rgb_buf);

	// Draw Grid
	glColor3f(72.0f / 255.0f, 72.0f / 255.0f, 120.0f / 255.0f);
	glLineWidth(1);
	glBegin(GL_LINES);
	glVertex2f(-1.0, 0);
	glVertex2f(1.0f, 0);
	glVertex2f(0, -1.0);
	glVertex2f(0, 1.0);
	glEnd();
	glBegin(GL_LINE_LOOP);
	double ed = 0.99999;
	glVertex2d(-ed, ed);
	glVertex2d(ed, ed);
	glVertex2d(ed, -ed);
	glVertex2d(-ed, -ed);
	glEnd();
	// Draw Center Rect
	float crs = 3.0f / df->w;
	glColor3f(1.0, 0.0, 0.0);
	float crs_pos[2][2] = {-0.5f, 0.5f, 0.5, 0.5};
	for (int i = 0; i < 2; i++) {
		glPushMatrix();
		glTranslatef(crs_pos[i][0], crs_pos[i][1], 0);
		glBegin(GL_LINE_LOOP);
		glVertex2f(-crs, crs * w / h);
		glVertex2f(crs, crs * w / h);
		glVertex2f(crs, -crs * w / h);
		glVertex2f(-crs, -crs * w / h);
		glEnd();
		glPopMatrix();
	}
}

static void DrawFirmwareUpgradeWindow(DepthCameraCmdPort *cmd, DepthCameraUvcPort *uvc) {
	if (ShowFirmareUpgradeFlag) {
		static char firmware_file_name[256];
		static char info_msg[256] = "";
		static float upgrade_progress = 0;
		static bool last_uvc_status, last_upgrade_status = false;
		int cur_progress = cmd->GetUpgradeProgress() ;
		upgrade_progress = (float)(cur_progress > 0 ? cur_progress / 100.0f : 0);
		if (!cmd->IsUpgrading() && last_upgrade_status) {
			if (cur_progress < 0) {
				sprintf(info_msg, "Update failed!");
			}else {
				sprintf(info_msg, "Update Finished!");
			}
			if (last_uvc_status) {
				last_uvc_status = false;
				uvc->Open(CurrentUvcName);
			}
		}
		last_upgrade_status = cmd->IsUpgrading();
		ImVec2 wnd_size = ImVec2(512, 0);
		if (ImGui::Begin("Firmware upgrade", &ShowFirmareUpgradeFlag, wnd_size, -1.0f, ImGuiWindowFlags_NoResize)) {
			ImGui::Text("Please input the firmware file path(*.ifw|*.tie):"); 			
			ImGui::SameLine();
			ShowHelpMarker("*.ifw is the MCU firmware file\n*.tie is the TOF firmware file");
			ImGui::PushItemWidth(-1);
			ImGui::InputText("Firmware File Name", firmware_file_name, IM_ARRAYSIZE(firmware_file_name)); 
			ImGui::PopItemWidth();
			ImGui::ProgressBar(upgrade_progress, ImVec2(-1.0f, 0.0f));
			ImGui::Separator();
			if (cmd->IsUpgrading() == false && ImGui::Button("Start Upgrade")) {
				string file_name = firmware_file_name;
				string type;
				if (file_name.find(".ifw") != string::npos)
					type = "app";
				else if (file_name.find(".tie") != string::npos)
					type = "dc";
				last_uvc_status = uvc->IsOpened();
				// Should Close UVC Port First
				uvc->Close();
				sprintf(info_msg, "Updating...");
				// Then Start upgrade
				if (cmd->StartUpgrade(file_name, type) == false) {
					sprintf(info_msg, "Update Failed!");
				}
				ImGui::SameLine();
			}
			if (cmd->IsUpgrading() && ImGui::Button("Stop Upgrade")) {
				cmd->StopUpgrade();
			}ImGui::SameLine();
			ShowHelpMarker("Mcu firmware will reload on next boot time.\nUpgrade progress will stop current video stream, and reopen after finished."); ImGui::SameLine();
			ImGui::Text("%s", info_msg);
			ImGui::SameLine();
			if (!cmd->IsUpgrading()) {
				if (ImGui::Button("Reboot")) {
					ImGui::OpenPopup("Reboot?");
				}ImGui::SameLine();
				ShowHelpMarker("Reboot the depth camera");
				if (ImGui::BeginPopupModal("Reboot?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
				{
					ImGui::Text("Reboot will lost usb connection, you nead to reopen the device!");
					ImGui::Separator();
					ImGui::Text("Are your sure to reboot device?");
					ImGui::SameLine();
					if (ImGui::Button("Yes", ImVec2(100, 0))) {
						uvc->Close();
						cmd->SystemReboot();
						cmd->Close();
						this_thread::sleep_for(chrono::seconds(2));
					}
					ImGui::SameLine();
					if (ImGui::Button("No", ImVec2(100, 0))) { ImGui::CloseCurrentPopup(); }
					ImGui::EndPopup();
				}
			}
		}
		ImGui::End();
	}	
}

static void DrawConfigWindow(DepthCameraCmdPort *cmd) {
	static bool last_show_status = false;
	int ww, wh;
	glfwGetWindowSize(MainWnd, &ww, &wh);

	// Draw Draw Config Window
	if (ShowConfigFlag) {
		static int32_t hdr_ratio = 0, cali_distance = 1000, integration_time = 40;
		static int32_t extern_illum_power = 0, internal_illum_power = 0, fps = 30;
		static int32_t mirror = 0, save_avg = 8;
		static string sensor_temperature = "", laser_temperature = "", illum_temperature = "";
		static string freq1 = "", freq2 = "", vco_freq1 = "", vco_freq2 = "";
		static string center_phase_value = "", center_amplitude_value = "", scale = "";
		static string mcu_fw_version = "", opt_fw_version = "", devcie_id = "", current_fps = "", max_distance = "0.0", max_phase = "3072";
		static char customer_cmd[1024];
		static bool cmd_sent_flag = false;
		ImVec2 wnd_size = ImVec2(520, 0);
		if (ImGui::Begin("Configuration", &ShowConfigFlag, wnd_size, -1.0f, ImGuiWindowFlags_NoResize)) {
			ImGui::PushItemWidth(200);			
			ImGui::SliderInt("hdr", &hdr_ratio, 0, 3); ImGui::SameLine();
			ShowHelpMarker("Config HDR Ratio to config the camera into hdr mode.\nSet to 0 to disable hdr mode.");
			ImGui::SameLine(); if (ImGui::Button("Set Hdr Ratio")) {
				cmd->SetHdrRatio(hdr_ratio);
			}
			ImGui::DragInt("intg", &integration_time, 0.5, 0, 100, "%.0f%%");; ImGui::SameLine();
			ShowHelpMarker("Set the camera's integration time");
			ImGui::SameLine(); if (ImGui::Button("Set Integration Time(%)")) {
				cmd->SetIntegrationTime((uint8_t)integration_time);
			}
			ImGui::SliderInt("exillu", &extern_illum_power, 0, 255); ImGui::SameLine();
			ShowHelpMarker("Set the extern illuminate power (0~255)");
			ImGui::SameLine(); if (ImGui::Button("Set Extern Illuminate")) {
				cmd->SetExternIlluminatePower((uint8_t)extern_illum_power);
			}
			ImGui::SliderInt("inillum", &internal_illum_power, 0, 255); ImGui::SameLine();
			ShowHelpMarker("Set the internal illuminate power (0~255)");
			ImGui::SameLine(); if (ImGui::Button("Set Internal Illuminate")) {
				cmd->SetInternalIlluminatePower((uint8_t)internal_illum_power);
			}
			//ImGui::SliderInt("fps", &fps, 10, 60); ImGui::SameLine();
			//ShowHelpMarker("Set the frame rate of the camera.\nSome depth camera will not work, in a wrong frame rate value.High frame rate will reduce the detect range of camera.");
			//ImGui::SameLine(); if (ImGui::Button("Set Frame Rate")) {
			//	cmd->SetFrameRate((uint16_t)fps);
			//}ImGui::SameLine();
			if (ImGui::Checkbox("Mirror", (bool *)&mirror)) {
				cmd->SwitchMirror();
			}ImGui::SameLine(); ShowHelpMarker("Mirror the output by 180 degree");
			ImGui::Separator();
			ImGui::InputInt("Calibration Distance(mm)", &cali_distance, 10, 1000); ImGui::SameLine();
			ShowHelpMarker("Calibration the phase offset use given distance at center 6*6 rect.");
			ImGui::SameLine(); if (ImGui::Button("Calibration")) {
				if (cmd->Calibration(cali_distance, 2))
					cout << "Calibration finished!" << endl;
				else
					cout << "Calibration failed!" << endl;
			}
			ImGui::Separator();
			sprintf(customer_cmd, "Save Frame(%d)", SavePointsCount);
			ImGui::InputInt(customer_cmd, &save_avg, 1, 1000); ImGui::SameLine();
			ShowHelpMarker("Save frame use given average frame.");
			ImGui::SameLine(); if (ImGui::Button("Save")) {
				if (save_avg > 0) {
					SavePointsAvgCountSize = save_avg;
					SavePointsCount = save_avg;
				}
			}

			ImGui::PopItemWidth();

			ImGui::Separator();
			if (cmd_sent_flag) {
				ImGui::SetKeyboardFocusHere();
				cmd_sent_flag = false;
			}				
			customer_cmd[0] = 0;
			if (ImGui::InputText("Custom Command", customer_cmd, IM_ARRAYSIZE(customer_cmd) - 2,
				ImGuiInputTextFlags_EnterReturnsTrue)) {
				int32_t len = strlen(customer_cmd);
				if (len > 0) {
					customer_cmd[len++] = '\r';
					customer_cmd[len++] = '\n';
					customer_cmd[len] = '\0';
					cmd->SendCmd(customer_cmd, len);
					cmd_sent_flag = true;
					customer_cmd[len - 2] = '\0';
				}
			}ImGui::SameLine();
			ShowHelpMarker("Press ENTER to Send custom command to camera");

			const char *string_keys[] = { "Device ID", "Mcu Firmware Version", "Opt Firmware Version", "Center Phase Value", "Center Amplitude Value",
				"FPS", "Max Distance(m)", "Max Avilable Phase Value", "Phase To Distance Scale", "Freq1(MHz)", "Freq2(MHz)", "Freq1Vco(MHz)", "Freq2Vco(MHz)",
				"Sensor Temperature", "Illum Temperature", "Laser Driver Temperature" };
			static string *string_values[] = { &devcie_id , &mcu_fw_version, &opt_fw_version, &center_phase_value, &center_amplitude_value,
				&current_fps , &max_distance, &max_phase, &scale, &freq1 , &freq2, &vco_freq1, &vco_freq2 , 
				&sensor_temperature, &illum_temperature, &laser_temperature };

			if (ImGui::CollapsingHeader("Camera Status")) {
				wnd_size.y = 0;
				ImGui::SetWindowSize("Configuration", wnd_size);
				ImGui::Columns(2, "status values");
				ImGui::Separator();
				ImGui::Text("Status Name"); ImGui::NextColumn();
				ImGui::Text("Value"); ImGui::NextColumn();
				ImGui::Separator();
				for (int i = 0; i < IM_ARRAYSIZE(string_keys); i++)
				{
					ImGui::Text("%s", string_keys[i]); ImGui::NextColumn();
					ImGui::Text("%s", string_values[i]->c_str()); ImGui::NextColumn();
				}
				ImGui::Columns(1);
			}else{
				wnd_size.y = 0;
				ImGui::SetWindowSize("Configuration", wnd_size);
			}

			ImGui::Separator();
			if (ImGui::Button("Get Values") || (last_show_status == false) || TemperatureRefreshFlag) {
				string status;
				if (cmd->GetSystemStatus(status)) {
					std::stringstream ss;
					ss.str(status);
					string line;
					const char *int32_keys[] = {"FPS", "Integration Time(%)", "Extern Illumination", 
						                  "Internal Illumination", "Hdr Ratio", "Mirror" };
					static int32_t *int32_values[] = {&fps , &integration_time, &extern_illum_power,
					                     &internal_illum_power , &hdr_ratio, &mirror};
					while (getline(ss, line)) {
						unsigned long pos;
						for (int i = 0; i < IM_ARRAYSIZE(int32_keys); i++) {
							if ((pos = line.find(int32_keys[i])) != string::npos) {
								string v = line.substr(pos + 2 + strlen(int32_keys[i]));
								*int32_values[i] = std::stoi(v);
								break;
							}
						}
						for (int i = 0; i < IM_ARRAYSIZE(string_keys); i++) {
							if ((pos = line.find(string_keys[i])) != string::npos) {
								string v = line.substr(pos + 2 + strlen(string_keys[i]));
								*string_values[i] = v;
								break;
							}
						}
					}

					TempStrings[0] = sensor_temperature;
					TempStrings[1] = illum_temperature;
					TempStrings[2] = laser_temperature;
					TemperatureRefreshFlag = false;
					MaxRange = std::stof(max_distance);
				}
				
			}ImGui::SameLine();
			ShowHelpMarker("Read current value from camera");
			ImGui::SameLine();
			if (ImGui::Button("Save Config")) {
				cmd->SaveConfig();
			}ImGui::SameLine();
			ShowHelpMarker("Save modified settings to internal FLASH");
			ImGui::SameLine();
			if (ImGui::Button("Restore Factory Config")) {
				cmd->RestoreFactorySettings();
			}ImGui::SameLine();
			ShowHelpMarker("Restore depth camera setting to factory setting.");
		}
		ImGui::End();
	}

	last_show_status = ShowConfigFlag;
}

static void DrawMainWindow(DepthCameraCmdPort * cmd, DepthCameraUvcPort *uvc, DepthFrame *df) {
	int ww, wh;
	static bool open_video_only = false, open_cmd_only = false, hdr_filter = false;
	glfwGetWindowSize(MainWnd, &ww, &wh);
	// Draw MainMenu
	if (ImGui::BeginMainMenuBar()) {
		bool both_opened = cmd->IsOpened() && uvc->IsOpened();
		bool at_least_one_opend = cmd->IsOpened() | uvc->IsOpened();	
		if (ImGui::BeginMenu("Device")) {
			if (ImGui::BeginMenu("Open", !at_least_one_opend)) {
				vector<string> camera_list;
				uvc->GetDepthCameraList(camera_list);
				if (camera_list.size() > 0) {
					for (string full_name : camera_list) {
#ifdef WIN32
#else
						full_name = full_name.substr(0, full_name.find_last_of(":")); // add to fix Linux name match issue.
#endif
						string simple_name = full_name.substr(full_name.find_last_of("__") + 1);
						if (ImGui::MenuItem(simple_name.c_str())) {
							cout << "Opening " << simple_name << " ..." << endl;
							string cmd_port_name;
							if (cmd->GetUvcRelatedCmdPort(full_name, cmd_port_name)) {
								bool ret = true;
								// open cmd port first
								if (!open_video_only) {
									ret = cmd->Open(cmd_port_name);
									if (ret) {
										cout << "Open Cmd Port successed" << endl;										
									}
								}
								if (ret && !open_cmd_only) {
									if (uvc->Open(full_name)) {
										cout << "Open Uvc port successed" << endl;
										CurrentUvcName = full_name;
										// get camera infomatrions
										cmd->GetDepthScale(DepthScale);
									}else {
										cout << "Open Uvc port failed!" << endl;
										cout << "Will close the cmd port" << endl;
										if(!open_video_only)
											cmd->Close();
									}
								}
							}else
								cout << "Can't find UVC related command port!" << endl;
						}
					}
				} else {
					ImGui::MenuItem("No depth device", NULL, (bool *)NULL, false);
				}
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Open video only", NULL, &open_video_only, !at_least_one_opend)) open_cmd_only = false;
			if (ImGui::MenuItem("Open cmd only", NULL, &open_cmd_only, !at_least_one_opend)) open_video_only = false;
			if (ImGui::MenuItem("Close", NULL, (bool *)NULL, at_least_one_opend)) {
				if (uvc->IsOpened()) {
					cout << "Closing uvc port" << endl;
					uvc->Close();
				}
				if (cmd->IsOpened()) {
					cout << "Closing cmd port" << endl;
					cmd->Close();
				}
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Config")){
			ImGui::MenuItem("Parameter Config", NULL, &ShowConfigFlag, both_opened);
			ImGui::MenuItem("Upgrade Firmware", NULL, &ShowFirmareUpgradeFlag, cmd->IsOpened());
			ImGui::EndMenu();
		}
		
		if (ImGui::BeginMenu("View")){
			ImGui::MenuItem("3D Points Cloud", NULL, &ShowPointsCloudFlag, uvc->IsOpened());
			ImGui::MenuItem("Histogram", NULL, &ShowStaticsFlag, uvc->IsOpened());
			if (ImGui::MenuItem("Enable Uvc Hdr filter", NULL, &hdr_filter, uvc->IsOpened())) {
				uvc->SetHdrMode(hdr_filter);
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
	
	// Draw Info panel
	ImGui::SetNextWindowPos(ImVec2(10, wh - 95.0f));
	if (ImGui::Begin("Frame Info", NULL, ImVec2(160, 80), 0.3f,
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings)) {
		auto t = chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		std::stringstream ss;
		ss << put_time(localtime(&t), "%Y-%m-%d %H:%M:%S");
		ImGui::Text(ss.str().c_str(), ImGui::GetIO().Framerate);
		ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);
		ImGui::Separator();
		if (df) {
			#define FILETER_SIZE 16
			static uint32_t phase_sum = 0, amplitude_sum = 0;
			static uint32_t index = 0;
			static uint32_t phase_array[FILETER_SIZE], amplitude_array[FILETER_SIZE];
			uint32_t pv = df->CalcCenterRectSum(0, 6, 6) / 36;
			uint32_t av = df->CalcCenterRectSum(1, 6, 6) / 36;
			phase_sum -= phase_array[index];
			phase_array[index] = pv;
			phase_sum += pv;
			
			amplitude_sum -= amplitude_array[index];
			amplitude_array[index] = av;
			amplitude_sum += av;
			if (++index == FILETER_SIZE) index = 0;

			CurrentDistance = phase_sum * DepthScale / FILETER_SIZE;
			CurrentConfidence = amplitude_sum / FILETER_SIZE;
			ImGui::Text("Phase: %d(%.3fm)\nAmplitude: %d",
				phase_sum / FILETER_SIZE, CurrentDistance, CurrentConfidence);
		}else
			ImGui::Text("Phase: N/A\nAmplitude: N/A");
	}
	ImGui::End();
}

static void DrawStaticsWindow(DepthFrame *df, DepthCameraUvcPort *uvc) {
	static bool last_show_flag = false;
	int ww, wh;
	glfwGetWindowSize(MainWnd, &ww, &wh);

	// Set the first Position of modal window
	if (ShowStaticsFlag && last_show_flag != ShowStaticsFlag)
		ImGui::SetNextWindowPos(ImVec2(40, 40));
	last_show_flag = ShowStaticsFlag;
	// Draw Histogram Window
	if (ShowStaticsFlag) {
		if (ImGui::Begin("Histograms", &ShowStaticsFlag, ImVec2(wh / 1.5f, 0))) {
			static float phase_hist[4096], amplitude_hist[256];
			float phase_max = df->CalcHistogram(0, phase_hist, IM_ARRAYSIZE(phase_hist));
			float amplitude_max = df->CalcHistogram(1, amplitude_hist, IM_ARRAYSIZE(amplitude_hist));
			ImGui::PushItemWidth(-1);
			ImGui::PlotHistogram("", phase_hist, IM_ARRAYSIZE(phase_hist), 0, "Phase",
				0, phase_max, ImVec2(0, 120));
			ImGui::PlotHistogram("", amplitude_hist, IM_ARRAYSIZE(amplitude_hist), 0, "Amplitude",
				0, amplitude_max, ImVec2(0, 120));
			ImGui::PopItemWidth();
		}
		ImGui::End();
	}
}

static void OnRxCmdData(const uint8_t * data, int32_t len, void *param) {
	static char strbuf[4096];
	if (len > IM_ARRAYSIZE(strbuf) - 1) {
		len = IM_ARRAYSIZE(strbuf) - 1;
	}
	memcpy(strbuf, data, len);
	strbuf[len] = 0;
#ifndef _MSC_VER
	// Yallow output mark text as output from camera
	cout << "\033[;33m" << strbuf << "\033[0m" << endl;
#else
	cout << strbuf ;
#endif
}

//static void OnNewFrame(const DepthFrame *df, void* param) {
	// notify new frame event to main thread
//	glfwPostEmptyEvent();
//}



int main(int argc, char **argv)
{
	DepthCameraCmdPort cmd_port;
	DepthCameraUvcPort uvc_port;
	int32_t w = 320, h = 240;
	DepthFrame *df = NULL;
	int32_t *avg_phase = NULL;
	int32_t *avg_amplitude = NULL;
	int32_t *avg_ambient = NULL;


	// Initialize the glfw library
	if (!glfwInit())
		return -1;

	ImGui::GetStyle().WindowTitleAlign.x = 0.5f;
	ImGui::GetStyle().WindowRounding = 4;
	ImGui::GetStyle().FrameRounding = 2;
	ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(GetVeraMonoFontData(), GetVeraMonoFontSize(), FONT_SIZE);

	MainMenuHeight = (int)(ImGui::GetStyle().FramePadding.y * 2) + FONT_SIZE;

	MainWnd = glfwCreateWindow((int)(w * 2), (int)(h * 2) + MainMenuHeight, "Inmotion Depth Camera Full Example" , NULL, NULL);
	
	cmd_port.SetRxDataCallBack(OnRxCmdData, &cmd_port);
	//uvc_port.SetDepthFrameCallback(OnNewFrame, &uvc_port);

	if (!MainWnd){
		glfwTerminate();
		return -1;
	}
	
	InitPointsCloudWindow(w, h, &uvc_port, DepthScale, MaxRange);

	glfwMakeContextCurrent(MainWnd);
	ImGui_ImplGlfw_Init(MainWnd, true);
	
	while (!glfwWindowShouldClose(MainWnd))
	{
		this_thread::sleep_for(chrono::milliseconds(1));
		glfwPollEvents();
		
		// Main Window
		if (uvc_port.IsOpened()) {
			df = df ? df : new DepthFrame(uvc_port.GetWidth(), uvc_port.GetHeight());

			if (uvc_port.GetDepthFrame(df)){

				if (SavePointsCount > 0) {
					int32_t size = df->w * df->h;
					if (avg_phase == NULL) {
						avg_phase = new int32_t[size];
						avg_amplitude = new int32_t[size];
						avg_ambient = new int32_t[size];
					}

					if (SavePointsCount == SavePointsAvgCountSize) {
						memset(avg_phase, 0, size * sizeof(int32_t));
						memset(avg_amplitude, 0, size * sizeof(int32_t));
						memset(avg_ambient, 0, size * sizeof(int32_t));
					}

					for (int i = 0; i < size; i++) {
						avg_phase[i] += df->phase[i];
						avg_amplitude[i] += df->amplitude[i];
						avg_ambient[i] += df->ambient[i];
					}

					SavePointsCount--;
					
					if (SavePointsCount == 0 && SavePointsAvgCountSize > 0) {
						// last avg frame
						for (int i = 0; i < size; i++) {
							avg_phase[i] /= SavePointsAvgCountSize;
							avg_amplitude[i] /= SavePointsAvgCountSize;
							avg_ambient[i] /= SavePointsAvgCountSize;
						}
						const char *files[] = {"phase.csv", "amplitude.csv", "ambient.csv", "distance.csv"};
						const int *datas[] = { avg_phase , avg_amplitude, avg_ambient, avg_phase};
						const float factor[] = {1, 1, 1, DepthScale};
						for (int k = 0; k < 4; k++) {
							ofstream ofs(files[k]);
							if (ofs) {
								for (int i = 0; i < df->h; i++) {
									for (int j = 0; j < df->w; j++) {
										ofs << datas[k][i * df->w + j] * factor[k] << ",";
									}
									ofs << endl;
								}
							}
							ofs.close();
						}
					}
				}

				// PointsCloudWindow
				UpdatePointsCloudWindow(&ShowPointsCloudFlag, df);

				glfwMakeContextCurrent(MainWnd);
				ImGui_ImplGlfw_NewFrame(MainWnd);
				glClear(GL_COLOR_BUFFER_BIT);

				DrawMainWindow(&cmd_port, &uvc_port, df);
				DrawConfigWindow(&cmd_port);
				DrawStaticsWindow(df, &uvc_port);
				DrawFirmwareUpgradeWindow(&cmd_port, &uvc_port);

				DrawDepthFrame(df, &uvc_port);
				
				glfwGetFramebufferSize(MainWnd, &w, &h);
				glViewport(0, 0, w, h);
				ImGui::Render();
				glfwSwapBuffers(MainWnd);
			}
		}else { // device not opened, show main form only

			glfwMakeContextCurrent(MainWnd);
			ImGui_ImplGlfw_NewFrame(MainWnd);
			glClear(GL_COLOR_BUFFER_BIT);

			DrawMainWindow(&cmd_port, &uvc_port, NULL);

			if(cmd_port.IsOpened() && ShowFirmareUpgradeFlag)
				DrawFirmwareUpgradeWindow(&cmd_port, &uvc_port);

			glfwGetFramebufferSize(MainWnd, &w, &h);
			glViewport(0, 0, w, h);
			ImGui::Render();
			glfwSwapBuffers(MainWnd);
		}
	}
	
	cout << "close uvc port" << endl;
	uvc_port.Close();

	cout << "close cmd port" << endl;
	cmd_port.Close();

	delete df;
	glfwTerminate();
	cout << "app shutdown" << endl;
	return 0;
}