#include <fstream>
#include "depth_camera_cmd.h"

#ifdef _MSC_VER
#pragma warning(disable:4996) // disable "declared deprecated" warning
#endif

static const uint32_t Crc32Table[] =
{
	0x00000000, 0x04C11DB7, 0x09823B6E, 0x0D4326D9, 0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005,
	0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61, 0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD,
	0x4C11DB70, 0x48D0C6C7, 0x4593E01E, 0x4152FDA9, 0x5F15ADAC, 0x5BD4B01B, 0x569796C2, 0x52568B75,
	0x6A1936C8, 0x6ED82B7F, 0x639B0DA6, 0x675A1011, 0x791D4014, 0x7DDC5DA3, 0x709F7B7A, 0x745E66CD,
	0x9823B6E0, 0x9CE2AB57, 0x91A18D8E, 0x95609039, 0x8B27C03C, 0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5,
	0xBE2B5B58, 0xBAEA46EF, 0xB7A96036, 0xB3687D81, 0xAD2F2D84, 0xA9EE3033, 0xA4AD16EA, 0xA06C0B5D,
	0xD4326D90, 0xD0F37027, 0xDDB056FE, 0xD9714B49, 0xC7361B4C, 0xC3F706FB, 0xCEB42022, 0xCA753D95,
	0xF23A8028, 0xF6FB9D9F, 0xFBB8BB46, 0xFF79A6F1, 0xE13EF6F4, 0xE5FFEB43, 0xE8BCCD9A, 0xEC7DD02D,
	0x34867077, 0x30476DC0, 0x3D044B19, 0x39C556AE, 0x278206AB, 0x23431B1C, 0x2E003DC5, 0x2AC12072,
	0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16, 0x018AEB13, 0x054BF6A4, 0x0808D07D, 0x0CC9CDCA,
	0x7897AB07, 0x7C56B6B0, 0x71159069, 0x75D48DDE, 0x6B93DDDB, 0x6F52C06C, 0x6211E6B5, 0x66D0FB02,
	0x5E9F46BF, 0x5A5E5B08, 0x571D7DD1, 0x53DC6066, 0x4D9B3063, 0x495A2DD4, 0x44190B0D, 0x40D816BA,
	0xACA5C697, 0xA864DB20, 0xA527FDF9, 0xA1E6E04E, 0xBFA1B04B, 0xBB60ADFC, 0xB6238B25, 0xB2E29692,
	0x8AAD2B2F, 0x8E6C3698, 0x832F1041, 0x87EE0DF6, 0x99A95DF3, 0x9D684044, 0x902B669D, 0x94EA7B2A,
	0xE0B41DE7, 0xE4750050, 0xE9362689, 0xEDF73B3E, 0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2,
	0xC6BCF05F, 0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686, 0xD5B88683, 0xD1799B34, 0xDC3ABDED, 0xD8FBA05A,
	0x690CE0EE, 0x6DCDFD59, 0x608EDB80, 0x644FC637, 0x7A089632, 0x7EC98B85, 0x738AAD5C, 0x774BB0EB,
	0x4F040D56, 0x4BC510E1, 0x46863638, 0x42472B8F, 0x5C007B8A, 0x58C1663D, 0x558240E4, 0x51435D53,
	0x251D3B9E, 0x21DC2629, 0x2C9F00F0, 0x285E1D47, 0x36194D42, 0x32D850F5, 0x3F9B762C, 0x3B5A6B9B,
	0x0315D626, 0x07D4CB91, 0x0A97ED48, 0x0E56F0FF, 0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623,
	0xF12F560E, 0xF5EE4BB9, 0xF8AD6D60, 0xFC6C70D7, 0xE22B20D2, 0xE6EA3D65, 0xEBA91BBC, 0xEF68060B,
	0xD727BBB6, 0xD3E6A601, 0xDEA580D8, 0xDA649D6F, 0xC423CD6A, 0xC0E2D0DD, 0xCDA1F604, 0xC960EBB3,
	0xBD3E8D7E, 0xB9FF90C9, 0xB4BCB610, 0xB07DABA7, 0xAE3AFBA2, 0xAAFBE615, 0xA7B8C0CC, 0xA379DD7B,
	0x9B3660C6, 0x9FF77D71, 0x92B45BA8, 0x9675461F, 0x8832161A, 0x8CF30BAD, 0x81B02D74, 0x857130C3,
	0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640, 0x4E8EE645, 0x4A4FFBF2, 0x470CDD2B, 0x43CDC09C,
	0x7B827D21, 0x7F436096, 0x7200464F, 0x76C15BF8, 0x68860BFD, 0x6C47164A, 0x61043093, 0x65C52D24,
	0x119B4BE9, 0x155A565E, 0x18197087, 0x1CD86D30, 0x029F3D35, 0x065E2082, 0x0B1D065B, 0x0FDC1BEC,
	0x3793A651, 0x3352BBE6, 0x3E119D3F, 0x3AD08088, 0x2497D08D, 0x2056CD3A, 0x2D15EBE3, 0x29D4F654,
	0xC5A92679, 0xC1683BCE, 0xCC2B1D17, 0xC8EA00A0, 0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB, 0xDBEE767C,
	0xE3A1CBC1, 0xE760D676, 0xEA23F0AF, 0xEEE2ED18, 0xF0A5BD1D, 0xF464A0AA, 0xF9278673, 0xFDE69BC4,
	0x89B8FD09, 0x8D79E0BE, 0x803AC667, 0x84FBDBD0, 0x9ABC8BD5, 0x9E7D9662, 0x933EB0BB, 0x97FFAD0C,
	0xAFB010B1, 0xAB710D06, 0xA6322BDF, 0xA2F33668, 0xBCB4666D, 0xB8757BDA, 0xB5365D03, 0xB1F740B4
};

