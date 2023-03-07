#define TESLA_INIT_IMPL // If you have more than one file using the tesla header, only define this in the main one
#include <tesla.hpp>    // The Tesla Header
#include <sys/stat.h>
#include "SaltyNX.h"

uint8_t* FPS_shared = 0;
uint8_t* FPSmode_shared = 0;
uint8_t* FPSlocked_shared = 0;
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
uint64_t PID = 0;
uint64_t TID = 0;
Handle remoteSharedMemory = 1;
SharedMemory _sharedmemory = {};
bool SharedMemoryUsed = false;

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

class GuiTest : public tsl::Gui {
public:
	GuiTest(u8 arg1, u8 arg2, bool arg3) { }

	// Called when this Gui gets loaded to create the UI
	// Allocate all elements on the heap. libtesla will make sure to clean them up when not needed anymore
	virtual tsl::elm::Element* createUI() override {
		// A OverlayFrame is the base element every overlay consists of. This will draw the default Title and Subtitle.
		// If you need more information in the header or want to change it's look, use a HeaderOverlayFrame.
		auto frame = new tsl::elm::OverlayFrame("FPSLocker", APP_VERSION);

		// A list that can contain sub elements and handles scrolling
		auto list = new tsl::elm::List();
		
		list->addItem(new tsl::elm::CustomDrawer([](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
			if (!SaltySD) {
				renderer->drawString("SaltyNX is not working!", false, x, y+50, 20, renderer->a(0xF33F));
			}
			else if (!plugin) {
				renderer->drawString("Can't detect NX-FPS plugin on sdcard!", false, x, y+50, 20, renderer->a(0xF33F));
			}
			else if (!check) {
				if (closed) {
					renderer->drawString("Game was closed! Overlay disabled!", false, x, y+20, 19, renderer->a(0xF33F));
				}
				else {
					renderer->drawString("Game is not running! Overlay disabled!", false, x, y+20, 19, renderer->a(0xF33F));
				}
			}
			else if (!PluginRunning) {
				renderer->drawString("Game is running.", false, x, y+20, 20, renderer->a(0xFFFF));
				renderer->drawString("NX-FPS is not running!", false, x, y+40, 20, renderer->a(0xF33F));
			}
			else {
				renderer->drawString("NX-FPS plugin is running.", false, x, y+20, 20, renderer->a(0xFFFF));
				renderer->drawString(FPSMode_c, false, x, y+40, 20, renderer->a(0xFFFF));
				renderer->drawString(FPSTarget_c, false, x, y+60, 20, renderer->a(0xFFFF));
				renderer->drawString(PFPS_c, false, x+290, y+48, 50, renderer->a(0xFFFF));
			}
	}), 100);

		if (PluginRunning && *pluginActive) {
			auto *clickableListItem = new tsl::elm::ListItem("Increase FPS target");
			clickableListItem->setClickListener([](u64 keys) { 
				if ((keys & HidNpadButton_A) && PluginRunning) {
					if (*FPSmode_shared == 2 && !*FPSlocked_shared) {
						*FPSlocked_shared = 35;
					}
					else if (!*FPSlocked_shared) {
						*FPSlocked_shared = 60;
					}
					else if (*FPSlocked_shared < 60) {
						*FPSlocked_shared += 5;
					}
					return true;
				}
				return false;
			});

			list->addItem(clickableListItem);
			
			auto *clickableListItem2 = new tsl::elm::ListItem("Decrease FPS target");
			clickableListItem2->setClickListener([](u64 keys) { 
				if ((keys & HidNpadButton_A) && PluginRunning) {
					if (*FPSmode_shared < 2 && !*FPSlocked_shared) {
						*FPSlocked_shared = 55;
					}
					else if (!*FPSlocked_shared) {
						*FPSlocked_shared = 25;
					}
					else if (*FPSlocked_shared > 15) {
						*FPSlocked_shared -= 5;
					}
					return true;
				}
				return false;
			});
			list->addItem(clickableListItem2);

			auto *clickableListItem4 = new tsl::elm::ListItem("Disable custom FPS target");
			clickableListItem4->setClickListener([](u64 keys) { 
				if ((keys & HidNpadButton_A) && PluginRunning) {
					if (*FPSlocked_shared) {
						*FPSlocked_shared = 0;
					}
					return true;
				}
				return false;
			});
			list->addItem(clickableListItem4);

			auto *clickableListItem3 = new tsl::elm::ToggleListItem("Sync Wait (!)", !*ZeroSync_shared);
			clickableListItem3->setClickListener([](u64 keys) { 
				if ((keys & HidNpadButton_A) && PluginRunning) {
					*ZeroSync_shared = !*ZeroSync_shared;
					return true;
				}
				return false;
			});
			list->addItem(clickableListItem3);

			auto *clickableListItem5 = new tsl::elm::ListItem("Save settings");
			clickableListItem5->setClickListener([](u64 keys) { 
				if ((keys & HidNpadButton_A) && PluginRunning) {
					char path[64];
					pminfoGetProgramId(&TID, PID);
					sprintf(&path[0], "sdmc:/SaltySD/plugins/FPSLocker/%016lX.dat", TID);
					if (!*FPSlocked_shared && !*ZeroSync_shared) {
						remove(path);
					}
					else {
						mkdir("sdmc:/SaltySD/plugins/FPSLocker/", 777);
						FILE* file = fopen(path, "wb");
						if (file) {
							fwrite(FPSlocked_shared, 1, 1, file);
							fwrite(ZeroSync_shared, 1, 1, file);
							fclose(file);
						}
					}
					return true;
				}
				return false;
			});
			list->addItem(clickableListItem5);
		}

		// Add the list to the frame for it to be drawn
		frame->setContent(list);
        
		// Return the frame to have it become the top level element of this Gui
		return frame;
	}

