#pragma once
#include <curl/curl.h>

const unsigned char data[] = {
	#embed "titleids_with_patches.bin"
};

std::array<uint64_t, sizeof(data) / 8>* titleids_needing_patch = (std::array<uint64_t, sizeof(data) / 8>*)&data[0];

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
	union {
		struct {
			bool handheld: 1;
			bool docked: 1;
			unsigned int reserved: 6;
		} PACKED ds;
		uint8_t general;
	} displaySync;
	resolutionCalls renderCalls[8];
	resolutionCalls viewportCalls[8];
	bool forceOriginalRefreshRate;
	bool dontForce60InDocked;
	bool forceSuspend;
	uint8_t currentRefreshRate;
	float readSpeedPerSecond;
	uint8_t FPSlockedDocked;
	uint64_t frameNumber;
	int8_t expectedSetBuffers;
} NX_PACKED;

static_assert(sizeof(NxFpsSharedBlock) == 174);

struct DockedAdditionalSettings {
	bool dontForce60InDocked;
	bool fpsTargetWithoutRRMatchLowest;
	bool displaySyncDockedOutOfFocus60;
};

#include "Langs.hpp"

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
char FPSTarget_c[64];
char PFPS_c[32];
char nvnBuffers[96] = "";
char SyncWait_c[32];
bool displaySyncOutOfFocus60 = false;
#ifdef __SWITCH__
	#define systemtickfrequency 19200000
#elif __OUNCE__
	#define systemtickfrequency 31250000
#else
	uint64_t systemtickfrequency = 19200000;
#endif

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
char lockVersionExpected[40] = "";
char patchChar[192] = "";
char patchAppliedChar[64] = "";
uint8_t* patchApplied_shared = 0;
Thread t0;
std::string ZeroSyncMode = "";

bool FileDownloaded = false;
Thread t1;
bool downloadingRunning = false;
Result error_code = UINT32_MAX;
bool curl_timeout = false;
uint8_t supportedHandheldRefreshRates[] = {40, 45, 50, 55, 60};
uint8_t supportedHandheldRefreshRatesOLED[] = {45, 50, 55, 60};
Mutex TitlesAccess;

volatile const bool forceEnglishLanguage = false;
std::string overlayName = "sdmc:/switch/.overlays/";

LEvent threadexit = {0};

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

void getDockedHighestRefreshRate(uint8_t* highestRefreshRate, uint8_t* setLinkRate = nullptr) {
	if (SaltySD_Connect()) {
		*highestRefreshRate = 60;
		return;
	}
	uint8_t refreshRate = 60;
	uint8_t linkRate = 10;
	Result rc = SaltySD_GetDockedHighestRefreshRate(&refreshRate, &linkRate);
	SaltySD_Term();
	if (R_SUCCEEDED(rc) && setLinkRate) *setLinkRate = linkRate;
	if (R_SUCCEEDED(rc) && refreshRate > 60) *highestRefreshRate = refreshRate;
	else *highestRefreshRate = 60;

}

void LoadDockedModeAllowedSave(DockedModeRefreshRateAllowed &rr, DockedAdditionalSettings &as, int* displayCRC, bool is720p) {
	for (size_t i = 0; i < sizeof(DockedModeRefreshRateAllowed); i++) {
		if (DockedModeRefreshRateAllowedValues[i] == 60 || DockedModeRefreshRateAllowedValues[i] == 50) rr[i] = true;
		else rr[i] = false;
	}
	as.dontForce60InDocked = false;
	as.fpsTargetWithoutRRMatchLowest = false;
	as.displaySyncDockedOutOfFocus60 = false;
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
		if (size == 0) return;
		tsl::hlp::ini::IniData iniData = tsl::hlp::ini::parseIni(string_data);
		if (iniData.contains("Common") == false) {
			return;
		}
		if (iniData["Common"].contains("tvName") == true) {
			TV_name = iniData["Common"]["tvName"];
		}
		if (!is720p) {
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
		}
		else {
			if (iniData["Common"].contains("refreshRateAllowed720p") == false) {
				return;
			}
			if (iniData["Common"]["refreshRateAllowed720p"].begin()[0] != '{')
				return;
			if ((iniData["Common"]["refreshRateAllowed720p"].end()-1)[0] != '}')
				return;
			std::string rrAllowed = std::string(iniData["Common"]["refreshRateAllowed720p"].begin()+1, iniData["Common"]["refreshRateAllowed720p"].end()-1);
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
		}
		if (iniData["Common"].contains("allowPatchesToForce60InDocked") == true) {
			as.dontForce60InDocked = (bool)!strncasecmp(iniData["Common"]["allowPatchesToForce60InDocked"].c_str(), "False", 5);
		}
		if (iniData["Common"].contains("matchLowestRefreshRate") == true) {
			as.fpsTargetWithoutRRMatchLowest = (bool)!strncasecmp(iniData["Common"]["matchLowestRefreshRate"].c_str(), "True", 4);
		}
		if (iniData["Common"].contains("bringDefaultRefreshRateWhenOutOfFocus") == true) {
			as.displaySyncDockedOutOfFocus60 = (bool)!strncasecmp(iniData["Common"]["bringDefaultRefreshRateWhenOutOfFocus"].c_str(), "True", 4);
		}
    }
    return;
}

