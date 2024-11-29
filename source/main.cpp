#define TESLA_INIT_IMPL // If you have more than one file using the tesla header, only define this in the main one
#include <tesla.hpp>    // The Tesla Header
#include "MiniList.hpp"
#include "NoteHeader.hpp"
#include "List.hpp"
#include <sys/stat.h>
#include <dirent.h>
#include "SaltyNX.h"
#include "Lock.hpp"
#include "Utils.hpp"
#include <cstdlib>

bool displaySync = false;
bool isOLED = false;
uint8_t refreshRate_g = 60;
bool oldSalty = false;

#include "Modes/Advanced.hpp"

class NoGameSub : public tsl::Gui {
public:
	uint64_t _titleid = 0;
	char _titleidc[17] = "";
	std::string _titleName = "";

	NoGameSub(uint64_t titleID, std::string titleName) {
		_titleid = titleID;
		sprintf(&_titleidc[0], "%016lX", _titleid);
		_titleName = titleName;
	}

	// Called when this Gui gets loaded to create the UI
	// Allocate all elements on the heap. libtesla will make sure to clean them up when not needed anymore
	virtual tsl::elm::Element* createUI() override {
		// A OverlayFrame is the base element every overlay consists of. This will draw the default Title and Subtitle.
		// If you need more information in the header or want to change it's look, use a HeaderOverlayFrame.
		auto frame = new tsl::elm::OverlayFrame(_titleidc, _titleName);

		// A list that can contain sub elements and handles scrolling
		auto list = new tsl::elm::List();

		auto *clickableListItem = new tsl::elm::ListItem2("Delete settings");
		clickableListItem->setClickListener([this](u64 keys) { 
			if (keys & HidNpadButton_A) {
				char path[512] = "";
				if (_titleid != 0x1234567890ABCDEF) {
					sprintf(&path[0], "sdmc:/SaltySD/plugins/FPSLocker/%016lx.dat", _titleid);
					remove(path);
				}
				else {
					struct dirent *entry;
    				DIR *dp;
					sprintf(&path[0], "sdmc:/SaltySD/plugins/FPSLocker/");

					dp = opendir(path);
					if (!dp)
						return true;
					while ((entry = readdir(dp))) {
						if (entry -> d_type != DT_DIR && std::string(entry -> d_name).find(".dat") != std::string::npos) {
							sprintf(&path[0], "sdmc:/SaltySD/plugins/FPSLocker/%s", entry->d_name);
							remove(path);
						}
					}
					closedir(dp);
				}
				return true;
			}
			return false;
		});

		list->addItem(clickableListItem);

		auto *clickableListItem2 = new tsl::elm::ListItem2("Delete patches");
		clickableListItem2->setClickListener([this](u64 keys) { 
			if (keys & HidNpadButton_A) {
				char folder[640] = "";
				if (_titleid != 0x1234567890ABCDEF) {
					sprintf(&folder[0], "sdmc:/SaltySD/plugins/FPSLocker/patches/%016lx/", _titleid);

					struct dirent *entry;
    				DIR *dp;

					dp = opendir(folder);
					if (!dp)
						return true;
					while ((entry = readdir(dp))) {
						if (entry -> d_type != DT_DIR && std::string(entry -> d_name).find(".bin") != std::string::npos) {
							sprintf(&folder[0], "sdmc:/SaltySD/plugins/FPSLocker/patches/%016lx/%s", _titleid, entry -> d_name);
							remove(folder);
						}
					}
					closedir(dp);
				}
				else {
					struct dirent *entry;
					struct dirent *entry2;
    				DIR *dp;
					DIR *dp2;

					sprintf(&folder[0], "sdmc:/SaltySD/plugins/FPSLocker/patches/");
					dp = opendir(folder);
					if (!dp)
						return true;
					while ((entry = readdir(dp))) {
						if (entry -> d_type != DT_DIR)
							continue;
						sprintf(&folder[0], "sdmc:/SaltySD/plugins/FPSLocker/patches/%s/", entry -> d_name);
						dp2 = opendir(folder);
						while ((entry2 = readdir(dp2))) {
							if (entry2 -> d_type != DT_DIR && std::string(entry2 -> d_name).find(".bin") != std::string::npos) {
								sprintf(&folder[0], "sdmc:/SaltySD/plugins/FPSLocker/patches/%s/%s", entry -> d_name, entry2 -> d_name);
								remove(folder);
							}
						}
						closedir(dp2);
					}
					closedir(dp);
				}
				return true;
			}
			return false;
		});

		list->addItem(clickableListItem2);

		frame->setContent(list);

		return frame;
	}
};

