#include <iostream>

#include <depth_camera_cmd_video.h>

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <GLFW/glfw3.h>

#include <VeraMono_font.h>
#include <sstream>

using namespace std;
using namespace chrono;

GLFWwindow* MainWnd;

#define FONT_SIZE 14
#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))

bool ShowPointsCloudFlag = false, ShowStaticsFlag = false;
bool ShowFirmareUpgradeFlag = false, ShowConfigFlag = false;

float DepthScale = 7.5f / 3072, MaxRange = 7.5f;
int32_t MainMenuHeight = 20;
string CurrentUvcName;

bool InitPointsCloudWindow(int32_t w, int32_t h, DepthCameraCmdVideo *cmd_video, float scale, float max_range);
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

static void DrawDepthFrame(DepthFrame *df, DepthCameraCmdVideo *cmd_video) {
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

static void DrawFirmwareUpgradeWindow(DepthCameraCmdVideo *cmd_video) {
	if (ShowFirmareUpgradeFlag) {
		static char firmware_file_name[256];
		static char info_msg[256] = "";
		static float upgrade_progress = 0;
		static bool last_uvc_status, last_upgrade_status = false;
		int cur_progress = cmd_video->GetUpgradeProgress() ;
		upgrade_progress = (float)(cur_progress > 0 ? cur_progress / 100.0f : 0);
		if (!cmd_video->IsUpgrading() && last_upgrade_status) {
			if (cur_progress < 0) {
				sprintf(info_msg, "Update failed!");
			}else {
				sprintf(info_msg, "Update Finished!");
			}
			if (last_uvc_status) {
				last_uvc_status = false;
				cmd_video->VideoControl(true);
			}
		}
		last_upgrade_status = cmd_video->IsUpgrading();
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
			if (cmd_video->IsUpgrading() == false && ImGui::Button("Start Upgrade")) {
				string file_name = firmware_file_name;
				string type;
				if (file_name.find(".ifw") != string::npos)
					type = "app";
				else if (file_name.find(".tie") != string::npos)
					type = "dc";
				last_uvc_status = cmd_video->IsOpened();
				// Should Close UVC Port First
				cmd_video->VideoControl(false);
				sprintf(info_msg, "Updating...");
				// Then Start upgrade
				if (cmd_video->StartUpgrade(file_name, type) == false) {
					sprintf(info_msg, "Update Failed!");
				}
				ImGui::SameLine();
			}
			if (cmd_video->IsUpgrading() && ImGui::Button("Stop Upgrade")) {
				cmd_video->StopUpgrade();
			}ImGui::SameLine();
			ShowHelpMarker("Mcu firmware will reload on next boot time.\nUpgrade progress will stop current video stream, and reopen after finished."); ImGui::SameLine();
			ImGui::Text("%s", info_msg);
			
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
		static int32_t extern_illum_power = 0, internal_illum_power = 0, fps = 30, fps_set = 30;
		static int32_t mirror = 0;
		static string freq1 = "", freq2 = "", vco_freq1 = "", vco_freq2 = "";
		static string center_phase_value = "", center_amplitude_value = "", scale = "";
		static string fw_version = "", product = "", devcie_id = "", set_fps = "";
		static string current_fps = "", max_distance = "0.0", max_phase = "3072";
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
			ImGui::SliderInt("fps", &fps_set, 10, 60); ImGui::SameLine();
			ShowHelpMarker("Set the frame rate of the camera.\nSome depth camera will not work, in a wrong frame rate value.High frame rate will reduce the detect range of camera.");
			ImGui::SameLine(); if (ImGui::Button("Set Frame Rate")) {
				cmd->SetFrameRate((uint16_t)fps);
			}ImGui::SameLine();
			if (ImGui::Checkbox("Mirror", (bool *)&mirror)) {
				cmd->SwitchMirror();
			}ImGui::SameLine(); ShowHelpMarker("Mirror the output by 180 degree");
			ImGui::Separator();
			ImGui::InputInt("Calibration Distance(mm)", &cali_distance, 10, 1000); ImGui::SameLine();
			ShowHelpMarker("Calibration the phase offset use given distance at center 6*6 rect.");
			ImGui::SameLine(); if (ImGui::Button("Calibration")) {
				if (cmd->Calibration(cali_distance, 1))
					cout << "Calibration finished!" << endl;
				else
					cout << "Calibration failed!" << endl;
			}
			ImGui::PopItemWidth();

			ImGui::Separator();
			if (cmd_sent_flag) {
				ImGui::SetKeyboardFocusHere();
				cmd_sent_flag = false;
			}				
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

			const char *string_keys[] = { "Product", "Device ID", "Firmware Version", "Center Phase Value", "Center Amplitude Value",
				"FPS", "Max Distance(m)", "Max Avilable Phase Value", "Phase To Distance Scale", "Freq1(MHz)", "Freq2(MHz)", "Freq1Vco(MHz)", "Freq2Vco(MHz)", "FrameRate Set"};
			static string *string_values[] = { &product, &devcie_id , &fw_version, &center_phase_value, &center_amplitude_value,
				&current_fps , &max_distance, &max_phase, &scale, &freq1 , &freq2, &vco_freq1, &vco_freq2, &set_fps };

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
			if (ImGui::Button("Get Values") || (last_show_status == false)) {
				string status;
				if (cmd->GetSystemStatus(status)) {
					std::stringstream ss;
					ss.str(status);
					string line;
					const char *int32_keys[] = {"FPS", "Integration Time(%)", "Extern Illumination", 
						                  "Internal Illumination", "Hdr Ratio", "Mirror", "FrameRate Set" };
					static int32_t *int32_values[] = {&fps , &integration_time, &extern_illum_power,
					                     &internal_illum_power , &hdr_ratio, &mirror, &fps_set};
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

static void DrawMainWindow(DepthCameraCmdVideo * cmd_video, DepthFrame *df) {
	int ww, wh;
	static bool hdr_filter = false;
	glfwGetWindowSize(MainWnd, &ww, &wh);
	// Draw MainMenu
	if (ImGui::BeginMainMenuBar()) {
		bool opened = cmd_video->IsOpened();
		if (ImGui::BeginMenu("Device")) {
			if (ImGui::BeginMenu("Open", !opened)) {
				vector<string> camera_list;
				cmd_video->GetDepthCameraList(camera_list);
				if (camera_list.size() > 0) {
					for (string full_name : camera_list) {
						if (ImGui::MenuItem(full_name.c_str())) {
							cout << "Opening " << full_name << " ..." << endl;
							// open cmd port first
							if (cmd_video->Open(full_name)) {
								cout << "Open Uvc port successed" << endl;
								CurrentUvcName = full_name;
								// get camera infomatrions
								cmd_video->GetDepthScale(DepthScale);
							}
						}
					}
				} else {
					ImGui::MenuItem("No depth device", NULL, (bool *)NULL, false);
				}
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Close", NULL, (bool *)NULL, opened)) {
				if (cmd_video->IsOpened()) {
					cout << "Closing cmd video port" << endl;
					cmd_video->Close();
				}
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Config")){
			ImGui::MenuItem("Parameter Config", NULL, &ShowConfigFlag, opened);
			ImGui::MenuItem("Upgrade Firmware", NULL, &ShowFirmareUpgradeFlag, opened);
			ImGui::EndMenu();
		}
		
		if (ImGui::BeginMenu("View")){
			ImGui::MenuItem("3D Points Cloud", NULL, &ShowPointsCloudFlag, opened);
			ImGui::MenuItem("Histogram", NULL, &ShowStaticsFlag, opened);
			if (ImGui::MenuItem("Enable Uvc Hdr filter", NULL, &hdr_filter, opened)) {
				cmd_video->SetHdrMode(hdr_filter);
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
	
	// Draw Info panel
	ImGui::SetNextWindowPos(ImVec2(10, wh - 100.0f));
	if (ImGui::Begin("Frame Info", NULL, ImVec2(170, 90), 0.3f,
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings)) {
		static int32_t dly_cnt = 0;
		static int64_t last_rx_cnt = 0;
		static auto last_time = system_clock::now();
		static float speed = 0;

		int64_t cur_cnt = cmd_video->GetRxCount();
		if(dly_cnt++ > 30){
			auto dcnt = cur_cnt - last_rx_cnt;
			auto dt = duration_cast<milliseconds>(system_clock::now() - last_time);
			speed = dt.count() ? dcnt * 1000.0f / (dt.count() * 1024) : 0;

			last_rx_cnt = cur_cnt;
			last_time = system_clock::now();
			dly_cnt = 0;
		}
		
		ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);
		if(cur_cnt > 1024 * 1024)
			ImGui::Text("Rx: %.2fM(%.2fk/s)", cur_cnt / (1024.0f * 1024.0f), speed);
		else
			ImGui::Text("Rx: %.2fK(%.2fk/s)", cur_cnt / (1024.0f), speed);
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
			ImGui::Text("Phase: %d(%.3fm)\nAmplitude: %d",
				phase_sum / FILETER_SIZE, phase_sum * DepthScale / FILETER_SIZE, amplitude_sum / FILETER_SIZE);
		}else
			ImGui::Text("Phase: N/A\nAmplitude: N/A");
	}
	ImGui::End();
}

static void DrawStaticsWindow(DepthFrame *df, DepthCameraCmdVideo *cmd_video) {
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

	while (data[len - 1] == 0)
		len--;

	if (len > IM_ARRAYSIZE(strbuf) - 1) {
		len = IM_ARRAYSIZE(strbuf) - 1;
	}

	memcpy(strbuf, data, len);
	strbuf[len] = 0;
#ifndef _MSC_VER
	// Yallow output mark text as output from camera
	cout << "\033[;33m" << strbuf << "\033[0m";
#else
	cout << strbuf;
#endif
}

int main(int argc, char **argv)
{
	DepthCameraCmdVideo cmd_video;
	int32_t w = 320, h = 240;
	DepthFrame *df = NULL;

	// Initialize the glfw library
	if (!glfwInit())
		return -1;

	ImGui::GetStyle().WindowTitleAlign.x = 0.5f;
	ImGui::GetStyle().WindowRounding = 4;
	ImGui::GetStyle().FrameRounding = 2;
	ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(GetVeraMonoFontData(), GetVeraMonoFontSize(), FONT_SIZE);

	MainMenuHeight = (int)(ImGui::GetStyle().FramePadding.y * 2) + FONT_SIZE;

	MainWnd = glfwCreateWindow((int)(w * 2), (int)(h * 2) + MainMenuHeight, "Inmotion Depth Camera Full Example Using CmdVideoPort" , NULL, NULL);
	
	cmd_video.SetRxDataCallBack(OnRxCmdData, &cmd_video);

	if (!MainWnd){
		glfwTerminate();
		return -1;
	}
	
	InitPointsCloudWindow(w, h, &cmd_video, DepthScale, MaxRange);

	glfwMakeContextCurrent(MainWnd);
	ImGui_ImplGlfw_Init(MainWnd, true);
	
	while (!glfwWindowShouldClose(MainWnd))
	{
		glfwPollEvents();
		
		// Main Window
		if (cmd_video.IsOpened()) {
			df = df ? df : new DepthFrame(cmd_video.GetWidth(), cmd_video.GetHeight());
			if (cmd_video.GetDepthFrame(df)){

				// PointsCloudWindow
				UpdatePointsCloudWindow(&ShowPointsCloudFlag, df);

				glfwMakeContextCurrent(MainWnd);
				ImGui_ImplGlfw_NewFrame(MainWnd);
				glClear(GL_COLOR_BUFFER_BIT);

				DrawMainWindow(&cmd_video, df);
				DrawConfigWindow(&cmd_video);
				DrawStaticsWindow(df, &cmd_video);
				DrawFirmwareUpgradeWindow(&cmd_video);

				DrawDepthFrame(df, &cmd_video);
				
				glfwGetFramebufferSize(MainWnd, &w, &h);
				glViewport(0, 0, w, h);
				ImGui::Render();
				glfwSwapBuffers(MainWnd);
			}
		}else { // device not opened, show main form only

			glfwMakeContextCurrent(MainWnd);
			ImGui_ImplGlfw_NewFrame(MainWnd);
			glClear(GL_COLOR_BUFFER_BIT);

			DrawMainWindow(&cmd_video, NULL);

			if(cmd_video.IsOpened() && ShowFirmareUpgradeFlag)
				DrawFirmwareUpgradeWindow(&cmd_video);

			glfwGetFramebufferSize(MainWnd, &w, &h);
			glViewport(0, 0, w, h);
			ImGui::Render();
			glfwSwapBuffers(MainWnd);
		}
	}
	
	cout << "close cmd video port" << endl;
	cmd_video.Close();
	
	delete df;
	glfwTerminate();
	cout << "app shutdown" << endl;
	return 0;
}