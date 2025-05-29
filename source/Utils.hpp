#pragma once
#include <curl/curl.h>
#include "parse-cta-block.hpp"

struct resolutionCalls {
	uint16_t width;
	uint16_t height;
	uint16_t calls;
};

struct NxFpsSharedBlock {
	uint32_t MAGIC;
	uint8_t FPS;
	float FPSavg;
	bool pluginActive;
	uint8_t FPSlocked;
	uint8_t FPSmode;
	uint8_t ZeroSync;
	uint8_t patchApplied;
	uint8_t API;
	uint32_t FPSticks[10];
	uint8_t Buffers;
	uint8_t SetBuffers;
	uint8_t ActiveBuffers;
	uint8_t SetActiveBuffers;
	bool displaySync;
	resolutionCalls renderCalls[8];
	resolutionCalls viewportCalls[8];
	bool forceOriginalRefreshRate;
	bool dontForce60InDocked;
	bool forceSuspend;
	uint8_t currentRefreshRate;
} NX_PACKED;

struct DockedAdditionalSettings {
	bool dontForce60InDocked;
	bool fpsTargetWithoutRRMatchLowest;
};


NxFpsSharedBlock* Shared = 0;
uint8_t* refreshRate_shared = 0;
bool _isDocked = false;
bool _def = true;
bool PluginRunning = false;
bool state = false;
bool closed = false;
bool check = false;
bool SaltySD = false;
bool bak = false;
bool plugin = true;
uint8_t SetBuffers_save = 0;
bool forceSuspend_save = false;
char FPSMode_c[64];
char FPSTarget_c[32];
char PFPS_c[32];
char nvnBuffers[96] = "";
char SyncWait_c[32];

char configPath[128] = "";
char patchPath[128] = "";
char savePath[64] = "";
uint64_t PID = 0;
uint64_t TID = 0;
uint64_t BID = 0;

Handle remoteSharedMemory = 1;
SharedMemory _sharedmemory = {};
bool SharedMemoryUsed = false;

Result configValid = 10;
Result patchValid = 0x202;
char lockInvalid[96] = "";
char patchChar[192] = "";
char patchAppliedChar[64] = "";
uint8_t* patchApplied_shared = 0;
Thread t0;
bool threadActive = true;
std::string ZeroSyncMode = "";

bool FileDownloaded = false;
Thread t1;
bool downloadingRunning = false;
Result error_code = UINT32_MAX;
bool curl_timeout = false;
uint8_t supportedHandheldRefreshRates[] = {40, 45, 50, 55, 60};

struct Title
{
	uint64_t TitleID;
	std::string TitleName;
};

struct DisplayData {
	uint32_t pixelClockkHz;
	uint16_t width;
	uint16_t height;
	float refreshRate;
	uint16_t widthFrontPorch;
	uint16_t heightFrontPorch;
	uint16_t widthSync;
	uint16_t heightSync;
	uint16_t widthBackPorch;
	uint16_t heightBackPorch;
};

std::vector<Title> titles;
std::string TV_name = "Unknown";

bool file_exists(const char *filename)
{
    struct stat buffer;
    return stat(filename, &buffer) == 0 ? true : false;
}

