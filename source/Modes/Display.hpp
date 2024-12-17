class DockedFrameskipGui : public tsl::Gui {
public:
	int y_1;
	int x_1;
	int height = 480;
	int block_width = 20;
	int block_height = 40;
	int columns = 18;
	bool block;
    uint64_t tick;
    bool state;
    DockedFrameskipGui() {
		y_1 = 0;
		x_1 = 0;
        tick = 0;
		block = false;
        state = false;
	}

    virtual tsl::elm::Element* createUI() override {
        auto frame = new tsl::elm::OverlayFrame("FPSLocker", "Frameskip tester");

		auto list = new tsl::elm::List();

		list->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
            if (!state) {
                renderer->drawString(   "How to use it:\n"
                                        "1. Get a camera with options to\n"
                                        "manually set shutter speed and ISO.\n"
                                        "2. Set shutter speed to 1/10s\n"
                                        "or longer, and ISO so it's not\n"
                                        "too bright or dark\n"
                                        "(usually around 50 for 1/10s).\n"
                                        "3. Press \uE0E0 to continue.\n"
                                        "4. Take picture of display.\n"
                                        "5. If all blocks except for first\n"
                                        "and last are unevenly bright,\n"
                                        "your display doesn't support\n"
                                        "natively your current refresh rate\n"
                                        "and it's running at some other\n"
                                        "refresh rate.\n\n"
                                        "Still take into consideration that\n"
                                        "even if your display is frameskipping,\n"
                                        "it still works miles better than\n"
                                        "using lower FPS target that doesn't\n"
                                        "match your refresh rate because\n"
                                        "hardware solution is the best way\n"
                                        "to divide evenly frametimes.", false, x, y+20, 20, renderer->a(0xFFFF));
            }
			else if (!block) {
				renderer->fillScreen(0xF000);
				renderer->drawRect(x+x_1, y+y_1, block_width, block_height, renderer->a(0xFFFF));
				renderer->drawString("Press \uE0E1 to exit", false, x+20, y+height+20, 20, renderer->a(0xFFFF));
			}
			else {
				renderer->drawString("Rendering takes too long!\nClose game, go to home screen,\ntry again.", false, x, y+20, 20, renderer->a(0xFFFF));
			}
			
		}), height+40);		
		
		frame->setContent(list);

        return frame;
    }

	// Called once every frame to handle inputs not handled by other UI elements
	virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
        if (!state && (keysDown & HidNpadButton_A)) {
            state = true;
            return true;
        }
		if (state && !block) {
			y_1 += block_height;
			y_1 %= height;
			if (y_1 == 0) {
				x_1 += block_width;
				x_1 %= (block_width*columns);
			}
			if (!tick) {
				tick = svcGetSystemTick();
				return false;
			}
			if ((svcGetSystemTick() - tick) > ((19200000 / *refreshRate_shared) * 2)) {
				block = true;
			}
			else tick = svcGetSystemTick();
		}
		return false;   // Return true here to singal the inputs have been consumed
	}
};

class DockedManualGui;

class DockedWizardGui : public tsl::Gui {
public:
	uint64_t tick;
    size_t i;
	char Docked_c[256] ="This menu will go through all\n"
						"supported refresh rates below 60 Hz:\n"
						"40, 45, 50, 55. Press button you are\n"
						"asked for to confirm that it works.\n"
						"If nothing is pressed in 15 seconds,\n"
						"it will check next refresh rate.";

	char PressButton[40] = "To start press X.";
	DockedModeRefreshRateAllowed rr;
	DockedModeRefreshRateAllowed rr_default;
	DockedAdditionalSettings as;
    DockedWizardGui() {
		LoadDockedModeAllowedSave(rr_default, as);
		memset(&rr, 1, sizeof(rr));
		tick = 0;
        i = 0;
	}