DepthCameraCmdPort::DepthCameraCmdPort()
{
	mStopUpgradingFlag = false;
	mIsUpgrading = false;
	mUpgradeProgress = 0;
}

DepthCameraCmdPort::~DepthCameraCmdPort()
{

}

bool DepthCameraCmdPort::StartUpgrade(string firmware_file_name, string type)
{
	if (mIsUpgrading)
		return false;

	// check if the file is avilable
	std::ifstream fw(firmware_file_name, ios::in);
	if (!fw.is_open()) {
		mUpgradeProgress = -1;
		return false;
	}
	fw.close();

	// start upgrade file in a independent thread
	mUpgradeFuture = std::async(std::launch::async, [this, firmware_file_name, type]{
		// reset upgrade status
		mUpgradeProgress = 0;
		mStopUpgradingFlag = false;

		// Open firmware file
		std::ifstream fw_file(firmware_file_name, ios::binary|ios::in);
		if(!fw_file.is_open()){
			mUpgradeProgress = -1;
			return false;
		}
		fw_file.seekg(0, ios::end);
		uint32_t firmware_file_size = (uint32_t)fw_file.tellg();
		fw_file.seekg(0, ios::beg);

		// Set upgrade flag
		mIsUpgrading = true;

		// send erase cmd
		char cmd_str[128];
		int32_t cmd_str_len = sprintf(cmd_str, "fwu %s erase\r\n", type.c_str());
		if(SendCmdAndWaitResult(cmd_str, cmd_str_len, "success ->", 10000) == false){
			mUpgradeProgress = -2;
			mIsUpgrading = false;
			return false;
		}

		// read firmware file , encode to base64, send to camera
		const int32_t SINGLE_TX_LEN = 128;
		char *buf_read = new char[SINGLE_TX_LEN];
		char *buf_send = new char[SINGLE_TX_LEN * 2];
		int32_t total_tx_len = 0;
		uint32_t file_crc = 0xFFFFFFFF;
		do{
			fw_file.read(buf_read, SINGLE_TX_LEN);
			int32_t read_len = (int32_t)fw_file.gcount();
			if(read_len > 0){
				uint32_t crc = Crc32((const uint8_t *)buf_read, read_len);
				int32_t cmd_len = sprintf(buf_send, "fwu %s write 0x%x 0x%x 0x%x ", type.c_str(), total_tx_len, read_len, crc);
				int32_t base64_len = Base64Encode(buf_read, read_len, buf_send + cmd_len, SINGLE_TX_LEN * 2 - 2 - cmd_len);
				if(base64_len > 0){
					cmd_len += base64_len;
					buf_send[cmd_len++] = '\r';
					buf_send[cmd_len++] = '\n';
					buf_send[cmd_len] = '\0';
					if(SendCmdAndWaitResult(buf_send, cmd_len, "success ->", 2000) == false){
						mUpgradeProgress = -3;
						break;
					}
					total_tx_len += read_len;
					file_crc = Crc32((const uint8_t *)buf_read, read_len, 0, file_crc);
					mUpgradeProgress = total_tx_len * 100 / firmware_file_size;
				}else{
					mUpgradeProgress = -4;
					break;
				}
			}
		}while(!fw_file.eof() && !mStopUpgradingFlag);

		delete []buf_read;
		delete []buf_send;
		fw_file.close();

		cmd_str_len = sprintf(cmd_str, "fwu %s finish 0x%x 0x%x\r\n", type.c_str(), total_tx_len, file_crc);
		if (SendCmdAndWaitResult(cmd_str, cmd_str_len, "success ->", 3000) == false) {
			mUpgradeProgress = -5;
			mIsUpgrading = false;
			return false;
		}

		mIsUpgrading = false;
		return mUpgradeProgress == 100 ? true : false;
	});
	return true;
}