template <typename T> float parseEdid(T* edid_impl) {
	unsigned char* edid = (unsigned char*)edid_impl;
	uint8_t magic[8] = {0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0};
	if (memcmp(magic, edid, 8)) {
		#ifdef DEBUG
		printf("WRONG MAGIC! %d\n", memcmp(magic, edid, 8));
		#endif
		return 0;
	}
	SetSysModeLine* timingd = (SetSysModeLine*)calloc(sizeof(SetSysModeLine), 2);
	memcpy(timingd, &edid[offsetof(SetSysEdid, timing_descriptor)], sizeof(SetSysModeLine)*2);
	float highestRefreshRate = 0;
	for (size_t i = 0; i < 2; i++) {
		SetSysModeLine td = timingd[i];
		uint32_t width = (uint32_t)td.horizontal_active_pixels_msb << 8 | td.horizontal_active_pixels_lsb;
		uint32_t height = (uint32_t)td.vertical_active_lines_msb << 8 | td.vertical_active_lines_lsb;
		uint32_t h_total = width + ((uint32_t)td.horizontal_blanking_pixels_msb << 8 | td.horizontal_blanking_pixels_lsb);
		uint32_t v_total = height + ((uint32_t)td.vertical_blanking_lines_msb << 8 | td.vertical_blanking_lines_lsb);
		float refreshRate = ((float)(td.pixel_clock * 10000) / (float)(h_total * v_total));
		#ifdef DEBUG
		if (td.pixel_clock) printf("Res: %dx%d, pixel clock: %d, refresh rate: %0.4f\n", width, height, td.pixel_clock * 10, refreshRate);
		#endif
		if (!td.interlaced && refreshRate > highestRefreshRate) highestRefreshRate = refreshRate;
		/*
		if (!td.interlaced && ((width == 1280 && height == 720) || (width == 1920 && height == 1080)) && (refreshRate > 60)) {
			uint16_t widthSync = (uint16_t)td.horizontal_sync_pulse_width_pixels_msb << 8 | td.horizontal_sync_pulse_width_pixels_lsb;
			uint16_t heightSync = (uint16_t)td.vertical_sync_pulse_width_lines_msb << 8 | td.vertical_sync_pulse_width_lines_lsb;
			uint16_t widthFrontPorch = (uint16_t)td.horizontal_sync_offset_pixels_msb << 8 | td.horizontal_sync_offset_pixels_lsb;
			uint16_t heightFrontPorch = (uint16_t)td.horizontal_sync_offset_pixels_msb << 8 | td.horizontal_sync_offset_pixels_lsb;
			EdidData.push_back({
				.pixelClockkHz = (uint32_t)td.pixel_clock * 10,
				.width = (uint16_t)width,
				.height = (uint16_t)height,
				.refreshRate = refreshRate,
				.widthFrontPorch = widthFrontPorch,
				.heightFrontPorch = heightFrontPorch,
				.widthSync = widthSync,
				.heightSync = heightSync,
				.widthBackPorch = (uint16_t)(((uint16_t)td.horizontal_blanking_pixels_msb << 8 | td.horizontal_blanking_pixels_lsb) - (widthSync + widthFrontPorch)),
				.heightBackPorch = (uint16_t)(((uint16_t)td.vertical_blanking_lines_msb << 8 | td.vertical_blanking_lines_lsb) - (heightSync + heightFrontPorch))
			});
		}
		*/
	}
	#ifdef DEBUG
	printf("\n");
	#endif
	uint8_t extension_count = 0;
	memcpy(&extension_count, &edid[offsetof(SetSysEdid, extension_count)], 1);
	if (extension_count != 1) {
		#ifdef DEBUG
		printf("More than one extension count!\n");
		#endif
	}
	uint8_t extension_type = 0;
	memcpy(&extension_type, &edid[offsetof(SetSysEdid, extension_tag)], 1);
	if (extension_type != 2) {
		#ifdef DEBUG
		printf("Wrong extension type!\n");
		#endif
		return 0;
	}
	uint8_t dtd_start = 0;
	memcpy(&dtd_start, &edid[offsetof(SetSysEdid, dtd_start)], 1);
	if (dtd_start) {
		uint8_t native_dtd_count = 0;
		memcpy(&native_dtd_count, &edid[offsetof(SetSysEdid, dtd_start) + 1], 1);
		native_dtd_count &= 0b1111;
		SetSysDataBlock* data = (SetSysDataBlock*)calloc(sizeof(SetSysDataBlock), 1);
		size_t offset = offsetof(SetSysEdid, data_block);
		memcpy(data, &edid[offset], sizeof(SetSysDataBlock));
		if (data -> video.block_type != SetSysBlockType_Video) {
			while (offset < (size_t)offsetof(SetSysEdid, extension_tag)+dtd_start) {
				offset += 1;
				offset += data -> video.size;
				memcpy(data, &edid[offset], sizeof(SetSysDataBlock));
				if (data -> video.block_type == SetSysBlockType_Video) break;
			}
		}
		if (data -> video.block_type == SetSysBlockType_Video) {
			for (size_t i = 0; i < data -> video.size; i++) {
				uint8_t index = data->video.svd[i].svd_index;
				if (index > 64) memcpy(&index, &data->video.svd[i], 1);
				const timings* timing = find_vic_id(index);
				double refreshRate = (double)(timing->pixclk_khz * 1000) / ((timing->hact + timing->hfp + timing->hsync + timing->hbp) * ((timing->vact / (timing->interlaced ? 2 : 1)) + timing->vfp + timing->vsync + timing->vbp));
				#ifdef DEBUG
				printf("VIC: %u%s, Res: %ux%u%s, refresh rate: %.4f\n", index, ((index <= 64 && data -> video.svd[i].native_flag) ? " (native)" : ""), timing->hact, timing->vact, (timing->interlaced ? "i" : ""), refreshRate);
				#endif
				if (!timing->interlaced && refreshRate > highestRefreshRate) highestRefreshRate = refreshRate;
				/*
				if (!timing->interlaced && ((timing->hact == 1280 && timing->vact == 720) || (timing->hact == 1920 && timing->vact == 1080)) && (refreshRate > 60)) {
					EdidData.push_back({
						.pixelClockkHz = timing->pixclk_khz,
						.width = (uint16_t)timing->hact,
						.height = (uint16_t)timing->vact,
						.refreshRate = (float)refreshRate,
						.widthFrontPorch = (uint16_t)timing->vfp,
						.heightFrontPorch = (uint16_t)timing->hfp,
						.widthSync = (uint16_t)timing->hsync,
						.heightSync = (uint16_t)timing->vsync,
						.widthBackPorch = (uint16_t)timing->hbp,
						.heightBackPorch = (uint16_t)timing->vbp
					});
				}
				*/
			}
			#ifdef DEBUG
			printf("\n");
			#endif
		}
		free(data);
		SetSysModeLine* modeline = (SetSysModeLine*)calloc(sizeof(SetSysModeLine), 5);
		memcpy(modeline, &edid[offsetof(SetSysEdid, extension_tag)+dtd_start], sizeof(SetSysModeLine) * 5);
		for (size_t i = 0; i < 5; i++) {
			SetSysModeLine td = modeline[i];
			uint32_t width = (uint32_t)td.horizontal_active_pixels_msb << 8 | td.horizontal_active_pixels_lsb;
			uint32_t height = (uint32_t)td.vertical_active_lines_msb << 8 | td.vertical_active_lines_lsb;
			if (width <= 1 || height <= 1) continue;
			uint32_t h_total = width + ((uint32_t)td.horizontal_blanking_pixels_msb << 8 | td.horizontal_blanking_pixels_lsb);
			uint32_t v_total = height + ((uint32_t)td.vertical_blanking_lines_msb << 8 | td.vertical_blanking_lines_lsb);
			float refreshRate = ((float)(td.pixel_clock * 10000) / (float)(h_total * v_total));
			#ifdef DEBUG
			if (td.pixel_clock) printf("Res: %dx%d, pixel clock: %d, refresh rate: %0.4f\n", width, height, td.pixel_clock * 10, refreshRate);
			#endif
			if (!td.interlaced && refreshRate > highestRefreshRate) highestRefreshRate = refreshRate;
			/*
			if (!td.interlaced && ((width == 1280 && height == 720) || (width == 1920 && height == 1080)) && (refreshRate > 60)) {
				uint16_t widthSync = (uint16_t)td.horizontal_sync_pulse_width_pixels_msb << 8 | td.horizontal_sync_pulse_width_pixels_lsb;
				uint16_t heightSync = (uint16_t)td.vertical_sync_pulse_width_lines_msb << 8 | td.vertical_sync_pulse_width_lines_lsb;
				uint16_t widthFrontPorch = (uint16_t)td.horizontal_sync_offset_pixels_msb << 8 | td.horizontal_sync_offset_pixels_lsb;
				uint16_t heightFrontPorch = (uint16_t)td.horizontal_sync_offset_pixels_msb << 8 | td.horizontal_sync_offset_pixels_lsb;
				EdidData.push_back({
				.pixelClockkHz = (uint32_t)td.pixel_clock * 10,
				.width = (uint16_t)width,
				.height = (uint16_t)height,
				.refreshRate = refreshRate,
				.widthFrontPorch = widthFrontPorch,
				.heightFrontPorch = heightFrontPorch,
				.widthSync = widthSync,
				.heightSync = heightSync,
				.widthBackPorch = (uint16_t)(((uint16_t)td.horizontal_blanking_pixels_msb << 8 | td.horizontal_blanking_pixels_lsb) - (widthSync + widthFrontPorch)),
				.heightBackPorch = (uint16_t)(((uint16_t)td.vertical_blanking_lines_msb << 8 | td.vertical_blanking_lines_lsb) - (heightSync + heightFrontPorch))
				});
			}
			*/
		}
		free(modeline);
	}
	return highestRefreshRate;
}