    virtual tsl::elm::Element* createUI() override {
        auto frame = new tsl::elm::OverlayFrame("FPSLocker", "Docked display settings wizard");

		auto list = new tsl::elm::List();

		list->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {

			renderer->drawString(Docked_c, false, x, y+20, 20, renderer->a(0xFFFF));

			renderer->drawString(PressButton, false, x, y+160, 20, renderer->a(0xFFFF));
			
		}), 200);		
		
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
		if (keysHeld & HidNpadButton_B) {
			if (R_SUCCEEDED(SaltySD_Connect())) {
				SaltySD_SetAllowedDockedRefreshRates(rr_default);
				svcSleepThread(100'000);
				SaltySD_SetDisplayRefreshRate(60);
				svcSleepThread(100'000);
				SaltySD_Term();
			}
			tsl::goBack();
			return true;
		}
		static u64 keyCheck = HidNpadButton_ZL;
		if ((keysHeld & HidNpadButton_X) && !tick) {
			tick = svcGetSystemTick();
			if (R_SUCCEEDED(SaltySD_Connect())) {
				SaltySD_SetAllowedDockedRefreshRates(rr);
				svcSleepThread(100'000);
				SaltySD_SetDisplayRefreshRate(40);
				svcSleepThread(100'000);
				SaltySD_Term();
				snprintf(PressButton, sizeof(PressButton), "Press ZL to confirm 40 Hz is working.");
			}
			return true;
		}
		if (tick) {
			if (DockedModeRefreshRateAllowedValues[i] == 60) {
				if (R_SUCCEEDED(SaltySD_Connect())) {
					SaltySD_SetAllowedDockedRefreshRates(rr);
					svcSleepThread(100'000);
					SaltySD_Term();
				}
				SaveDockedModeAllowedSave(rr, as);
				tsl::goBack();
				tsl::changeTo<DockedManualGui>();
				return true;
			}
			if (svcGetSystemTick() - tick < (15 * 19200000)) {
				if (keysHeld & keyCheck) {
					rr[i] = true;
					i++;
					if (R_SUCCEEDED(SaltySD_Connect())) {
						SaltySD_SetDisplayRefreshRate(DockedModeRefreshRateAllowedValues[i]);
						svcSleepThread(100'000);
						SaltySD_Term();
					}
					if (i % 1 == 0) {
						keyCheck = HidNpadButton_X;
						snprintf(PressButton, sizeof(PressButton), "Press X to confirm %d Hz is working.", DockedModeRefreshRateAllowedValues[i]);
					}
					if (i % 3 == 0) {
						keyCheck = HidNpadButton_Y;
						snprintf(PressButton, sizeof(PressButton), "Press Y to confirm %d Hz is working.", DockedModeRefreshRateAllowedValues[i]);
					}
					if (i % 2 == 0) {
						keyCheck = HidNpadButton_ZR;
						snprintf(PressButton, sizeof(PressButton), "Press ZR to confirm %d Hz is working.", DockedModeRefreshRateAllowedValues[i]);
					}
					tick = svcGetSystemTick();
					return true;
				}
			}
			else {
				rr[i] = false;
				i++;
				if (i % 1 == 0) {
					keyCheck = HidNpadButton_X;
					snprintf(PressButton, sizeof(PressButton), "Press X to confirm %d Hz is working.", DockedModeRefreshRateAllowedValues[i]);
				}
				if (i % 3 == 0) {
					keyCheck = HidNpadButton_Y;
					snprintf(PressButton, sizeof(PressButton), "Press Y to confirm %d Hz is working.", DockedModeRefreshRateAllowedValues[i]);
				}
				if (i % 2 == 0) {
					keyCheck = HidNpadButton_ZR;
					snprintf(PressButton, sizeof(PressButton), "Press ZR to confirm %d Hz is working.", DockedModeRefreshRateAllowedValues[i]);
				}
				if (R_SUCCEEDED(SaltySD_Connect())) {
					SaltySD_SetDisplayRefreshRate(DockedModeRefreshRateAllowedValues[i]);
					svcSleepThread(100'000);
					SaltySD_Term();
				}
				tick = svcGetSystemTick();
			}
		}
		return false;   // Return true here to singal the inputs have been consumed
	}
};

class DockedManualGui : public tsl::Gui {
public:
	uint32_t crc = 0;
	DockedModeRefreshRateAllowed rr = {0};
	DockedAdditionalSettings as;
    DockedManualGui() {
		LoadDockedModeAllowedSave(rr, as);
	}

    virtual tsl::elm::Element* createUI() override {
        auto frame = new tsl::elm::OverlayFrame("FPSLocker", "Docked display manual settings");

		auto list = new tsl::elm::List();

		size_t i = 0;
		while (i < 4) {
			char Hz[] = "120 Hz";
			snprintf(Hz, sizeof(Hz), "%d Hz", DockedModeRefreshRateAllowedValues[i]);
			auto *clickableListItem = new tsl::elm::ToggleListItem(Hz, rr[i]);
			clickableListItem->setClickListener([this, i](u64 keys) { 
				if (keys & HidNpadButton_A) {
					rr[i] = !rr[i];
					return true;
				}
				return false;
			});
			list->addItem(clickableListItem);	
			i++;
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
				tsl::goBack();
				return true;
			}
		}
		smExit();
		if (keysHeld & HidNpadButton_B) {
			if (R_SUCCEEDED(SaltySD_Connect())) {
				SaveDockedModeAllowedSave(rr, as);
				SaltySD_SetAllowedDockedRefreshRates(rr);
				svcSleepThread(100'000);
				SaltySD_Term();
			}
			tsl::goBack();
			return true;
		}
		return false;   // Return true here to singal the inputs have been consumed
	}
};

class DockedAdditionalGui : public tsl::Gui {
public:
	uint32_t crc = 0;
	DockedModeRefreshRateAllowed rr = {0};
	DockedAdditionalSettings as;
    DockedAdditionalGui() {
		LoadDockedModeAllowedSave(rr, as);
	}

    virtual tsl::elm::Element* createUI() override {
        auto frame = new tsl::elm::OverlayFrame("FPSLocker", "Docked display additional settings");

		auto list = new tsl::elm::List();

		auto *clickableListItem4 = new tsl::elm::ToggleListItem("Allow patches to force 60 Hz", !as.dontForce60InDocked);
		clickableListItem4->setClickListener([this](u64 keys) { 
			if (keys & HidNpadButton_A) {
				as.dontForce60InDocked = !as.dontForce60InDocked;
				if (R_SUCCEEDED(SaltySD_Connect())) {
					SaltySD_SetDontForce60InDocked(as.dontForce60InDocked);
					SaltySD_Term();
				}
				SaveDockedModeAllowedSave(rr, as);
				return true;
			}
			return false;
		});

		list->addItem(clickableListItem4);

		auto *clickableListItem5 = new tsl::elm::ToggleListItem("Use lowest refresh rate for unmatched FPS targets", as.fpsTargetWithoutRRMatchLowest);
		clickableListItem5->setClickListener([this](u64 keys) { 
			if (keys & HidNpadButton_A) {
				as.fpsTargetWithoutRRMatchLowest = !as.fpsTargetWithoutRRMatchLowest;
				if (R_SUCCEEDED(SaltySD_Connect())) {
					SaltySD_SetMatchLowestRR(as.fpsTargetWithoutRRMatchLowest);
					SaltySD_Term();
				}
				SaveDockedModeAllowedSave(rr, as);
				return true;
			}
			return false;
		});

		list->addItem(clickableListItem5);
		
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
				tsl::goBack();
				return true;
			}
		}
		smExit();
		return false;   // Return true here to singal the inputs have been consumed
	}
};

class DockedGui : public tsl::Gui {
private:
	char Docked_c[256] = "";
	DockedModeRefreshRateAllowed rr;
	DockedAdditionalSettings as;
public:
    DockedGui() {
		mkdir("sdmc:/SaltySD/plugins/FPSLocker/", 777);
		mkdir("sdmc:/SaltySD/plugins/FPSLocker/ExtDisplays/", 777);
		LoadDockedModeAllowedSave(rr, as);
		smInitialize();
		setsysInitialize();
		SetSysEdid2 edid2 = {0};
		if (R_SUCCEEDED(setsysGetEdid2(setsysGetServiceSession(), &edid2))) {
			uint8_t highestRefreshRate = 0;
			for (int i = 0; i < 2; i++) {
				auto td = edid2.edid.timing_descriptor[i];
				uint32_t pixel_clock = td.pixel_clock * 10000;
				if (!pixel_clock) continue;
				uint32_t h_total = ((uint32_t)td.horizontal_active_pixels_msb << 8 | td.horizontal_active_pixels_lsb) + ((uint32_t)td.horizontal_blanking_pixels_msb << 8 | td.horizontal_blanking_pixels_lsb);
				uint32_t v_total = ((uint32_t)td.vertical_active_lines_msb << 8 | td.vertical_active_lines_lsb) + ((uint32_t)td.vertical_blanking_lines_msb << 8 | td.vertical_blanking_lines_lsb);
				uint8_t refreshRate = (uint8_t)round((float)pixel_clock / (float)(h_total * v_total));
				if (refreshRate > highestRefreshRate) highestRefreshRate = refreshRate;
			}
			SetSysModeLine* modes = (SetSysModeLine*)((uintptr_t)(&edid2.edid) + 0x80 + edid2.edid.dtd_start);
			for (int i = 0; i < 5; i++) {
				auto td = modes[i];
				uint32_t pixel_clock = td.pixel_clock * 10000;
				if (!pixel_clock) continue;
				uint32_t h_total = ((uint32_t)td.horizontal_active_pixels_msb << 8 | td.horizontal_active_pixels_lsb) + ((uint32_t)td.horizontal_blanking_pixels_msb << 8 | td.horizontal_blanking_pixels_lsb);
				uint32_t v_total = ((uint32_t)td.vertical_active_lines_msb << 8 | td.vertical_active_lines_lsb) + ((uint32_t)td.vertical_blanking_lines_msb << 8 | td.vertical_blanking_lines_lsb);
				uint8_t refreshRate = (uint8_t)round((float)pixel_clock / (float)(h_total * v_total));
				if (refreshRate > highestRefreshRate) highestRefreshRate = refreshRate;
			}
			snprintf(Docked_c, sizeof(Docked_c), "Reported max refresh rate: %d Hz", highestRefreshRate);
		}
		setsysExit();
		smExit();
	}

	size_t base_height = 128;
	bool block = false;

    virtual tsl::elm::Element* createUI() override {
        auto frame = new tsl::elm::OverlayFrame("FPSLocker", "Docked display settings");

		auto list = new tsl::elm::List();

		list->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {

			renderer->drawString(Docked_c, false, x, y+20, 20, renderer->a(0xFFFF));
			
		}), 65);

		auto *clickableListItem1 = new tsl::elm::ListItem2("Allowed refresh rates");
		clickableListItem1->setClickListener([this](u64 keys) { 
			if ((keys & HidNpadButton_A) && !block) {
				tsl::changeTo<DockedManualGui>();
				return true;
			}
			return false;
		});

		list->addItem(clickableListItem1);

		auto *clickableListItem2 = new tsl::elm::ListItem2("Refresh rate wizard");
		clickableListItem2->setClickListener([this](u64 keys) { 
			if ((keys & HidNpadButton_A) && !block) {
				tsl::changeTo<DockedWizardGui>();
				return true;
			}
			return false;
		});

		list->addItem(clickableListItem2);

		auto *clickableListItem3 = new tsl::elm::ListItem2("Frameskip tester");
		clickableListItem3->setClickListener([this](u64 keys) { 
			if ((keys & HidNpadButton_A)) {
				tsl::changeTo<DockedFrameskipGui>();
				return true;
			}
			return false;
		});

		list->addItem(clickableListItem3);


		auto *clickableListItem4 = new tsl::elm::ListItem2("Additional settings");
		clickableListItem4->setClickListener([this](u64 keys) { 
			if ((keys & HidNpadButton_A) && !block) {
				tsl::changeTo<DockedAdditionalGui>();
				return true;
			}
			return false;
		});

		list->addItem(clickableListItem4);
		
		frame->setContent(list);

        return frame;
    }

	virtual void update() override {
		if (!block) tsl::hlp::doWithSmSession([this]{
			if (R_SUCCEEDED(apmInitialize())) {
				ApmPerformanceMode mode = ApmPerformanceMode_Invalid;
				apmGetPerformanceMode(&mode);
				if (mode != ApmPerformanceMode_Boost ) {
					block = true;
					snprintf(Docked_c, sizeof(Docked_c),	"You are not in docked mode.\n"
															"Go back, put your Switch to dock\n"
															"and come back.");
				}
				apmExit();
			}
		});
	}
};

class DockedRefreshRateChangeGui : public tsl::Gui {
public:
	DockedModeRefreshRateAllowed rr;
	DockedAdditionalSettings as;
	DockedRefreshRateChangeGui () {
		LoadDockedModeAllowedSave(rr, as);
	}

	// Called when this Gui gets loaded to create the UI
	// Allocate all elements on the heap. libtesla will make sure to clean them up when not needed anymore
	virtual tsl::elm::Element* createUI() override {
		// A OverlayFrame is the base element every overlay consists of. This will draw the default Title and Subtitle.
		// If you need more information in the header or want to change it's look, use a HeaderOverlayFrame.
		auto frame = new tsl::elm::OverlayFrame("FPSLocker", "Change Refresh Rate");

		// A list that can contain sub elements and handles scrolling
		auto list = new tsl::elm::List();

		for (size_t i = 0; i < sizeof(rr); i++) {
			if (rr[i] == false)
				continue;
			char Hz[] = "254 Hz";
			snprintf(Hz, sizeof(Hz), "%d Hz", DockedModeRefreshRateAllowedValues[i]);
			auto *clickableListItem = new tsl::elm::MiniListItem(Hz);
			clickableListItem->setClickListener([this, i](u64 keys) { 
				if (keys & HidNpadButton_A) {
					if (!oldSalty) {
						if (R_SUCCEEDED(SaltySD_Connect())) {
							SaltySD_SetDisplayRefreshRate(DockedModeRefreshRateAllowedValues[i]);
							SaltySD_Term();
							refreshRate_g = DockedModeRefreshRateAllowedValues[i];
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

class DisplayGui : public tsl::Gui {
private:
	char refreshRate_c[32] = "";
	char oled_c[48] = "Not available for Switch OLED\nin handheld mode.";
	bool isDocked = false;
	ApmPerformanceMode entry_mode = ApmPerformanceMode_Invalid;
public:
    DisplayGui() {
		if (isLite) entry_mode = ApmPerformanceMode_Normal;
		else {
			smInitialize();
			if (R_SUCCEEDED(apmInitialize())) {
				apmGetPerformanceMode(&entry_mode);
				apmExit();
			}
			else entry_mode = ApmPerformanceMode_Normal;
			smExit();
		}
	}
	size_t base_height = 128;

    virtual tsl::elm::Element* createUI() override {
        auto frame = new tsl::elm::OverlayFrame("FPSLocker", "Display settings");

		auto list = new tsl::elm::List();

		list->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {

			renderer->drawString(refreshRate_c, false, x, y+20, 20, renderer->a(0xFFFF));
			if (isOLED && !isDocked) renderer->drawString(oled_c, false, x, y+50, 20, renderer->a(0xFFFF));
			
		}), 90);

		if (!displaySync) {
			if (entry_mode == ApmPerformanceMode_Normal) {
				auto *clickableListItem = new tsl::elm::ListItem2("Increase Refresh Rate");
				clickableListItem->setClickListener([this](u64 keys) { 
					if ((keys & HidNpadButton_A) && (!isOLED || isDocked)) {
						if ((refreshRate_g >= 40) && (refreshRate_g < 60)) {
							if (R_SUCCEEDED(SaltySD_Connect())) {
								refreshRate_g += 5;
								SaltySD_SetDisplayRefreshRate(refreshRate_g);
								SaltySD_Term();
								if (Shared) (Shared -> displaySync) = refreshRate_g;
							}
						}
						return true;
					}
					return false;
				});

				list->addItem(clickableListItem);

				auto *clickableListItem2 = new tsl::elm::ListItem2("Decrease Refresh Rate");
				clickableListItem2->setClickListener([this](u64 keys) { 
					if ((keys & HidNpadButton_A) && (!isOLED || isDocked)) {
						if (refreshRate_g > 40) {
							if (R_SUCCEEDED(SaltySD_Connect())) {
								refreshRate_g -= 5;
								SaltySD_SetDisplayRefreshRate(refreshRate_g);
								if (Shared) (Shared -> displaySync) = refreshRate_g;
								SaltySD_Term();
							}
						}
						return true;
					}
					return false;
				});

				list->addItem(clickableListItem2);
			}
			else if (entry_mode == ApmPerformanceMode_Boost) {
				auto *clickableListItem2 = new tsl::elm::ListItem2("Change Refresh Rate");
				clickableListItem2->setClickListener([](u64 keys) { 
					if (keys & HidNpadButton_A) {
						tsl::changeTo<DockedRefreshRateChangeGui>();
						return true;
					}
					return false;
				});	
				list->addItem(clickableListItem2);	
			}
		}

		if (!oldSalty) {
			list->addItem(new tsl::elm::CategoryHeader("Match refresh rate with FPS Target.", true));
			auto *clickableListItem3 = new tsl::elm::ToggleListItem("Display Sync", displaySync);
			clickableListItem3->setClickListener([this](u64 keys) { 
				if (keys & HidNpadButton_A) {
					if (R_SUCCEEDED(SaltySD_Connect())) {
						SaltySD_SetDisplaySync(!displaySync);
						svcSleepThread(100'000);
						if (!isOLED || entry_mode == ApmPerformanceMode_Boost) {
							u64 PID = 0;
							Result rc = pmdmntGetApplicationProcessId(&PID);
							if (R_SUCCEEDED(rc) && Shared) {
								if (!displaySync == true && (Shared -> FPSlocked) < 40) {
									SaltySD_SetDisplayRefreshRate(60);
									(Shared -> displaySync) = 0;
								}
								else if (!displaySync == true) {
									SaltySD_SetDisplayRefreshRate((Shared -> FPSlocked));
									(Shared -> displaySync) = (Shared -> FPSlocked);
								}
								else {
									(Shared -> displaySync) = 0;
								}
							}
							else if (!displaySync == true && (R_FAILED(rc) || !PluginRunning)) {
								SaltySD_SetDisplayRefreshRate(60);
							}
						}
						SaltySD_Term();
						displaySync = !displaySync;
					}
					tsl::goBack();
					tsl::changeTo<DisplayGui>();
					return true;
				}
				return false;
			});

			list->addItem(clickableListItem3);

			if (!isLite) {
				auto *clickableListItem4 = new tsl::elm::ListItem2("Docked Settings");
				clickableListItem4->setClickListener([this](u64 keys) { 
					if ((keys & HidNpadButton_A)) {
						tsl::changeTo<DockedGui>();
						return true;
					}
					return false;
				});

				list->addItem(clickableListItem4);
			}
		}
		
		frame->setContent(list);

        return frame;
    }

	virtual void update() override {
		refreshRate_g = *refreshRate_shared;
		snprintf(refreshRate_c, sizeof(refreshRate_c), "Display Refresh Rate: %d Hz", refreshRate_g);
	}

	// Called once every frame to handle inputs not handled by other UI elements
	virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
		if (!isLite) {
			smInitialize();
			if (R_SUCCEEDED(apmInitialize())) {
				ApmPerformanceMode mode = ApmPerformanceMode_Invalid;
				apmGetPerformanceMode(&mode);
				apmExit();
				if (mode != entry_mode) {
					smExit();
					tsl::goBack();
					tsl::changeTo<DisplayGui>();
					return true;
				}
			}
			smExit();
		}
		return false;   // Return true here to singal the inputs have been consumed
	}
};

class WarningDisplayGui : public tsl::Gui {
private:
	uint8_t refreshRate = 0;
	std::string Warning =	"THIS IS EXPERIMENTAL FUNCTION!\n\n"
							"It can cause irreparable damage\n"
							"to your display.\n\n"
							"By pressing Accept you are taking\n"
							"full responsibility for anything\n"
							"that can occur because of this tool.";
public:
    WarningDisplayGui() {}

	size_t base_height = 128;

    virtual tsl::elm::Element* createUI() override {
        auto frame = new tsl::elm::OverlayFrame("FPSLocker", "Display settings warning");

		auto list = new tsl::elm::List();

		list->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
			renderer->drawString(Warning.c_str(), false, x, y+20, 20, renderer->a(0xFFFF));
		}), 200);

		auto *clickableListItem1 = new tsl::elm::ListItem2("Decline");
		clickableListItem1->setClickListener([this](u64 keys) { 
			if (keys & HidNpadButton_A) {
				tsl::goBack();
				return true;
			}
			return false;
		});

		list->addItem(clickableListItem1);

		auto *clickableListItem2 = new tsl::elm::ListItem2("Accept");
		clickableListItem2->setClickListener([this](u64 keys) { 
			if ((keys & HidNpadButton_A)) {
				tsl::goBack();
				tsl::changeTo<DisplayGui>();
				return true;
			}
			return false;
		});

		list->addItem(clickableListItem2);
		
		frame->setContent(list);

        return frame;
    }
};