class NoGame2 : public tsl::Gui {
public:

	Result rc = 1;
	NoGame2(Result result, u8 arg2, bool arg3) {
		rc = result;
	}

	// Called when this Gui gets loaded to create the UI
	// Allocate all elements on the heap. libtesla will make sure to clean them up when not needed anymore
	virtual tsl::elm::Element* createUI() override {
		// A OverlayFrame is the base element every overlay consists of. This will draw the default Title and Subtitle.
		// If you need more information in the header or want to change it's look, use a HeaderOverlayFrame.
		auto frame = new tsl::elm::OverlayFrame("FPSLocker", APP_VERSION);

		// A list that can contain sub elements and handles scrolling
		auto list = new tsl::elm::List();

		if (oldSalty || !SaltySD) {
			list->addItem(new tsl::elm::CustomDrawer([](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
				if (!SaltySD) {
					renderer->drawString("SaltyNX is not working!", false, x, y+20, 20, renderer->a(0xF33F));
				}
				else if (!plugin) {
					renderer->drawString("Can't detect NX-FPS plugin on sdcard!", false, x, y+20, 20, renderer->a(0xF33F));
				}
				else if (!check) {
					renderer->drawString("Game is not running!", false, x, y+20, 19, renderer->a(0xF33F));
				}
			}), 30);
		}

		if (R_FAILED(rc)) {
			char error[24] = "";
			sprintf(&error[0], "Err: 0x%x", rc);
			auto *clickableListItem2 = new tsl::elm::ListItem2(error);
			clickableListItem2->setClickListener([](u64 keys) { 
				if (keys & HidNpadButton_A) {
					return true;
				}
				return false;
			});

			list->addItem(clickableListItem2);
		}
		else {
			auto *clickableListItem3 = new tsl::elm::ListItem2("All");
			clickableListItem3->setClickListener([](u64 keys) { 
				if (keys & HidNpadButton_A) {
					tsl::changeTo<NoGameSub>(0x1234567890ABCDEF, "Everything");
					return true;
				}
				return false;
			});

			list->addItem(clickableListItem3);

			for (size_t i = 0; i < titles.size(); i++) {
				auto *clickableListItem = new tsl::elm::ListItem2(titles[i].TitleName);
				clickableListItem->setClickListener([i](u64 keys) { 
					if (keys & HidNpadButton_A) {
						tsl::changeTo<NoGameSub>(titles[i].TitleID, titles[i].TitleName);
						return true;
					}
					return false;
				});

				list->addItem(clickableListItem);
			}
		}

		frame->setContent(list);

		return frame;
	}
};

#include "Modes/Display.hpp"

class NoGame : public tsl::Gui {
public:

