#pragma once
#include <curl/curl.h>

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
	uint8_t displaySync;
	resolutionCalls renderCalls[8];
	resolutionCalls viewportCalls[8];
	bool forceOriginalRefreshRate;
	bool dontForce60InDocked;
	bool forceSuspend;
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

std::vector<Title> titles;

/// Edid2
typedef struct {
	SetSysEdid edid;
	char reserved[0x100];
} SetSysEdid2;

Result setsysGetEdid2(Service* g_setsysSrv, SetSysEdid2 *out) {
    return serviceDispatch(g_setsysSrv, 41,
        .buffer_attrs = { SfBufferAttr_FixedSize | SfBufferAttr_HipcPointer | SfBufferAttr_Out },
        .buffers = { { out, sizeof(*out) } },
    );
}

void SaveDockedModeAllowedSave(DockedModeRefreshRateAllowed rr, DockedAdditionalSettings &as) {
	tsl::hlp::doWithSmSession([]{
		setsysInitialize();
	});
    SetSysEdid2 edid2 = {0};
    if (R_FAILED(setsysGetEdid2(setsysGetServiceSession(), &edid2))) {
		return;
    }
    char path[128] = "";
    snprintf(path, sizeof(path), "sdmc:/SaltySD/plugins/FPSLocker/ExtDisplays/%08X.dat", crc32Calculate(&edid2.edid, sizeof(edid2.edid)));
    FILE* file = fopen(path, "wb");
    if (file) {
		fwrite(&edid2, sizeof(edid2), 1, file);
        fseek(file, 0x100, 0);
        for (size_t i = 0; i < sizeof(DockedModeRefreshRateAllowed); i++) {
            fwrite(&rr[i], 1, 1, file);
        }
		fseek(file, 0x180, 0);
		fwrite(&as.dontForce60InDocked, 1, 1, file);
		fwrite(&as.fpsTargetWithoutRRMatchLowest, 1, 1, file);
        fclose(file);

    }
    return;
}

void LoadDockedModeAllowedSave(DockedModeRefreshRateAllowed &rr, DockedAdditionalSettings &as) {
	for (size_t i = 0; i < sizeof(DockedModeRefreshRateAllowed); i++) {
		if (DockedModeRefreshRateAllowedValues[i] == 60 || DockedModeRefreshRateAllowedValues[i] == 50) rr[i] = true;
		else rr[i] = false;
	}
	tsl::hlp::doWithSmSession([]{
		setsysInitialize();
	});
    SetSysEdid2 edid2 = {0};
    if (R_FAILED(setsysGetEdid2(setsysGetServiceSession(), &edid2))) {
		return;
    }
    char path[128] = "";
    snprintf(path, sizeof(path), "sdmc:/SaltySD/plugins/FPSLocker/ExtDisplays/%08X.dat", crc32Calculate(&edid2.edid, sizeof(edid2.edid)));
    FILE* file = fopen(path, "rb");
    if (file) {
        u64 MAGIC = 0x00FFFFFFFFFFFF00;
        u64 checkMAGIC = 0;
        fread(&checkMAGIC, 8, 1, file);
        if (checkMAGIC != MAGIC) {
			fclose(file);
			return;
        }

        fseek(file, 0x100, 0);
        for (size_t i = 0; i < sizeof(DockedModeRefreshRateAllowed); i++) {
            bool temp = false;
            if (!fread(&temp, 1, 1, file))
                break;
            if (DockedModeRefreshRateAllowedValues[i] == 60) {
				rr[i] = true;
                continue;
			}
            rr[i] = temp;
        }
		fseek(file, 0x180, 0);
		uint8_t temp = 2;
		fread(&temp, 1, 1, file);
		if (temp < 2) as.dontForce60InDocked = temp;
		temp = 2;
		fread(&temp, 1, 1, file);
		if (temp < 2) as.fpsTargetWithoutRRMatchLowest = temp;
		fclose(file);
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