void SaveDockedModeAllowedSave(DockedModeRefreshRateAllowed rr, DockedAdditionalSettings &as) {
	tsl::hlp::doWithSmSession([]{
		setsysInitialize();
	});
    SetSysEdid edid = {0};
    if (R_FAILED(setsysGetEdid(&edid))) {
		return;
    }
    char path[128] = "";
    snprintf(path, sizeof(path), "sdmc:/SaltySD/plugins/FPSLocker/ExtDisplays/%08X.ini", crc32Calculate(&edid, sizeof(edid)));
    FILE* file = fopen(path, "w");
    if (file) {
		std::string allowedRR = "{";
        for (size_t i = 0; i < sizeof(DockedModeRefreshRateAllowed); i++) {
			if (rr[i]) {
				allowedRR += std::to_string(DockedModeRefreshRateAllowedValues[i]);
				allowedRR += ",";
			}
        }
		allowedRR.erase(allowedRR.end()-1);
		allowedRR += "}";
		fwrite("[Common]\n", 9, 1, file);
		fwrite("tvName=", 7, 1, file);
		fwrite(TV_name.c_str(), TV_name.length(), 1, file);
		fwrite("\n", 1, 1, file);
		fwrite("refreshRateAllowed=", 19, 1, file);
		fwrite(allowedRR.c_str(), allowedRR.length(), 1, file);
		fwrite("\n", 1, 1, file);
		std::string df60 = (as.dontForce60InDocked ? "False" : "True");
		std::string fpst = (as.fpsTargetWithoutRRMatchLowest ? "True" : "False");
		fwrite("allowPatchesToForce60InDocked=", 30, 1, file);
		fwrite(df60.c_str(), df60.length(), 1, file);
		fwrite("\n", 1, 1, file);
		fwrite("matchLowestRefreshRate=", 23, 1, file);
		fwrite(fpst.c_str(), fpst.length(), 1, file);
		fwrite("\n", 1, 1, file);
        fclose(file);

    }
    return;
}