	Result rc = 1;
	NoGame(Result result, u8 arg2, bool arg3) {
		rc = result;
	}

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
				renderer->drawString("SaltyNX is not working!", false, x, y+20, 20, renderer->a(0xF33F));
			}
			else if (!plugin) {
				renderer->drawString("Can't detect NX-FPS plugin on sdcard!", false, x, y+20, 20, renderer->a(0xF33F));
			}
			else if (!check) {
				renderer->drawString("Game is not running!", false, x, y+20, 19, renderer->a(0xF33F));
			}
		}), 30);

		auto *clickableListItem2 = new tsl::elm::ListItem2("Games list");
		clickableListItem2->setClickListener([this](u64 keys) { 
			if (keys & HidNpadButton_A) {
				tsl::changeTo<NoGame2>(this -> rc, 2, true);
				return true;
			}
			return false;
		});

		list->addItem(clickableListItem2);

		auto *clickableListItem3 = new tsl::elm::ListItem2("Display settings", "\uE151");
		clickableListItem3->setClickListener([](u64 keys) { 
			if (keys & HidNpadButton_A) {
				tsl::changeTo<WarningDisplayGui>();
				return true;
			}
			return false;
		});

		list->addItem(clickableListItem3);


		frame->setContent(list);

		return frame;
	}

	virtual void update() override {}

	// Called once every frame to handle inputs not handled by other UI elements
	virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
		return false;   // Return true here to singal the inputs have been consumed
	}
};

uint8_t AllowedFPSTargets[] = {15, 20, 25, 30, 35, 40, 45, 50, 55, 60};

class DockedFPSTargetGui : public tsl::Gui {
public:
	uint8_t maxFPS = 0;
	DockedModeRefreshRateAllowed rr = {0};
	DockedAdditionalSettings as = {0};
	DockedFPSTargetGui() {
		LoadDockedModeAllowedSave(rr, as);
		for (size_t i = 0; i < sizeof(DockedModeRefreshRateAllowed); i++) {
			if (rr[i] == true)
				maxFPS = DockedModeRefreshRateAllowedValues[i];
		}
	}

	// Called when this Gui gets loaded to create the UI
	// Allocate all elements on the heap. libtesla will make sure to clean them up when not needed anymore
	virtual tsl::elm::Element* createUI() override {
		// A OverlayFrame is the base element every overlay consists of. This will draw the default Title and Subtitle.
		// If you need more information in the header or want to change it's look, use a HeaderOverlayFrame.
		auto frame = new tsl::elm::OverlayFrame("FPSLocker", "Change FPS Target");

		// A list that can contain sub elements and handles scrolling
		auto list = new tsl::elm::List();

		for (int8_t i = sizeof(AllowedFPSTargets) - 1; i >= 0; i -= 1) {
			char FPS[] = "254 FPS";
			snprintf(FPS, sizeof(FPS), "%d FPS", AllowedFPSTargets[i]);
			auto *clickableListItem = new tsl::elm::MiniListItem(FPS);
			clickableListItem->setClickListener([this, i](u64 keys) { 
				if (keys & HidNpadButton_A) {
					(Shared -> FPSlocked) = AllowedFPSTargets[i];
					if (!oldSalty && displaySync) {
						if (R_SUCCEEDED(SaltySD_Connect())) {
							bool skip = false;
							SaltySD_SetDisplayRefreshRate(AllowedFPSTargets[i]);
							for (uint8_t x = 0; x < sizeof(DockedModeRefreshRateAllowed); x++) {
								if (rr[x] == true && DockedModeRefreshRateAllowedValues[x] == AllowedFPSTargets[i]) {
									refreshRate_g = AllowedFPSTargets[i];
									skip = true;
								}
								else if (rr[x] == true && DockedModeRefreshRateAllowedValues[x] == (AllowedFPSTargets[i] * 2)) {
									refreshRate_g = AllowedFPSTargets[i] * 2;
									skip = true;
								}
								else if (rr[x] == true && DockedModeRefreshRateAllowedValues[x] == (AllowedFPSTargets[i] * 3)) {
									refreshRate_g = AllowedFPSTargets[i] * 3;
									skip = true;
								}
								else if (rr[x] == true && DockedModeRefreshRateAllowedValues[x] == (AllowedFPSTargets[i] * 4)) {
									refreshRate_g = AllowedFPSTargets[i] * 4;
									skip = true;
								}
								if (skip) break;
							}
							if (!skip) {
								uint8_t target = 60;
								for (uint8_t x = 0; x < sizeof(DockedModeRefreshRateAllowed); x++) {
									if (rr[x] == true && AllowedFPSTargets[i] <= DockedModeRefreshRateAllowedValues[x]) {
										if (DockedModeRefreshRateAllowedValues[x] < target) target = DockedModeRefreshRateAllowedValues[x];
									}
								}
								refreshRate_g = target;
							}
							SaltySD_Term();
							(Shared -> displaySync) = refreshRate_g;
						}
					}
					tsl::goBack();
					return true;
				}
				return false;
			});

			list->addItem(clickableListItem);
		}

		frame->setContent(list);

		return frame;
	}