bool DepthCameraCmdPort::StopUpgrade()
{
	//wait for future result
	if (mUpgradeFuture.valid()) {
		mStopUpgradingFlag = false;
		mUpgradeFuture.get();
		mIsUpgrading = false;
	}
	return true;
}

int32_t DepthCameraCmdPort::GetUpgradeProgress()
{
	return mUpgradeProgress;
}

int32_t DepthCameraCmdPort::IsUpgrading()
{
	return mIsUpgrading;
}

bool DepthCameraCmdPort::SetIntegrationTime(uint8_t value)
{
	char cmd[32];
	int32_t len = snprintf(cmd, sizeof(cmd), "intg %d\r\n", value);
	return SendCmdAndWaitResult(cmd, len, "success ->");
}

bool DepthCameraCmdPort::SetExternIlluminatePower(uint8_t value)
{
	char cmd[32];
	int32_t len = snprintf(cmd, sizeof(cmd), "illum cd %d\r\n", value);
	return SendCmdAndWaitResult(cmd, len, "success ->");
}

bool DepthCameraCmdPort::SetInternalIlluminatePower(uint8_t value)
{
	char cmd[32];
	int32_t len = snprintf(cmd, sizeof(cmd), "inled %d\r\n", value);
	return SendCmdAndWaitResult(cmd, len, "success ->");
}

bool DepthCameraCmdPort::SetFrameRate(uint16_t value)
{
	char cmd[32];
	int32_t len = snprintf(cmd, sizeof(cmd), "fps %d\r\n", value);
	return SendCmdAndWaitResult(cmd, len, "success ->");
}

bool DepthCameraCmdPort::SwitchMirror()
{
	char cmd[32];
	int32_t len = snprintf(cmd, sizeof(cmd), "mirror\r\n");
	return SendCmdAndWaitResult(cmd, len, "success ->");
}

bool DepthCameraCmdPort::SetBinning(uint8_t rows, uint8_t columns)
{
	char cmd[32];
	rows = rows > 3 ? 3 : rows;
	columns = columns > 3 ? 3 : columns;
	int32_t len = snprintf(cmd, sizeof(cmd), "binning %d %d\r\n", rows, columns);
	return SendCmdAndWaitResult(cmd, len, "success ->");
}

bool DepthCameraCmdPort::RestoreFactorySettings()
{
	char cmd[32];
	int32_t len = snprintf(cmd, sizeof(cmd), "rfs\r\n");
	return SendCmdAndWaitResult(cmd, len, "success ->");
}

bool DepthCameraCmdPort::SetHdrRatio(uint8_t value) {
	char cmd[32];
	int32_t len = snprintf(cmd, sizeof(cmd), "hdr %d\r\n", value);
	return SendCmdAndWaitResult(cmd, len, "success ->");
}

bool DepthCameraCmdPort::GetDepthScale(float & scale)
{
	char response_buf[1024];
	char cmd[32];
	int32_t len = snprintf(cmd, sizeof(cmd), "scale\r\n");
	int32_t res_len = SendCmdAndWaitResponse(cmd, len, 3000, response_buf, sizeof(response_buf) - 1, "scale");
	if (res_len > 0) {
		response_buf[res_len] = 0;
		char * str = strstr((char*)response_buf, "scale: ");
		char * endstr = strstr((char*)response_buf, "\nsuccess ->");
		if (endstr) *endstr = 0;
		if (str) {
			str += strlen("scale: ");
			scale = strtof(str, NULL);
			return (scale == 0) ? false : true;
		}
	}
	return false;
}

bool DepthCameraCmdPort::Calibration(int32_t distance, int32_t freq_cnt)
{
	char cmd[32];

	for (int i = 1; i <= freq_cnt; i++) {
		// begin test
		int32_t len = snprintf(cmd, sizeof(cmd), "cali f%d test\r\n", i);
		bool ret = SendCmdAndWaitResult(cmd, len, "success ->");
		if (!ret)
			return false;

		// wait for a second
		this_thread::sleep_for(chrono::seconds(2));

		// set data
		len = snprintf(cmd, sizeof(cmd), "cali f%d set %d\r\n", i, distance);
		ret = SendCmdAndWaitResult(cmd, len, "success ->");
		if (!ret)
			return false;
	}

	return true;
}