void LoadDockedModeAllowedSave(DockedModeRefreshRateAllowed &rr, DockedAdditionalSettings &as, int* displayCRC) {
	for (size_t i = 0; i < sizeof(DockedModeRefreshRateAllowed); i++) {
		if (DockedModeRefreshRateAllowedValues[i] == 60 || DockedModeRefreshRateAllowedValues[i] == 50) rr[i] = true;
		else rr[i] = false;
	}
	as.dontForce60InDocked = false;
	as.fpsTargetWithoutRRMatchLowest = false;
	TV_name = "Unknown";
	tsl::hlp::doWithSmSession([]{
		setsysInitialize();
	});
    SetSysEdid edid = {0};
    if (R_FAILED(setsysGetEdid(&edid))) {
		return;
    }
    char path[128] = "";
	int crc32 = crc32Calculate(&edid, sizeof(edid));
	if (displayCRC) *displayCRC = crc32;
    snprintf(path, sizeof(path), "sdmc:/SaltySD/plugins/FPSLocker/ExtDisplays/%08X.ini", crc32);
    if (file_exists(path) == true) {
		FILE* file = fopen(path, "r");
		fseek(file, 0, 2);
		size_t size = ftell(file);
		fseek(file, 0, 0);
		std::string string_data(size, 0);
		fread(string_data.data(), size, 1, file);
		fclose(file);
		tsl::hlp::ini::IniData iniData = tsl::hlp::ini::parseIni(string_data);
		if (iniData.contains("Common") == false) {
			return;
		}
		if (iniData["Common"].contains("tvName") == true) {
			TV_name = iniData["Common"]["tvName"];
		}
		if (iniData["Common"].contains("refreshRateAllowed") == false) {
			return;
		}
		if (iniData["Common"]["refreshRateAllowed"].begin()[0] != '{')
			return;
		if ((iniData["Common"]["refreshRateAllowed"].end()-1)[0] != '}')
			return;
		std::string rrAllowed = std::string(iniData["Common"]["refreshRateAllowed"].begin()+1, iniData["Common"]["refreshRateAllowed"].end()-1);
		for (const auto& word_view : std::views::split(rrAllowed, ',')) {
			std::string temp_string(word_view.begin(), word_view.end());
			int value = 0;
			auto [ptr, ec] = std::from_chars(temp_string.c_str(), &temp_string.c_str()[temp_string.length()], value);
			if (ec != std::errc{}) return;
			for (size_t i = 0; i < sizeof(DockedModeRefreshRateAllowedValues); i++) {
				if (value == DockedModeRefreshRateAllowedValues[i]) {
					rr[i] = true;
					break;
				}
			}
		}
		if (iniData["Common"].contains("allowPatchesToForce60InDocked") == true) {
			as.dontForce60InDocked = (bool)!strncasecmp(iniData["Common"]["allowPatchesToForce60InDocked"].c_str(), "False", 5);
		}
		if (iniData["Common"].contains("matchLowestRefreshRate") == true) {
			as.fpsTargetWithoutRRMatchLowest = (bool)!strncasecmp(iniData["Common"]["matchLowestRefreshRate"].c_str(), "True", 4);
		}
    }
    return;
}

