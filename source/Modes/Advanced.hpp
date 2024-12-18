class SetBuffers : public tsl::Gui {
public:
    SetBuffers() {}

    virtual tsl::elm::Element* createUI() override {
		auto frame = new tsl::elm::OverlayFrame("NVN Set Buffering", " ");

		auto list = new tsl::elm::List();
		list->addItem(new tsl::elm::CategoryHeader("It will be applied on next game boot.", false));
		list->addItem(new tsl::elm::NoteHeader("Remember to save settings after change.", true, {0xF, 0x3, 0x3, 0xF}));
		auto *clickableListItem = new tsl::elm::ListItem2("Double");
		clickableListItem->setClickListener([](u64 keys) { 
			if ((keys & HidNpadButton_A) && PluginRunning) {
				SetBuffers_save = 2;
				tsl::goBack();
				return true;
			}
			return false;
		});
		list->addItem(clickableListItem);

		if ((Shared -> SetActiveBuffers) == 2 && (Shared -> Buffers) == 3 && !SetBuffers_save) {
			auto *clickableListItem2 = new tsl::elm::ListItem2("Triple (force)");
			clickableListItem2->setClickListener([](u64 keys) { 
				if ((keys & HidNpadButton_A) && PluginRunning) {
					SetBuffers_save = 3;
					tsl::goBack();
					return true;
				}
				return false;
			});
			list->addItem(clickableListItem2);
		}
		else {
			auto *clickableListItem2 = new tsl::elm::ListItem2("Triple");
			clickableListItem2->setClickListener([](u64 keys) { 
				if ((keys & HidNpadButton_A) && PluginRunning) {
					if ((Shared -> Buffers) == 4) SetBuffers_save = 3;
					else SetBuffers_save = 0;
					tsl::goBack();
					return true;
				}
				return false;
			});
			list->addItem(clickableListItem2);
		}
		
		if ((Shared -> Buffers) == 4) {
			if ((Shared -> SetActiveBuffers) < 4 && (Shared -> SetActiveBuffers) > 0 && (Shared -> Buffers) == 4) {
				auto *clickableListItem3 = new tsl::elm::ListItem2("Quadruple (force)");
				clickableListItem3->setClickListener([](u64 keys) { 
					if ((keys & HidNpadButton_A) && PluginRunning) {
						SetBuffers_save = 4;
						tsl::goBack();
						return true;
					}
					return false;
				});
				list->addItem(clickableListItem3);	
			}
			else {
				auto *clickableListItem3 = new tsl::elm::ListItem2("Quadruple");
				clickableListItem3->setClickListener([](u64 keys) { 
					if ((keys & HidNpadButton_A) && PluginRunning) {
						SetBuffers_save = 0;
						tsl::goBack();
						return true;
					}
					return false;
				});
				list->addItem(clickableListItem3);
			}
		}

		frame->setContent(list);

        return frame;
    }
};

class SyncMode : public tsl::Gui {
public:
    SyncMode() {}

    virtual tsl::elm::Element* createUI() override {
        auto frame = new tsl::elm::OverlayFrame("NVN Window Sync Wait", "Mode");

		auto list = new tsl::elm::List();

		auto *clickableListItem = new tsl::elm::ListItem2("Enabled");
		clickableListItem->setClickListener([](u64 keys) { 
			if ((keys & HidNpadButton_A) && PluginRunning) {
				ZeroSyncMode = "On";
				(Shared -> ZeroSync) = 0;
				tsl::goBack();
				tsl::goBack();
				return true;
			}
			return false;
		});
		list->addItem(clickableListItem);

		auto *clickableListItem2 = new tsl::elm::ListItem2("Semi-Enabled");
		clickableListItem2->setClickListener([](u64 keys) { 
			if ((keys & HidNpadButton_A) && PluginRunning) {
				ZeroSyncMode = "Semi";
				(Shared -> ZeroSync) = 2;
				tsl::goBack();
				tsl::goBack();
				return true;
			}
			return false;
		});
		list->addItem(clickableListItem2);

		auto *clickableListItem3 = new tsl::elm::ListItem2("Disabled");
		clickableListItem3->setClickListener([](u64 keys) { 
			if ((keys & HidNpadButton_A) && PluginRunning) {
				ZeroSyncMode = "Off";
				(Shared -> ZeroSync) = 1;
				tsl::goBack();
				tsl::goBack();
				return true;
			}
			return false;
		});
		list->addItem(clickableListItem3);
		
        frame->setContent(list);

        return frame;
    }
};