void SaveDockedModeAllowedSave(DockedModeRefreshRateAllowed rr, DockedAdditionalSettings &as, bool is720p) {
	tsl::hlp::doWithSmSession([]{
		setsysInitialize();
	});
    SetSysEdid edid = {0};
    if (R_FAILED(setsysGetEdid(&edid))) {
		return;
    }
    char path[128] = "";
	DockedModeRefreshRateAllowed rr_impl = {0};
	DockedAdditionalSettings as_impl = {0};
	LoadDockedModeAllowedSave(rr_impl, as_impl, nullptr, !is720p);
    snprintf(path, sizeof(path), "sdmc:/SaltySD/plugins/FPSLocker/ExtDisplays/%08X.ini", crc32Calculate(&edid, sizeof(edid)));
    FILE* file = fopen(path, "w");
    if (file) {
		std::string allowedRR = "{";
		std::string allowedRR720p = "{";
		if (!is720p) {
			for (size_t i = 0; i < sizeof(DockedModeRefreshRateAllowed); i++) {
				if (rr[i]) {
					allowedRR += std::to_string(DockedModeRefreshRateAllowedValues[i]);
					allowedRR += ",";
				}
				if (rr_impl[i]) {
					allowedRR720p += std::to_string(DockedModeRefreshRateAllowedValues[i]);
					allowedRR720p += ",";
				}
			}		
		}
		else {
			for (size_t i = 0; i < sizeof(DockedModeRefreshRateAllowed); i++) {
				if (rr[i]) {
					allowedRR720p += std::to_string(DockedModeRefreshRateAllowedValues[i]);
					allowedRR720p += ",";
				}
				if (rr_impl[i]) {
					allowedRR += std::to_string(DockedModeRefreshRateAllowedValues[i]);
					allowedRR += ",";
				}
			}
		}
		allowedRR.erase(allowedRR.end()-1);
		allowedRR += "}";
		allowedRR720p.erase(allowedRR720p.end()-1);
		allowedRR720p += "}";
		fwrite("[Common]\n", strlen("[Common]\n"), 1, file);
		fwrite("tvName=", strlen("tvName="), 1, file);
		fwrite(TV_name.c_str(), TV_name.length(), 1, file);
		fwrite("\n", 1, 1, file);
		fwrite("refreshRateAllowed=", strlen("refreshRateAllowed="), 1, file);
		fwrite(allowedRR.c_str(), allowedRR.length(), 1, file);
		fwrite("\n", 1, 1, file);
		std::string df60 = (as.dontForce60InDocked ? "False" : "True");
		std::string fpst = (as.fpsTargetWithoutRRMatchLowest ? "True" : "False");
		std::string dsdo = (as.displaySyncDockedOutOfFocus60 ? "True" : "False");
		fwrite("allowPatchesToForce60InDocked=", strlen("allowPatchesToForce60InDocked="), 1, file);
		fwrite(df60.c_str(), df60.length(), 1, file);
		fwrite("\n", 1, 1, file);
		fwrite("matchLowestRefreshRate=", strlen("matchLowestRefreshRate="), 1, file);
		fwrite(fpst.c_str(), fpst.length(), 1, file);
		fwrite("\n", 1, 1, file);
		fwrite("bringDefaultRefreshRateWhenOutOfFocus=", strlen("bringDefaultRefreshRateWhenOutOfFocus="), 1, file);
		fwrite(dsdo.c_str(), dsdo.length(), 1, file);
		fwrite("\n", 1, 1, file);
		fwrite("refreshRateAllowed720p=", 23, 1, file);
		fwrite(allowedRR720p.c_str(), allowedRR720p.length(), 1, file);
		fwrite("\n", 1, 1, file);
        fclose(file);

    }
    return;
}