void downloadPatch(void*) {

	Result temp_error_code = -1;

	curl_timeout = false;

    static const SocketInitConfig socketInitConfig = {

        .tcp_tx_buf_size = 0x8000,
        .tcp_rx_buf_size = 0x8000,
        .tcp_tx_buf_max_size = 0x20000,
        .tcp_rx_buf_max_size = 0x20000,

        .udp_tx_buf_size = 0,
        .udp_rx_buf_size = 0,

        .sb_efficiency = 1,
		.bsd_service_type = BsdServiceType_Auto
    };

	uint64_t startTick = svcGetSystemTick();
	uint64_t timeoutTick = startTick + (30 * 19'200'000); //30 seconds
	long msPeriod = (timeoutTick - svcGetSystemTick()) / 19200;

	smInitialize();


	nifmInitialize(NifmServiceType_System);
	u32 dummy = 0;
	NifmInternetConnectionType NifmConnectionType = (NifmInternetConnectionType)-1;
	NifmInternetConnectionStatus NifmConnectionStatus = (NifmInternetConnectionStatus)-1;
	if (R_FAILED(nifmGetInternetConnectionStatus(&NifmConnectionType, &dummy, &NifmConnectionStatus)) || NifmConnectionStatus != NifmInternetConnectionStatus_Connected) {
		nifmExit();
		smExit();
		error_code = 0x412;
		return;
	}
	nifmExit();

	socketInitialize(&socketInitConfig);

    curl_global_init(CURL_GLOBAL_DEFAULT);
	CURL *curl = curl_easy_init();

    if (curl) {

		char download_path[256] = "";
		char file_path[192] = "";
		snprintf(download_path, sizeof(download_path), "sdmc:/SaltySD/plugins/FPSLocker/patches/%016lX/", TID);
		
		std::filesystem::create_directories(download_path);

		snprintf(file_path, sizeof(file_path), "sdmc:/SaltySD/plugins/FPSLocker/patches/%016lX/temp.yaml", TID);

		FILE* fp = fopen(file_path, "wb+");
		if (!fp) {
			curl_easy_cleanup(curl);
			curl_global_cleanup();
			socketExit();
			smExit();
			error_code = 0x101;
			return;
		}

		snprintf(download_path, sizeof(download_path), "https://raw.githubusercontent.com/masagrator/FPSLocker-Warehouse/v3/SaltySD/plugins/FPSLocker/patches/%016lX/%016lX.yaml", TID, BID);
        curl_easy_setopt(curl, CURLOPT_URL, download_path);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_NOBODY, 0L);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		msPeriod = (timeoutTick - svcGetSystemTick()) / 19200;
		curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, msPeriod);

        CURLcode res = curl_easy_perform(curl);

		if (res != CURLE_OK) {
			fclose(fp);
			remove(file_path);
			if (res == CURLE_OPERATION_TIMEDOUT) temp_error_code = 0x316;
			else temp_error_code = 0x200 + res;
		}
		else {
			size_t filesize = ftell(fp);
			if (filesize > 512) {
				filesize = 512;
			}
			fseek(fp, 0, SEEK_SET);
			char* buffer = (char*)calloc(1, filesize + 1);
			fread(buffer, 1, filesize, fp);
			fclose(fp);
			char BID_char[18] = "";
			snprintf(BID_char, sizeof(BID_char), " %016lX", BID);
			if (std::search(&buffer[0], &buffer[filesize], &BID_char[0], &BID_char[17]) == &buffer[filesize]) {
				remove(file_path);
				char Not_found[] = "404: Not Found";
				if (!strncmp(buffer, Not_found, strlen(Not_found))) {
					temp_error_code = 0x404;
				}
				else temp_error_code = 0x312;
			}
			else temp_error_code = 0;
			free(buffer);
		}

		if (!temp_error_code) {
			fp = fopen(file_path, "rb");
			fseek(fp, 0, SEEK_END);
			size_t filesize1 = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			char* buffer1 = (char*)calloc(1, filesize1 + 1);
			fread(buffer1, 1, filesize1, fp);
			fclose(fp);
			fp = fopen(configPath, "rb");
			if (fp) {
				fseek(fp, 0, SEEK_END);
				size_t filesize2 = ftell(fp);
				fseek(fp, 0, SEEK_SET);
				if (filesize2 != filesize1) {
					fclose(fp);
					free(buffer1);
					FileDownloaded = true;
				}
				else {
					char* buffer2 = (char*)calloc(1, filesize2 + 1);
					fread(buffer2, 1, filesize2, fp);
					fclose(fp);
					if (memcmp(buffer1, buffer2, filesize1)) {
						FileDownloaded = true;
					}
					else {
						temp_error_code = 0x104;
						remove(file_path);
					}
					free(buffer1);
					free(buffer2);
				}
			}
			else {
				free(buffer1);
				FileDownloaded = true;
			}
			if (!temp_error_code) {
				remove(configPath);
				rename(file_path, configPath);
				FILE* config = fopen(configPath, "r");
				memset(&LOCK::configBuffer, 0, sizeof(LOCK::configBuffer));
				fread(&LOCK::configBuffer, 1, 32768, config);
				fclose(config);
				strcat(&LOCK::configBuffer[0], "\n");
				LOCK::tree = ryml::parse_in_place(LOCK::configBuffer);
				size_t root_id = LOCK::tree.root_id();
				if (LOCK::tree.is_map(root_id) && LOCK::tree.find_child(root_id, "Addons") != c4::yml::NONE && !LOCK::tree["Addons"].is_keyval() && LOCK::tree["Addons"].num_children() > 0) {
					for (size_t i = 0; i < LOCK::tree["Addons"].num_children(); i++) {
						std::string temp = "";
						LOCK::tree["Addons"][i] >> temp;
						std::string dpath = "https://raw.githubusercontent.com/masagrator/FPSLocker-Warehouse/v3/" + temp;
						std::string path = "sdmc:/" + temp;
						strncpy(&download_path[0], dpath.c_str(), 255);
						strncpy(&file_path[0], path.c_str(), 191);
						curl_easy_setopt(curl, CURLOPT_URL, download_path);
						msPeriod = (timeoutTick - svcGetSystemTick()) / 19200;
						curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, msPeriod);
						FILE* fp = fopen(file_path, "wb");
						if (!fp) {
							std::filesystem::create_directories(std::filesystem::path(file_path).parent_path());
							fp = fopen(file_path, "wb");
						}
						if (fp) {
							curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
							res = curl_easy_perform(curl);
							fclose(fp);
						}
					}
				}
			}
		}
		else if (temp_error_code == 0x404) {
			error_code = 0x404;
			curl_easy_setopt(curl, CURLOPT_URL, "https://raw.githubusercontent.com/masagrator/FPSLocker-Warehouse/v3/README.md");
			fp = fopen("sdmc:/SaltySD/plugins/FPSLocker/patches/README.md", "wb+");
			if (!fp) {
				curl_easy_cleanup(curl);
				curl_global_cleanup();
				socketExit();
				smExit();
				error_code = 0x101;
				return;
			}
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
			msPeriod = (timeoutTick - svcGetSystemTick()) / 19200;
			if (msPeriod < 1000) msPeriod = 1000;
			curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, msPeriod);
			CURLcode res = curl_easy_perform(curl);
			if (res == CURLE_OK) {
				size_t filesize = ftell(fp);
				fseek(fp, 0, SEEK_SET);
				char* buffer = (char*)calloc(1, filesize + 1);
				fread(buffer, 1, filesize, fp);
				fclose(fp);
				remove("sdmc:/SaltySD/plugins/FPSLocker/patches/README.md");
				char findText_char[] = "# FPSLocker Warehouse";
				char BID_search[] = "`1234567890ABCDEF` (◯";
				if (!strncmp(buffer, findText_char, strlen(findText_char))) {
					snprintf(BID_search, sizeof(BID_search), "`%016lX`", TID);
					auto start = std::search(&buffer[0], &buffer[filesize], &BID_search[0], &BID_search[strlen(BID_search)]);
					if (start == &buffer[filesize]) {
						temp_error_code = 0x1002;
					}
					else {
						strcpy(BID_search, ") |");
						auto end = std::search(start, &buffer[filesize], &BID_search[0], &BID_search[3]);
						snprintf(BID_search, sizeof(BID_search), "`%016lX` (◯", BID);
						if (std::search(start, end, &BID_search[0], &BID_search[strlen(BID_search)]) != end) {
							temp_error_code = 0x1001;
						}
						else {
							snprintf(BID_search, sizeof(BID_search), "`%016lX` (", BID);
							if (std::search(start, end, &BID_search[0], &BID_search[strlen(BID_search)]) == end) {
								strcpy(BID_search, " (");
								auto found = std::find_end(start, end, &BID_search[0], &BID_search[2]);
								found += 2;
								if (strncmp("◯", found, strlen("◯")) == 0) {
									temp_error_code = 0x1003;
								}
								else if (strncmp("❌", found, strlen("❌")) == 0) {
									temp_error_code = 0x1004;
								}
								else if (strncmp("[", found, strlen("[")) == 0) {
									temp_error_code = 0x1005;
								}
							}
							else {
								snprintf(BID_search, sizeof(BID_search), "`%016lX` (❌", BID);
								if (std::search(start, end, &BID_search[0], &BID_search[strlen(BID_search)]) != end) {
									temp_error_code = 0x1006;
								}						
							}	
						}
					}
				}
				free(buffer);
			}
			else if (res == CURLE_OPERATION_TIMEDOUT) {
				temp_error_code = 0x405;
			}
			else temp_error_code = 0x406;
		}

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
	socketExit();
	smExit();
	error_code = temp_error_code;
	return;
}