class AdvancedGui : public tsl::Gui {
public:
	bool exitPossible = true;
    AdvancedGui() {
		configValid = LOCK::readConfig(&configPath[0]);
		if (R_FAILED(configValid)) {
			if (configValid == 0x202) {
				sprintf(&lockInvalid[0], "Game config file not found\nTID: %016lX\nBID: %016lX", TID, BID);
			}
			else sprintf(&lockInvalid[0], "Game config error: 0x%X", configValid);
		}
		else {
			patchValid = checkFile(&patchPath[0]);
			if (R_FAILED(patchValid)) {
				if (!FileDownloaded) {
					if (R_SUCCEEDED(configValid)) {
						sprintf(&patchChar[0], "Patch file doesn't exist.\nUse \"Convert config to patch file\"\nto make it!");
					}
					else sprintf(&patchChar[0], "Patch file doesn't exist.");
				}
				else {
					sprintf(&patchChar[0], "New config downloaded successfully.\nUse \"Convert config to patch file\"\nto make it applicable!");
				}
			}
			else sprintf(&patchChar[0], "Patch file exists.");
		}
		switch((Shared -> ZeroSync)) {
			case 0:
				ZeroSyncMode = "On";
				break;
			case 1:
				ZeroSyncMode = "Off";
				break;
			case 2:
				ZeroSyncMode = "Semi";
		}
	}

	size_t base_height = 134;