	// Called once every frame to handle inputs not handled by other UI elements
	virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
		smInitialize();
		if (R_SUCCEEDED(apmInitialize())) {
			ApmPerformanceMode mode = ApmPerformanceMode_Invalid;
			apmGetPerformanceMode(&mode);
			apmExit();
			if (mode != ApmPerformanceMode_Boost) {
				smExit();
				tsl::goBack();
				return true;
			}
		}
		smExit();
		return false;   // Return true here to singal the inputs have been consumed
	}
};

class GuiTest : public tsl::Gui {
public:
	ApmPerformanceMode entry_mode = ApmPerformanceMode_Invalid;
	GuiTest(u8 arg1, u8 arg2, bool arg3) { 
		smInitialize();
		if (R_SUCCEEDED(apmInitialize())) {
			apmGetPerformanceMode(&entry_mode);
			apmExit();
		}
		smExit();
	}

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
			else if (!(Shared -> pluginActive)) {
				renderer->drawString("NX-FPS is running, but no frame was processed.", false, x, y+20, 20, renderer->a(0xF33F));
				renderer->drawString("Restart overlay to check again.", false, x, y+50, 20, renderer->a(0xFFFF));
			}
			else {
				renderer->drawString("NX-FPS is running.", false, x, y+20, 20, renderer->a(0xFFFF));
				if (((Shared -> API) > 0) && ((Shared -> API) <= 2))
					renderer->drawString(FPSMode_c, false, x, y+40, 20, renderer->a(0xFFFF));
				renderer->drawString(FPSTarget_c, false, x, y+60, 20, renderer->a(0xFFFF));
				renderer->drawString(PFPS_c, false, x+290, y+48, 50, renderer->a(0xFFFF));
				if (Shared -> forceOriginalRefreshRate) renderer->drawString("Patch is now forcing 60 Hz.", false, x, y+80, 20, renderer->a(0xF99F));
			}
		}), 90);

		if (PluginRunning && (Shared -> pluginActive)) {
			if (entry_mode == ApmPerformanceMode_Normal) {
				auto *clickableListItem = new tsl::elm::ListItem2("Increase FPS target");
				clickableListItem->setClickListener([](u64 keys) { 
					if ((keys & HidNpadButton_A) && PluginRunning) {
						if ((Shared -> FPSmode) == 2 && !(Shared -> FPSlocked)) {
							(Shared -> FPSlocked) = 35;
						}
						else if (!(Shared -> FPSlocked)) {
							(Shared -> FPSlocked) = 60;
						}
						else if ((Shared -> FPSlocked) < 60) {
							(Shared -> FPSlocked) += 5;
						}
						if (!oldSalty && displaySync) {
							if (R_SUCCEEDED(SaltySD_Connect())) {
								bool skip = false;
								SaltySD_SetDisplayRefreshRate((Shared -> FPSlocked));
								for (uint8_t x = 0; x < sizeof(supportedHandheldRefreshRates); x++) {
									if (supportedHandheldRefreshRates[x] == (Shared -> FPSlocked)) {
										refreshRate_g = (Shared -> FPSlocked);
										skip = true;
									}
									else if (supportedHandheldRefreshRates[x] == ((Shared -> FPSlocked) * 2)) {
										refreshRate_g = (Shared -> FPSlocked) * 2;
										skip = true;
									}
									else if (supportedHandheldRefreshRates[x] == ((Shared -> FPSlocked) * 3)) {
										refreshRate_g = (Shared -> FPSlocked) * 3;
										skip = true;
									}
									else if (supportedHandheldRefreshRates[x] == ((Shared -> FPSlocked) * 4)) {
										refreshRate_g = (Shared -> FPSlocked) * 4;
										skip = true;
									}
									if (skip) break;
								}
								if (!skip) {
									refreshRate_g = 60;
								}
								(Shared -> displaySync) = refreshRate_g;
								SaltySD_Term();
							}
						}
						return true;
					}
					return false;
				});

				list->addItem(clickableListItem);
				
				auto *clickableListItem2 = new tsl::elm::ListItem2("Decrease FPS target");
				clickableListItem2->setClickListener([](u64 keys) { 
					if ((keys & HidNpadButton_A) && PluginRunning) {
						if ((Shared -> FPSmode) < 2 && !(Shared -> FPSlocked)) {
							(Shared -> FPSlocked) = 55;
						}
						else if (!(Shared -> FPSlocked)) {
							(Shared -> FPSlocked) = 25;
						}
						else if ((Shared -> FPSlocked) > 15) {
							(Shared -> FPSlocked) -= 5;
						}
						if (!oldSalty && displaySync) {
							if (R_SUCCEEDED(SaltySD_Connect())) {
								bool skip = false;
								SaltySD_SetDisplayRefreshRate((Shared -> FPSlocked));
								for (uint8_t x = 0; x < sizeof(supportedHandheldRefreshRates); x++) {
									if (supportedHandheldRefreshRates[x] == (Shared -> FPSlocked)) {
										refreshRate_g = (Shared -> FPSlocked);
										skip = true;
									}
									else if (supportedHandheldRefreshRates[x] == ((Shared -> FPSlocked) * 2)) {
										refreshRate_g = (Shared -> FPSlocked) * 2;
										skip = true;
									}
									else if (supportedHandheldRefreshRates[x] == ((Shared -> FPSlocked) * 3)) {
										refreshRate_g = (Shared -> FPSlocked) * 3;
										skip = true;
									}
									else if (supportedHandheldRefreshRates[x] == ((Shared -> FPSlocked) * 4)) {
										refreshRate_g = (Shared -> FPSlocked) * 4;
										skip = true;
									}
									if (skip) break;
								}
								if (!skip) {
									refreshRate_g = 60;
								}
								SaltySD_Term();
								(Shared -> displaySync) = refreshRate_g;
							}
						}
						return true;
					}
					return false;
				});
				list->addItem(clickableListItem2);
			}
			else if (entry_mode == ApmPerformanceMode_Boost) {
				auto *clickableListItem2 = new tsl::elm::ListItem2("Change FPS target");
				clickableListItem2->setClickListener([](u64 keys) { 
					if ((keys & HidNpadButton_A) && PluginRunning) {
						tsl::changeTo<DockedFPSTargetGui>();
						return true;
					}
					return false;
				});	
				list->addItem(clickableListItem2);			
			}

			auto *clickableListItem4 = new tsl::elm::ListItem2("Disable custom FPS target");
			clickableListItem4->setClickListener([](u64 keys) { 
				if ((keys & HidNpadButton_A) && PluginRunning) {
					if ((Shared -> FPSlocked)) {
						(Shared -> FPSlocked) = 0;
					}
					if (displaySync) {
						if (!oldSalty && R_SUCCEEDED(SaltySD_Connect())) {
							SaltySD_SetDisplayRefreshRate(60);
							(Shared -> displaySync) = 0;
							SaltySD_Term();
						}
					}
					return true;
				}
				return false;
			});
			list->addItem(clickableListItem4);

			auto *clickableListItem3 = new tsl::elm::ListItem2("Advanced settings");
			clickableListItem3->setClickListener([](u64 keys) { 
				if ((keys & HidNpadButton_A) && PluginRunning) {
					tsl::changeTo<AdvancedGui>();
					return true;
				}
				return false;
			});
			list->addItem(clickableListItem3);

			auto *clickableListItem5 = new tsl::elm::ListItem2("Save settings");
			clickableListItem5->setClickListener([](u64 keys) { 
				if ((keys & HidNpadButton_A) && PluginRunning) {
					if (!(Shared -> FPSlocked) && !(Shared -> ZeroSync) && !SetBuffers_save) {
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
							if (SetBuffers_save) {
								fwrite(&SetBuffers_save, 1, 1, file);
							}
							fclose(file);
						}
					}
					return true;
				}
				return false;
			});
			list->addItem(clickableListItem5);
		}

		if (SaltySD) {
			auto *clickableListItem6 = new tsl::elm::ListItem2("Display settings", "\uE151");
			clickableListItem6->setClickListener([](u64 keys) { 
				if (keys & HidNpadButton_A) {
					tsl::changeTo<WarningDisplayGui>();
					return true;
				}
				return false;
			});
			list->addItem(clickableListItem6);
		}

		// Add the list to the frame for it to be drawn
		frame->setContent(list);
		
		// Return the frame to have it become the top level element of this Gui
		return frame;
	}

	// Called once every frame to update values
	virtual void update() override {
		static uint8_t i = 10;

		if (PluginRunning) {
			if (i > 9) {
				switch ((Shared -> FPSmode)) {
					case 0:
						//This is usually a sign that game doesn't use interval
						sprintf(FPSMode_c, "Interval Mode: 0 (Unused)");
						break;
					case 1 ... 5:
						if (std::fmod((double)refreshRate_g, (double)(Shared -> FPSmode)) != 0.0) {
							sprintf(FPSMode_c, "Interval Mode: %d (%.1f FPS)", (Shared -> FPSmode), (double)refreshRate_g / (Shared -> FPSmode));
						}
						else sprintf(FPSMode_c, "Interval Mode: %d (%d FPS)", (Shared -> FPSmode), refreshRate_g / (Shared -> FPSmode));
						break;
					default:
						sprintf(FPSMode_c, "Interval Mode: %d (Wrong)", (Shared -> FPSmode));
				}
				if (!(Shared -> FPSlocked)) {
					sprintf(FPSTarget_c, "Custom FPS Target: Disabled");
				}
				else sprintf(FPSTarget_c, "Custom FPS Target: %d", (Shared -> FPSlocked));
				sprintf(PFPS_c, "%d", (Shared -> FPS));
				i = 0;
			}
			else i++;
		}
	}

	// Called once every frame to handle inputs not handled by other UI elements
	virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
		refreshRate_g = *refreshRate_shared;
		smInitialize();
		if (R_SUCCEEDED(apmInitialize())) {
			ApmPerformanceMode mode = ApmPerformanceMode_Invalid;
			apmGetPerformanceMode(&mode);
			apmExit();
			if (mode != entry_mode) {
				smExit();
				tsl::goBack();
				tsl::changeTo<GuiTest>(0, 1, true);
				return true;
			}
		}
		smExit();
		if (keysDown & HidNpadButton_B) {
			tsl::goBack();
			tsl::goBack();
			return true;
		}
		return false;   // Return true here to singal the inputs have been consumed
	}
};