void loopThread(void*) {
	while(threadActive) {
		if (R_FAILED(pmdmntGetApplicationProcessId(&PID))) break;
		svcSleepThread(1'000'000'000);
	}
	PluginRunning = false;
	check = false;
	closed = true;
}

uint64_t checkFile(const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file)
        return 0x202;
    fclose(file);
    return 0;
}

uint64_t getBID() {
	u64 BID_temp = 0;

	if (R_SUCCEEDED(ldrDmntInitialize())) {
		LoaderModuleInfo* module_infos = (LoaderModuleInfo*)malloc(sizeof(LoaderModuleInfo) * 16);
		s32 module_infos_count = 0;
		Result ret = ldrDmntGetProcessModuleInfo(PID, module_infos, 16, &module_infos_count);
		ldrDmntExit();

		if (R_SUCCEEDED(ret)) {
			for (int itr = 0; itr < module_infos_count; itr++) {
				static u64 comp_address = 0;
				if (!comp_address) {
					comp_address = module_infos[itr].base_address;
					continue;
				}
				if ((module_infos[itr].base_address - comp_address == 0x4000) || (module_infos[itr].base_address - comp_address == 0x6000) || (module_infos[itr].base_address - comp_address == 0x5000)) {
					for (int itr2 = 0; itr2 < 8; itr2++) {
						*(uint8_t*)((uint64_t)&BID_temp+itr2) = module_infos[itr].build_id[itr2];
					}
					BID_temp = __builtin_bswap64(BID_temp);
					itr = module_infos_count;
				}
				else comp_address = module_infos[itr].base_address;
			}
		}
		free(module_infos);
	}

	return BID_temp;
}