bool DepthCameraCmdPort::GetSystemStatus(string &status_str)
{
	char response_buf[2048];
	char cmd[32];
	int32_t len = snprintf(cmd, sizeof(cmd), "show\r\n");
	int32_t res_len = SendCmdAndWaitResponse(cmd, len, 3000, response_buf, sizeof(response_buf) - 1, "show");
	if (res_len > 0) {
		response_buf[res_len] = 0;
		char * str = strstr((char*)response_buf, "show");
		char * endstr = strstr((char*)response_buf, "\r\nidcs>");
		if(endstr) *endstr = 0;
		if (str){
			status_str = str + len - 1;
			return true;
		}
	}
	return false;
}

bool DepthCameraCmdPort::SaveConfig()
{
	char cmd[32];
	int32_t len = snprintf(cmd, sizeof(cmd), "save\r\n");
	return SendCmdAndWaitResult(cmd, len, "success ->");
}

bool DepthCameraCmdPort::CdcVideoControl(bool enable_disable)
{
	char cmd[32];
	int32_t len;
	if(enable_disable)
		len = snprintf(cmd, sizeof(cmd), "cdcv on\r\n");
	else
		len = snprintf(cmd, sizeof(cmd), "cdcv off\r\n");
	return SendCmdAndWaitResult(cmd, len, "success ->");
}

bool DepthCameraCmdPort::SetOperationMode(int32_t mode, int32_t slave_fps)
{
	char cmd[32];
	int32_t len;
	if (mode == 0)
		len = snprintf(cmd, sizeof(cmd), "mode m\r\n");
	else {
		if (slave_fps < 1 || slave_fps > 33)
			return false;
		len = snprintf(cmd, sizeof(cmd), "mode s %d\r\n", slave_fps);
	}
		
	return SendCmdAndWaitResult(cmd, len, "success ->");
}

void DepthCameraCmdPort::SystemReboot()
{
	SendCmd("reboot\r\n", 8);
}

bool DepthCameraCmdPort::SendCmdAndWaitResult(const char * cmd, int32_t cmd_len, const char * result_ok_str, int32_t timeout)
{
	char response_buf[1024];
	int32_t res_len = SendCmdAndWaitResponse(cmd, cmd_len, timeout, response_buf, sizeof(response_buf) - 1, result_ok_str);
	return res_len > 0 ? true : false;
}

int32_t DepthCameraCmdPort::Base64Encode(const char *input, size_t input_length, char *out, size_t out_length) {
	static const char base64_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/";
	
	int32_t i = 0, j = 0;
	char *out_begin = out;
	unsigned char a3[3];
	unsigned char a4[4];

	size_t encoded_length = (input_length + 2 - ((input_length + 2) % 3)) / 3 * 4;
	if (out_length < encoded_length)
		return -1;

	while (input_length--) {
		a3[i++] = *input++;
		if (i == 3) {
			a4[0] = (a3[0] & 0xfc) >> 2;
			a4[1] = ((a3[0] & 0x03) << 4) + ((a3[1] & 0xf0) >> 4);
			a4[2] = ((a3[1] & 0x0f) << 2) + ((a3[2] & 0xc0) >> 6);
			a4[3] = (a3[2] & 0x3f);
			for (i = 0; i < 4; i++)
				*out++ = base64_alphabet[a4[i]];
			i = 0;
		}
	}

	if (i) {
		for (j = i; j < 3; j++)
			a3[j] = '\0';

		a4[0] = (a3[0] & 0xfc) >> 2;
		a4[1] = ((a3[0] & 0x03) << 4) + ((a3[1] & 0xf0) >> 4);
		a4[2] = ((a3[1] & 0x0f) << 2) + ((a3[2] & 0xc0) >> 6);
		a4[3] = (a3[2] & 0x3f);

		for (j = 0; j < i + 1; j++)
			*out++ = base64_alphabet[a4[j]];

		while ((i++ < 3))
			*out++ = '=';
	}

	return (out == (out_begin + encoded_length)) ? encoded_length : -1;
}

uint32_t DepthCameraCmdPort::Crc32(const uint8_t *p, int len, int32_t offset, uint32_t init_crc)
{
	uint32_t crc = init_crc;
	int pos = offset;
	int u32len = len / 4;
	while (u32len-- > 0){
		crc = (crc << 8) ^ Crc32Table[((crc >> 24) & 0xff) ^ p[pos + 3]];
		crc = (crc << 8) ^ Crc32Table[((crc >> 24) & 0xff) ^ p[pos + 2]];
		crc = (crc << 8) ^ Crc32Table[((crc >> 24) & 0xff) ^ p[pos + 1]];
		crc = (crc << 8) ^ Crc32Table[((crc >> 24) & 0xff) ^ p[pos]];
		pos += 4;
	}
	crc ^= 0x00000000;
	crc &= 0xFFFFFFFF;
	return (crc);
}