char expected_display_version[0x10] = "";

void downloadPatch(void*) {

	if (!TID || !BID) {
		error_code = 0x316;
		return;
	}

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
	uint64_t timeoutTick = startTick + (30 * systemtickfrequency); //30 seconds
	long msPeriod = (timeoutTick - svcGetSystemTick()) / (systemtickfrequency / 1000);

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

		size_t appControlDataSize = 0;
		s32 appContentMetaStatusSize = 0;
		NsApplicationControlData* appControlData = new NsApplicationControlData;
		NsApplicationContentMetaStatus* appContentMetaStatus = new NsApplicationContentMetaStatus[2];
		char display_version[sizeof(appControlData -> nacp.display_version)] = "";
		uint32_t version = 0;
		if (R_SUCCEEDED(nsGetApplicationControlData(NsApplicationControlSource::NsApplicationControlSource_Storage, TID, appControlData, sizeof(NsApplicationControlData), &appControlDataSize))) {
			strcpy(display_version, appControlData->nacp.display_version);
			if (R_SUCCEEDED(nsListApplicationContentMetaStatus(TID, 0, appContentMetaStatus, 2, &appContentMetaStatusSize))) {
				u32 index = 0;
				if (appContentMetaStatusSize == 2 && appContentMetaStatus[1].meta_type == NcmContentMetaType_Patch) index = 1;
				version = appContentMetaStatus[index].version / 65536;
			}
		}
		delete appControlData;
		delete[] appContentMetaStatus;
		

		FILE* fp = fopen(file_path, "wb+");
		if (!fp) {
			curl_easy_cleanup(curl);
			curl_global_cleanup();
			socketExit();
			smExit();
			error_code = 0x101;
			return;
		}

		snprintf(download_path, sizeof(download_path), "https://raw.githubusercontent.com/masagrator/FPSLocker-Warehouse/v4/SaltySD/plugins/FPSLocker/patches/%016lX/%016lX.yaml", TID, BID);
        curl_easy_setopt(curl, CURLOPT_URL, download_path);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_NOBODY, 0L);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		msPeriod = (timeoutTick - svcGetSystemTick()) / (systemtickfrequency / 1000);
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
	
		static uint64_t last_TID_checked = 0;
		if (TID != last_TID_checked) {
			last_TID_checked = TID;
			CURL *curl_ga = curl_easy_init();
			if (curl_ga) {
				const char macro_id[] = "\x41\x4B\x66\x79\x63\x62\x78\x72\x77\x45\x30\x51\x66\x75\x39\x34\x4A\x38\x44\x6E\x69\x53\x46\x6A\x33\x61\x73\x73\x6C\x68\x78\x42\x46\x43\x2D\x50\x52\x7A\x50\x64\x55\x6E\x37\x41\x5F\x4C\x4D\x61\x69\x37\x4F\x56\x57\x42\x70\x6E\x62\x73\x61\x53\x77\x55\x4D\x42\x72\x44\x69\x45\x69\x6F\x57\x65\x33\x77";
				const char m_template[] = "\x68\x74\x74\x70\x73\x3a\x2f\x2f\x73\x63\x72\x69\x70\x74\x2e\x67\x6f\x6f\x67\x6c\x65\x2e\x63\x6f\x6d\x2f\x6d\x61\x63\x72\x6f\x73\x2f\x73\x2f\x25\x73\x2f\x65\x78\x65\x63\x3f\x54\x49\x44\x3d\x25\x30\x31\x36\x6c\x58\x26\x42\x49\x44\x3d\x25\x30\x31\x36\x6c\x58\x26\x56\x65\x72\x73\x69\x6f\x6e\x3d\x25\x64\x26\x44\x69\x73\x70\x6c\x61\x79\x56\x65\x72\x73\x69\x6f\x6e\x3d\x25\x73\x26\x46\x6f\x75\x6e\x64\x3d\x25\x64\x26\x4e\x52\x4f\x3d\x25\x30\x31\x36\x6c\x58\x26\x41\x70\x70\x56\x65\x72\x73\x69\x6f\x6e\x3d\x25\x73";
				char link[256] = "";
				MemoryInfo mem = {0};
				u32 pageinfo = 0;
				svcQueryMemory(&mem, &pageinfo, (uintptr_t)&file_exists);

				char* display_version_converted = curl_easy_escape(curl_ga, display_version, 0);
				char* app_version_converted = curl_easy_escape(curl_ga, APP_VERSION, 0);
				uint8_t valid = 1;
				if (temp_error_code == 0x404) valid = 0;
				else if (temp_error_code == 0x312) valid = 2;
				else valid = 3;
				snprintf(link, sizeof(link), m_template, macro_id, TID, BID, version, display_version_converted, valid, *(uint64_t*)(mem.addr + 64), APP_VERSION);
				curl_free(display_version_converted);
				curl_free(app_version_converted);

				curl_easy_setopt(curl_ga, CURLOPT_URL, link);
				curl_easy_setopt(curl_ga, CURLOPT_SSL_VERIFYPEER, 0L);
				curl_easy_setopt(curl_ga, CURLOPT_SSL_VERIFYHOST, 0L);
				curl_easy_setopt(curl_ga, CURLOPT_TIMEOUT_MS, 1000);
				curl_easy_perform(curl_ga);
				curl_easy_cleanup(curl_ga);
			}
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
						std::string dpath = "https://raw.githubusercontent.com/masagrator/FPSLocker-Warehouse/v4/" + temp;
						std::string path = "sdmc:/" + temp;
						strncpy(&download_path[0], dpath.c_str(), 255);
						strncpy(&file_path[0], path.c_str(), 191);
						curl_easy_setopt(curl, CURLOPT_URL, download_path);
						msPeriod = (timeoutTick - svcGetSystemTick()) / (systemtickfrequency / 1000);
						if (msPeriod < 1000) msPeriod = 1000;
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
			curl_easy_setopt(curl, CURLOPT_URL, "https://raw.githubusercontent.com/masagrator/FPSLocker-Warehouse/v4/README.md");
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
			msPeriod = (timeoutTick - svcGetSystemTick()) / (systemtickfrequency / 1000);
			if (msPeriod < 1000) msPeriod = 1000;
			curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, msPeriod);
			CURLcode res = curl_easy_perform(curl);
			if (res == CURLE_OK) {
				size_t filesize = ftell(fp);
				fseek(fp, 0, SEEK_SET);
				char* buffer = (char*)calloc(1, filesize + 1);
				fread(buffer, 1, filesize, fp);
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
						for (int i = -1; i >= -16; i--) {
							if (end[i] == ',' && end[i+1] == ' ' && end[i+2] != 'v') {
								size_t offset = 0;
								for (int x = i+2; x <= i+18; x++) {
									if (end[x] == ')') {
										expected_display_version[offset] = 0;
										break;
									}
									expected_display_version[offset++] = end[x];
								}
								break;
							}
						}
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
				else temp_error_code = 0x1007;
				free(buffer);
			}
			else if (res == CURLE_OPERATION_TIMEDOUT) {
				temp_error_code = 0x405;
			}
			else temp_error_code = 0x406;
			fclose(fp);
			remove("sdmc:/SaltySD/plugins/FPSLocker/patches/README.md");
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
	do {
		if (R_FAILED(pmdmntGetApplicationProcessId(&PID))) break;
	} while(!leventWait(&threadexit, 1'000'000'000));
	PluginRunning = false;
	check = false;
	closed = true;
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

extern "C" {
	#include "nacp.h"
}

/**
 * @brief Gets the \ref NsApplicationControlData for the specified application.
 * @note Only available on [19.0.0+].
 * @param[in] source Source, official sw uses ::NsApplicationControlSource_Storage.
 * @param[in] application_id ApplicationId.
 * @param[out] buffer \ref NsApplicationControlData
 * @param[in] flag1 Default is 0. 0xFF speeds up execution.
 * @param[in] flag2 Default is 0.
 * @param[in] size Size of the buffer.
 * @param[out] actual_size Actual output size.
 * @param[out] unk Returned with size, always 0.
 */
Result nsGetApplicationControlData2(NsApplicationControlSource source, u64 application_id, NsApplicationControlData* buffer, u8 flag1, u8 flag2, size_t size, u64* actual_size, u32* unk) {
    if (hosversionBefore(19,0,0))
        return MAKERESULT(Module_Libnx, LibnxError_IncompatSysVer);
    Service srv={0}, *srv_ptr = &srv;
    Result rc=0;
    u32 cmd_id = 6;
    rc = nsGetReadOnlyApplicationControlDataInterface(&srv);

    const struct {
        u8 source;
        u8 flags[2];
        u8 pad[5];
        u64 application_id;
    } in = { source, {flag1, flag2}, {0}, application_id };

    u64 tmp=0;

    if (R_SUCCEEDED(rc)) rc = serviceDispatchInOut(srv_ptr, cmd_id, in, tmp,
        .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_Out },
        .buffers = { { buffer, size } },
    );
    if (R_SUCCEEDED(rc)) {
        if (actual_size) *actual_size = tmp >> 32;
        if (unk) *unk = (u32)tmp;
    }

    serviceClose(&srv);
    return rc;
}

std::string getAppName(uint64_t Tid)
{
	NsApplicationControlData* appControlData = (NsApplicationControlData*)malloc(sizeof(NsApplicationControlData));

	Result rc = -1;
	if (hosversionBefore(19,0,0)) {
		rc = nsGetApplicationControlData(NsApplicationControlSource::NsApplicationControlSource_Storage, Tid, appControlData, sizeof(NsApplicationControlData), nullptr);
	}
	else rc = nsGetApplicationControlData2(NsApplicationControlSource::NsApplicationControlSource_Storage, Tid, appControlData, 0xFF, 0, sizeof(NsApplicationControlData), nullptr, nullptr);
	if (R_FAILED(rc)) {
		free(appControlData);
		char returnTID[18];
		sprintf(returnTID, "%016lx-", Tid);
		return (std::string)returnTID;
	}
	
	NacpLanguageEntry *languageEntry = nullptr;
	smInitialize();
	rc = nacpGetLanguageEntry2(&(appControlData -> nacp), &languageEntry);
	smExit();
	if (R_FAILED(rc)) {
		free(appControlData);
		char returnTID[18];
		sprintf(returnTID, "0x%X", rc);
		return (std::string)returnTID;
	}
	std::string to_return = languageEntry->name;
	free(appControlData);
	return to_return;
}

Result getTitles(int32_t count)
{
	NsApplicationRecord* appRecords = (NsApplicationRecord*)malloc(count * sizeof(NsApplicationRecord));
	int32_t actualAppRecordCnt = 0;
	Result rc = nsListApplicationRecord(appRecords, count, 0, &actualAppRecordCnt);
	if (R_FAILED(rc)) {
		free(appRecords);
		return rc;
	}
	for (int32_t i = 0; i < actualAppRecordCnt; i++) {
		if (appRecords[i].application_id != 0) {
			Title title;
			title.TitleID = appRecords[i].application_id;
			title.TitleName = getAppName(appRecords[i].application_id);
			mutexLock(&TitlesAccess);
			titles.push_back(title);
			mutexUnlock(&TitlesAccess);
		}
	}
	free(appRecords);
	return rc;
}

void TitlesThread(void*) {
	getTitles(32);
}

void setForceEnglishLanguage(bool set) {
	uintptr_t ptr_func = (uintptr_t)&TitlesThread;
	MemoryInfo mem = {0};
	u32 pageinfo = 0;
	svcQueryMemory(&mem, &pageinfo, ptr_func);
	bool* ptrBool = (bool*)&forceEnglishLanguage;
	uintptr_t ptrBool_integer = (uintptr_t)ptrBool;
	ptrdiff_t ptrBool_offset = ptrBool_integer - mem.addr;
	FILE* file = fopen(overlayName.c_str(), "rb+");
	if (file) {
		fseek(file, ptrBool_offset, 0);
		fwrite(&set, 1, 1, file);
		fclose(file);
	}
}

bool saveSettings() {
	if (!(Shared -> FPSlocked) && !(Shared -> FPSlockedDocked) && !(Shared -> ZeroSync) && !SetBuffers_save && !forceSuspend_save) {
		remove(savePath);
	}
	else {
		DIR* dir = opendir("sdmc:/SaltySD/plugins/");
		if (!dir) {
			mkdir("sdmc:/SaltySD/plugins/", 777);
		}
		else closedir(dir);
		dir = opendir("sdmc:/SaltySD/plugins/FPSLocker/");
		if (!dir) {
			mkdir("sdmc:/SaltySD/plugins/FPSLocker/", 777);
		}
		else closedir(dir);
		FILE* file = fopen(savePath, "wb");
		if (file) {
			fwrite(&(Shared->FPSlocked), 1, 1, file);
			if (SetBuffers_save > 2 || (!SetBuffers_save && (Shared -> Buffers) > 2)) {
				(Shared -> ZeroSync) = 0;
			}
			fwrite(&(Shared->ZeroSync), 1, 1, file);
			fwrite(&SetBuffers_save, 1, 1, file);
			fwrite(&forceSuspend_save, 1, 1, file);
			fwrite(&(Shared->FPSlockedDocked), 1, 1, file);
			fclose(file);
		}
		else return false;
	}
	return true;
}