#define TESLA_INIT_IMPL // If you have more than one file using the tesla header, only define this in the main one
#include <tesla.hpp>    // The Tesla Header
#include <sys/stat.h>
#include "SaltyNX.h"
#include "Lock.hpp"
#include "Utils.hpp"

void loopThread(void*) {
	while(R_SUCCEEDED(pmdmntGetApplicationProcessId(&PID)) || threadActive) {
		switch (*FPSmode_shared) {
			case 0:
				//This is usually a sign that game doesn't use SetPresentInterval
				sprintf(FPSMode_c, "NVN Interval Mode: 0 (Unused)");
				break;
			case 1:
				sprintf(FPSMode_c, "NVN Interval Mode: 1 (60 FPS)");
				break;
			case 2:
				sprintf(FPSMode_c, "NVN Interval Mode: 2 (30 FPS)");
				break;
			default:
				sprintf(FPSMode_c, "NVN Interval Mode: %d (Wrong)", *FPSmode_shared);
		}
		if (!*FPSlocked_shared) {
			sprintf(FPSTarget_c, "Custom FPS Target: Disabled");
		}
		else sprintf(FPSTarget_c, "Custom FPS Target: %d", *FPSlocked_shared);
		if (*patchApplied_shared) {
			sprintf(patchAppliedChar, "Plugin loaded patch to game");
		}
		else sprintf(patchAppliedChar, "Plugin didn't apply patch to game");
		sprintf(PFPS_c, "%d", *FPS_shared);
		svcSleepThread(1'000'000'000/8);
	}
	PluginRunning = false;
	check = false;
	closed = true;
}

class AdvancedGui : public tsl::Gui {
public:
    AdvancedGui() {}

    virtual tsl::elm::Element* createUI() override {
        auto frame = new tsl::elm::OverlayFrame("FPSLocker", "Advanced settings");

		auto list = new tsl::elm::List();

		list->addItem(new tsl::elm::CustomDrawer([](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {

			if (R_SUCCEEDED(configValid)) {
				renderer->drawString("Found valid config file!", false, x, y+20, 20, renderer->a(0xFFFF));
				renderer->drawString(&patchChar[0], false, x, y+40, 20, renderer->a(0xFFFF));
				renderer->drawString(&patchAppliedChar[0], false, x, y+60, 20, renderer->a(0xFFFF));
			}
			else
				renderer->drawString(&lockInvalid[0], false, x, y+20, 20, renderer->a(0xFFFF));
				

		}), 70);

		if (*FPSmode_shared != 255) {
			list->addItem(new tsl::elm::CategoryHeader("NVN", true));
			auto *clickableListItem3 = new tsl::elm::ToggleListItem("Sync Wait", !*ZeroSync_shared);
			clickableListItem3->setClickListener([](u64 keys) { 
				if ((keys & HidNpadButton_A) && PluginRunning) {
					*ZeroSync_shared = !*ZeroSync_shared;
					return true;
				}
				return false;
			});
			list->addItem(clickableListItem3);
		}

		if (R_SUCCEEDED(configValid)) {
			list->addItem(new tsl::elm::CategoryHeader("Patch will be applied on next game boot", true));
			auto *clickableListItem = new tsl::elm::ListItem("Convert config to patch file");
			clickableListItem->setClickListener([](u64 keys) { 
				if ((keys & HidNpadButton_A) && PluginRunning) {
					patchValid = LOCK::createPatch(&patchPath[0]);
					if (R_SUCCEEDED(patchValid)) {
						sprintf(&patchChar[0], "Patch file created successfully.");
					}
					else sprintf(&patchChar[0], "Error while creating patch: 0x%x", patchValid);
					return true;
				}
				return false;
			});
			list->addItem(clickableListItem);

			list->addItem(new tsl::elm::CategoryHeader("Patch won't be applied on next game boot", true));
			auto *clickableListItem2 = new tsl::elm::ListItem("Delete patch file");
			clickableListItem2->setClickListener([](u64 keys) { 
				if ((keys & HidNpadButton_A) && PluginRunning) {
					if (R_SUCCEEDED(patchValid)) {
						remove(&patchPath[0]);
						patchValid = 0x202;
						sprintf(&patchChar[0], "Patch file deleted successfully.");
					}
					return true;
				}
				return false;
			});
			list->addItem(clickableListItem2);
		}

		frame->setContent(list);

        return frame;
    }
};

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
			else if (!*pluginActive) {
				renderer->drawString("NX-FPS is running, but no frame was processed.", false, x, y+20, 20, renderer->a(0xF33F));
				renderer->drawString("Restart overlay to check again.", false, x, y+50, 20, renderer->a(0xFFFF));
			}
			else {
				renderer->drawString("NX-FPS is running.", false, x, y+20, 20, renderer->a(0xFFFF));
				if (*FPSmode_shared != 255)
					renderer->drawString(FPSMode_c, false, x, y+40, 20, renderer->a(0xFFFF));
				renderer->drawString(FPSTarget_c, false, x, y+60, 20, renderer->a(0xFFFF));
				renderer->drawString(PFPS_c, false, x+290, y+48, 50, renderer->a(0xFFFF));
			}
		}), 90);

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

			auto *clickableListItem3 = new tsl::elm::ListItem("Advanced settings");
			clickableListItem3->setClickListener([](u64 keys) { 
				if ((keys & HidNpadButton_A) && PluginRunning) {
					tsl::changeTo<AdvancedGui>();
					return true;
				}
				return false;
			});
			list->addItem(clickableListItem3);

			auto *clickableListItem5 = new tsl::elm::ListItem("Save settings");
			clickableListItem5->setClickListener([](u64 keys) { 
				if ((keys & HidNpadButton_A) && PluginRunning) {
					if (!*FPSlocked_shared && !*ZeroSync_shared) {
						remove(savePath);
					}
					else {
						mkdir("sdmc:/SaltySD/plugins/FPSLocker/", 777);
						FILE* file = fopen(savePath, "wb");
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
	virtual void update() override {}

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
					pminfoGetProgramId(&TID, PID);
					pminfoExit();
					BID = getBID();
					sprintf(&patchPath[0], "sdmc:/SaltySD/plugins/FPSLocker/patches/%016lX/%016lX.bin", TID, BID);
					sprintf(&configPath[0], "sdmc:/SaltySD/plugins/FPSLocker/patches/%016lX/%016lX.yaml", TID, BID);
					sprintf(&savePath[0], "sdmc:/SaltySD/plugins/FPSLocker/%016lX.dat", TID);

					configValid = LOCK::readConfig(&configPath[0]);
					if (R_FAILED(configValid))
						sprintf(&lockInvalid[0], "Config error: 0x%X", configValid);
					else {
						patchValid = checkFile(&patchPath[0]);
						if (R_FAILED(patchValid))
							sprintf(&patchChar[0], "Patch file doesn't exist.");
						else sprintf(&patchChar[0], "Patch file exists.");
					}
					FPS_shared = (uint8_t*)(base + rel_offset + 4);
					pluginActive = (bool*)(base + rel_offset + 9);
					FPSlocked_shared = (uint8_t*)(base + rel_offset + 10);
					FPSmode_shared = (uint8_t*)(base + rel_offset + 11);
					ZeroSync_shared = (bool*)(base + rel_offset + 12);
					patchApplied_shared = (bool*)(base + rel_offset + 13);
					PluginRunning = true;
					threadCreate(&t0, loopThread, NULL, NULL, 0x100, 0x20, 0);
					threadStart(&t0);
				}		
			}
		
		});
	
	}  // Called at the start to initialize all services necessary for this Overlay
	
	virtual void exitServices() override {
		threadActive = false;
		threadWaitForExit(&t0);
		threadClose(&t0);
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