bool LoadSharedMemory() {
	if (SaltySD_Connect())
		return false;

	SaltySD_GetSharedMemoryHandle(&remoteSharedMemory);
	SaltySD_Term();

	shmemLoadRemote(&_sharedmemory, remoteSharedMemory, 0x1000, Perm_Rw);
	if (!shmemMap(&_sharedmemory)) {
		SharedMemoryUsed = true;
		return true;
	}
	return false;
}

ptrdiff_t searchSharedMemoryBlock(uintptr_t base) {
	ptrdiff_t search_offset = 0;
	while(search_offset < 0x1000) {
		uint32_t* MAGIC_shared = (uint32_t*)(base + search_offset);
		if (*MAGIC_shared == 0x465053) {
			return search_offset;
		}
		else search_offset += 4;
	}
	return -1;
}

bool CheckPort () {
	Handle saltysd;
	for (int i = 0; i < 67; i++) {
		if (R_SUCCEEDED(svcConnectToNamedPort(&saltysd, "InjectServ"))) {
			svcCloseHandle(saltysd);
			break;
		}
		else {
			if (i == 66) return false;
			svcSleepThread(1'000'000);
		}
	}
	for (int i = 0; i < 67; i++) {
		if (R_SUCCEEDED(svcConnectToNamedPort(&saltysd, "InjectServ"))) {
			svcCloseHandle(saltysd);
			return true;
		}
		else svcSleepThread(1'000'000);
	}
	return false;
}