    virtual tsl::elm::Element* createUI() override {
        auto frame = new tsl::elm::OverlayFrame("FPSLocker", "Advanced settings");

		auto list = new tsl::elm::List();

		if ((Shared -> API)) {
			switch((Shared -> API)) {
				case 1: {
					list->addItem(new tsl::elm::CategoryHeader("GPU API Interface: NVN", false));
					
					list->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
						
						renderer->drawString(&nvnBuffers[0], false, x, y+20, 20, renderer->a(0xFFFF));
							
					}), 40);

					if ((Shared -> Buffers) == 2 || (Shared -> SetBuffers) == 2 || (Shared -> ActiveBuffers) == 2) {
						auto *clickableListItem3 = new tsl::elm::MiniListItem("Window Sync Wait", ZeroSyncMode);
						clickableListItem3->setClickListener([](u64 keys) { 
							if ((keys & HidNpadButton_A) && PluginRunning) {
								tsl::changeTo<SyncMode>();
								return true;
							}
							return false;
						});
						list->addItem(clickableListItem3);
					}
					if ((Shared -> Buffers) > 2) {
						auto *clickableListItem3 = new tsl::elm::MiniListItem("Set Buffering");
						clickableListItem3->setClickListener([](u64 keys) { 
							if ((keys & HidNpadButton_A) && PluginRunning) {
								tsl::changeTo<SetBuffers>();
								return true;
							}
							return false;
						});
						list->addItem(clickableListItem3);
					}
					break;
				}
				case 2:
					list->addItem(new tsl::elm::CategoryHeader("GPU API Interface: EGL", false));
					break;
				case 3:
					list->addItem(new tsl::elm::CategoryHeader("GPU API Interface: Vulkan", false));
			}
		}

		list->addItem(new tsl::elm::CategoryHeader("FPSLocker Patches", false));

		if (R_FAILED(configValid)) {
			base_height = 154;
		}

		list->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
			
			if (R_SUCCEEDED(configValid)) {
				
				renderer->drawString("Found valid config file!", false, x, y+20, 20, renderer->a(0xFFFF));
				renderer->drawString(&patchAppliedChar[0], false, x, y+40, 20, renderer->a(0xFFFF));
				if (R_FAILED(patchValid)) {
					renderer->drawString(&patchChar[0], false, x, y+64, 20, renderer->a(0xF99F));
				}
				else renderer->drawString(&patchChar[0], false, x, y+64, 20, renderer->a(0xFFFF));
			}
			else {
				renderer->drawString(&lockInvalid[0], false, x, y+20, 20, renderer->a(0xFFFF));
				renderer->drawString(&patchChar[0], false, x, y+84, 20, renderer->a(0xF99F));
			}
				

		}), base_height);

		if (R_SUCCEEDED(configValid)) {
			list->addItem(new tsl::elm::NoteHeader("Remember to reboot the game after conversion!", true, {0xF, 0x3, 0x3, 0xF}));
			auto *clickableListItem = new tsl::elm::MiniListItem("Convert config to patch file");
			clickableListItem->setClickListener([](u64 keys) { 
				if ((keys & HidNpadButton_A) && PluginRunning) {
					patchValid = LOCK::createPatch(&patchPath[0]);
					if (R_SUCCEEDED(patchValid)) {
						sprintf(&patchChar[0], "Patch file created successfully.\nRestart the game and change\nFPS Target to apply the patch!");
					}
					else sprintf(&patchChar[0], "Error while creating patch: 0x%x", patchValid);
					return true;
				}
				return false;
			});
			list->addItem(clickableListItem);

			auto *clickableListItem2 = new tsl::elm::MiniListItem("Delete patch file");
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
		if (R_FAILED(configValid)) {
			list->addItem(new tsl::elm::NoteHeader("This can take up to 30 seconds.", true, {0xF, 0x3, 0x3, 0xF}));
		}
		auto *clickableListItem4 = new tsl::elm::MiniListItem("Check/download config file");
		clickableListItem4->setClickListener([this](u64 keys) { 
			if ((keys & HidNpadButton_A) && PluginRunning && exitPossible) {
				exitPossible = false;
				sprintf(&patchChar[0], "Checking Warehouse for config...\nExit not possible until finished!");
				threadCreate(&t1, downloadPatch, NULL, NULL, 0x20000, 0x3F, 3);
				threadStart(&t1);
				return true;
			}
			return false;
		});
		list->addItem(clickableListItem4);

		frame->setContent(list);

        return frame;
    }

	virtual void update() override {
		static uint8_t i = 10;

		if (PluginRunning) {
			if (i > 9) {
				if ((Shared -> patchApplied) == 1) {
					sprintf(patchAppliedChar, "Patch was loaded to game.");
				}
				else if ((Shared -> patchApplied) == 2) {
					sprintf(patchAppliedChar, "Master Write was loaded to game.");
				}
				else sprintf(patchAppliedChar, "Plugin didn't apply patch to game.");
				if ((Shared -> API) == 1) {
					if (((Shared -> Buffers) >= 2 && (Shared -> Buffers) <= 4)) {
						sprintf(&nvnBuffers[0], "Set/Active/Available buffers: %d/%d/%d", (Shared -> SetActiveBuffers), (Shared -> ActiveBuffers), (Shared -> Buffers));
					}
				}
				i = 0;
			}
			else i++;
		}
	}

    virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
		if (exitPossible) {
			if (keysDown & HidNpadButton_B) {
				tsl::goBack();
				return true;
			}
		}
		else if (!exitPossible) {
			if (keysDown & HidNpadButton_B)
				return true;
			Result rc = error_code;
			if (rc != UINT32_MAX && rc != 0x404) {
				threadWaitForExit(&t1);
				threadClose(&t1);
				exitPossible = true;
				error_code = UINT32_MAX;
			}
			if (rc == 0x316) {
				sprintf(&patchChar[0], "Connection timeout!");
			}
			else if (rc == 0x212 || rc == 0x312) {
				sprintf(&patchChar[0], "Config is not available! RC: 0x%x", rc);
			}
			else if (rc == 0x404) {
				sprintf(&patchChar[0], "Config is not available!\nChecking Warehouse for more info...\nExit not possible until finished!");
			}
			else if (rc == 0x405) {
				sprintf(&patchChar[0], "Config is not available!\nChecking Warehouse for more info...\nTimeout! It took too long to check.");
			}
			else if (rc == 0x406) {
				sprintf(&patchChar[0], "Config is not available!\nChecking Warehouse for more info...\nConnection error!");
			}
			else if (rc == 0x104) {
				sprintf(&patchChar[0], "No new config available.");
			}
			else if (rc == 0x412) {
				sprintf(&patchChar[0], "Internet connection not available!");
			}
			else if (rc == 0x1001) {
				sprintf(&patchChar[0], "Patch is not needed for this game!");
			}
			else if (rc == 0x1002) {
				sprintf(&patchChar[0], "This game is not listed in Warehouse!");
			}
			else if (rc == 0x1003) {
				sprintf(&patchChar[0], "This game is listed in Warehouse,\nbut with different version. Other\nversion doesn't need a patch, your\nversion maybe doesn't need it too!");
			}
			else if (rc == 0x1004) {
				sprintf(&patchChar[0], "This game is listed in Warehouse,\nbut with different version.\nOther version recommends patch,\nbut config is not available even for it!");
			}
			else if (rc == 0x1005) {
				sprintf(&patchChar[0], "This game is listed in Warehouse,\nbut with different version.\nOther version has config available!");
			}
			else if (rc == 0x1006) {
				sprintf(&patchChar[0], "This game is listed in Warehouse\nwith patch recommended for this\nversion, but config is not available!");
			}
			else if (R_SUCCEEDED(rc)) {
				FILE* fp = fopen(patchPath, "rb");
				if (fp) {
					fclose(fp);
					remove(patchPath);
				}
				tsl::goBack();
				tsl::changeTo<AdvancedGui>();
				return true;
			}
			else if (rc != UINT32_MAX) {
				sprintf(&patchChar[0], "Connection error! RC: 0x%x", rc);
			}
		}
        return false;   // Return true here to signal the inputs have been consumed
    }
};