	// Called once every frame to update values
	virtual void update() override {
		static uint8_t i = 10;
		Result rc = pmdmntGetApplicationProcessId(&PID);
		if (R_FAILED(rc) && PluginRunning) {
			PluginRunning = false;
			check = false;
			closed = true;
		}

		if (PluginRunning) {
			if (i > 9) {
				switch (*FPSmode_shared) {
					case 0:
						//This is usually a sign that game doesn't use SetPresentInterval
						sprintf(FPSMode_c, "Interval Mode: 0 (Unused)");
						break;
					case 1:
						sprintf(FPSMode_c, "Interval Mode: 1 (60 FPS)");
						break;
					case 2:
						sprintf(FPSMode_c, "Interval Mode: 2 (30 FPS)");
						break;
					default:
						sprintf(FPSMode_c, "Interval Mode: %d (Wrong)", *FPSmode_shared);
				}
				if (!*FPSlocked_shared) {
					sprintf(FPSTarget_c, "Custom FPS Target: Disabled");
				}
				else sprintf(FPSTarget_c, "Custom FPS Target: %d", *FPSlocked_shared);
				sprintf(PFPS_c, "%d", *FPS_shared);
				i = 0;
			}
			else i++;
		}
	
	}

	// Called once every frame to handle inputs not handled by other UI elements
	virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
		return false;   // Return true here to singal the inputs have been consumed
	}
};

class OverlayTest : public tsl::Overlay {
public:
	// libtesla already initialized fs, hid, pl, pmdmnt, hid:sys and set:sys
	virtual void initServices() override {

		tsl::hlp::doWithSmSession([]{
			
			fsdevMountSdmc();
			SaltySD = CheckPort();
			if (!SaltySD) return;

			FILE* temp = fopen("sdmc:/SaltySD/plugins/NX-FPS.elf", "rb");
			if (temp) {
				fclose(temp);
				plugin = true;
			}
			else return;

			if (R_FAILED(pmdmntGetApplicationProcessId(&PID))) return;
			check = true;
			
			if(!LoadSharedMemory()) return;

			if (!PluginRunning) {
				uintptr_t base = (uintptr_t)shmemGetAddr(&_sharedmemory);
				ptrdiff_t rel_offset = searchSharedMemoryBlock(base);
				if (rel_offset > -1) {
					pminfoInitialize();
					FPS_shared = (uint8_t*)(base + rel_offset + 4);
					pluginActive = (bool*)(base + rel_offset + 9);
					FPSlocked_shared = (uint8_t*)(base + rel_offset + 10);
					FPSmode_shared = (uint8_t*)(base + rel_offset + 11);
					ZeroSync_shared = (bool*)(base + rel_offset + 12);
					PluginRunning = true;
				}		
			}
		
		});
	
	}  // Called at the start to initialize all services necessary for this Overlay
	
	virtual void exitServices() override {
		pminfoExit();
		shmemClose(&_sharedmemory);
		fsdevUnmountDevice("sdmc");
	}  // Callet at the end to clean up all services previously initialized

	virtual void onShow() override {}    // Called before overlay wants to change from invisible to visible state
	
	virtual void onHide() override {}    // Called before overlay wants to change from visible to invisible state

	virtual std::unique_ptr<tsl::Gui> loadInitialGui() override {
		return initially<GuiTest>(1, 2, true);  // Initial Gui to load. It's possible to pass arguments to it's constructor like this
	}
};

int main(int argc, char **argv) {
    return tsl::loop<OverlayTest>(argc, argv);
}