std::string getAppName(uint64_t Tid)
{
	NsApplicationControlData appControlData;
	size_t appControlDataSize = 0;
	if (R_FAILED(nsGetApplicationControlData(NsApplicationControlSource::NsApplicationControlSource_Storage, Tid, &appControlData, sizeof(NsApplicationControlData), &appControlDataSize))) {
		char returnTID[18];
		sprintf(returnTID, "%016lx-", Tid);
		return (std::string)returnTID;
	}
	
	NacpLanguageEntry *languageEntry = nullptr;
	Result rc = nsGetApplicationDesiredLanguage(&appControlData.nacp, &languageEntry);
	if (R_FAILED(rc)) {
		char returnTID[18];
		sprintf(returnTID, "0x%X", rc);
		return (std::string)returnTID;
	}
	
	return std::string(languageEntry->name);
}

Result getTitles(int32_t count)
{
  NsApplicationRecord appRecords = {};
  int32_t actualAppRecordCnt = 0;
  Result rc;

  while (1)
  {
    static int32_t offset = 0;
    rc = nsListApplicationRecord(&appRecords, 1, offset, &actualAppRecordCnt);
    if (R_FAILED(rc) || (actualAppRecordCnt < 1) || (offset >= count)) break;
    if (appRecords.application_id != 0) {
        Title title;
        title.TitleID = appRecords.application_id;
        title.TitleName = getAppName(appRecords.application_id);
        titles.push_back(title);
    }
    offset++;
    appRecords = {};
  }
  return rc;
}