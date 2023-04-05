#pragma once

uint8_t* FPS_shared = 0;
uint8_t* FPSmode_shared = 0;
uint8_t* FPSlocked_shared = 0;
uint8_t* API_shared = 0;
bool* pluginActive = 0;
bool* ZeroSync_shared = 0;
bool _isDocked = false;
bool _def = true;
bool PluginRunning = false;
bool state = false;
bool closed = false;
bool check = false;
bool SaltySD = false;
bool bak = false;
bool plugin = false;
char FPSMode_c[64];
char FPSTarget_c[32];
char PFPS_c[32];
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
char lockInvalid[32] = "";
char patchChar[64] = "";
char patchAppliedChar[64] = "";
bool* patchApplied_shared = 0;
Thread t0;
bool threadActive = true;

struct Title
{
	uint64_t TitleID;
	std::string TitleName;
};

std::vector<Title> titles;

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
				if ((module_infos[itr].base_address - comp_address == 0x4000) || (module_infos[itr].base_address - comp_address == 0x6000)) {
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