class Dummy : public tsl::Gui {
public:
	Dummy(u8 arg1, u8 arg2, bool arg3) {}

	// Called when this Gui gets loaded to create the UI
	// Allocate all elements on the heap. libtesla will make sure to clean them up when not needed anymore
	virtual tsl::elm::Element* createUI() override {
		auto frame = new tsl::elm::OverlayFrame("FPSLocker", APP_VERSION);
		return frame;
	}

	// Called once every frame to handle inputs not handled by other UI elements
	virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
		tsl::changeTo<GuiTest>(0, 1, true);
		return true;   // Return true here to singal the inputs have been consumed
	}
};

class OverlayTest : public tsl::Overlay {
public:
	// libtesla already initialized fs, hid, pl, pmdmnt, hid:sys and set:sys
	virtual void initServices() override {

		tsl::hlp::doWithSmSession([]{
			
			setsysInitialize();
			SetSysProductModel model;
			if (R_SUCCEEDED(setsysGetProductModel(&model))) {
				if (model == SetSysProductModel_Aula) {
					isOLED = true;
					remove("sdmc:/SaltySD/flags/displaysync.flag");
				}
			}
			setsysExit();
			fsdevMountSdmc();
			FILE* file = fopen("sdmc:/SaltySD/flags/displaysync.flag", "rb");
			if (file) {
				displaySync = true;
				fclose(file);
			}
			SaltySD = CheckPort();
			if (!SaltySD) return;

			if (R_SUCCEEDED(SaltySD_Connect())) {
				uint8_t refreshRate_temp = 0;
				if (R_FAILED(SaltySD_GetDisplayRefreshRate(&refreshRate_temp))) {
					refreshRate_g = 60;
					oldSalty = true;
				}
				else refreshRate_g = refreshRate_temp;
				svcSleepThread(100'000);
				SaltySD_Term();
			}

			if(!LoadSharedMemory()) return;
			uintptr_t base = (uintptr_t)shmemGetAddr(&_sharedmemory);
			refreshRate_shared = (uint8_t*)(base + 1);

			if (R_FAILED(pmdmntGetApplicationProcessId(&PID))) return;
			check = true;
			
			ptrdiff_t rel_offset = searchSharedMemoryBlock(base);
			if (rel_offset > -1) {
				Shared = (NxFpsSharedBlock*)(base + rel_offset);
			}

			if (!PluginRunning) {
				if (rel_offset > -1) {
					pminfoInitialize();
					pminfoGetProgramId(&TID, PID);
					pminfoExit();
					BID = getBID();
					sprintf(&patchPath[0], "sdmc:/SaltySD/plugins/FPSLocker/patches/%016lX/%016lX.bin", TID, BID);
					sprintf(&configPath[0], "sdmc:/SaltySD/plugins/FPSLocker/patches/%016lX/%016lX.yaml", TID, BID);
					sprintf(&savePath[0], "sdmc:/SaltySD/plugins/FPSLocker/%016lX.dat", TID);

					SetBuffers_save = (Shared -> SetBuffers);
					PluginRunning = true;
					threadCreate(&t0, loopThread, NULL, NULL, 0x1000, 0x20, 0);
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
		nsExit();
		fsdevUnmountDevice("sdmc");
	}  // Callet at the end to clean up all services previously initialized

	virtual void onShow() override {}    // Called before overlay wants to change from invisible to visible state
	
	virtual void onHide() override {}    // Called before overlay wants to change from visible to invisible state

	virtual std::unique_ptr<tsl::Gui> loadInitialGui() override {
		if (SaltySD && check && plugin) {
			return initially<Dummy>(1, 2, true);  // Initial Gui to load. It's possible to pass arguments to it's constructor like this
		}
		else {
			tsl::hlp::doWithSmSession([]{
				nsInitialize();
			});
			Result rc = getTitles(32);
			if (oldSalty || !SaltySD)
				return initially<NoGame2>(rc, 2, true);
			else return initially<NoGame>(rc, 2, true);
		}
	}
};

int main(int argc, char **argv) {
	return tsl::loop<OverlayTest>(argc, argv);
}
