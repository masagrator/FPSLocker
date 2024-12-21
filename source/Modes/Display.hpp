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
			tsl::cfg::FPS60Lock = false;
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
				tsl::cfg::FPS60Lock = true;
			}
			else tick = svcGetSystemTick();
		}
		if (keysDown & HidNpadButton_B) {
			tsl::cfg::FPS60Lock = true;
			tsl::goBack();
			return true;
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
	uint32_t m_height;
	uint8_t m_maxRefreshRate;
    DockedWizardGui(uint8_t maxRefreshRate, uint32_t height) {
		if (!height) height = 1080;
		if (!maxRefreshRate) maxRefreshRate = 60;
		m_maxRefreshRate = maxRefreshRate;
		m_height = height;
		LoadDockedModeAllowedSave(rr_default, as, height);
		memcpy(&rr, &rr_default, sizeof(rr));
		memset(&rr, 1, 5);
		tick = 0;
        i = 0;
	}

    virtual tsl::elm::Element* createUI() override {
        auto frame = new tsl::elm::OverlayFrame("FPSLocker", "Docked underclocking display wizard");

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
				SaltySD_SetAllowedDockedRefreshRates(rr_default, m_height);
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
				SaltySD_SetAllowedDockedRefreshRates(rr, m_height);
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
					SaltySD_SetAllowedDockedRefreshRates(rr, m_height);
					svcSleepThread(100'000);
					SaltySD_Term();
				}
				SaveDockedModeAllowedSave(rr, as, m_height);
				tsl::goBack();
				tsl::changeTo<DockedManualGui>(m_maxRefreshRate, m_height);
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

class DockedOverWizardGui : public tsl::Gui {
public:
	uint64_t tick;
    size_t i;
	char Docked_c[256] ="";

	char PressButton[40] = "To start press X.";
	DockedModeRefreshRateAllowed rr;
	DockedModeRefreshRateAllowed rr_default;
	DockedAdditionalSettings as;
	uint32_t m_height;
	uint8_t m_maxRefreshRate;
	uint16_t delay_s = 10;
    DockedOverWizardGui(uint8_t maxRefreshRate, uint32_t height) {
		if (!height) height = 1080;
		m_maxRefreshRate = maxRefreshRate;
		m_height = height;
		LoadDockedModeAllowedSave(rr_default, as, m_height);
		memcpy(&rr, &rr_default, sizeof(rr));
		tick = 0;
        i = 5;
		uint8_t times = 0;
		for (size_t x = 5; x < sizeof(DockedModeRefreshRateAllowedValues); x++) {
			if (DockedModeRefreshRateAllowedValues[i] > maxRefreshRate)
				rr[x] = false;
			else {
				rr[x] = true;
				times++;
			}
		}
		snprintf(Docked_c, sizeof(Docked_c), "This menu will go through all\n"
											"supported refresh rates above 60 Hz\n"
											"up to %d Hz for %dp.\n"
											"Press button you are asked for\n"
											"to confirm that it works.\n"
											"If nothing is pressed in 10 seconds,\n"
											"it will check next refresh rate.\n"
											"This can take up to %d seconds.", maxRefreshRate, height, delay_s * times);
	}

    virtual tsl::elm::Element* createUI() override {
        auto frame = new tsl::elm::OverlayFrame("FPSLocker", "Docked overclocking display wizard");

		auto list = new tsl::elm::List();

		list->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {

			renderer->drawString(Docked_c, false, x, y+20, 20, renderer->a(0xFFFF));

			renderer->drawString(PressButton, false, x, y+200, 20, renderer->a(0xFFFF));
			
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
				SaltySD_SetAllowedDockedRefreshRates(rr_default, m_height);
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
				SaltySD_SetAllowedDockedRefreshRates(rr, m_height);
				svcSleepThread(100'000);
				SaltySD_SetDisplayRefreshRate(DockedModeRefreshRateAllowedValues[i]);
				svcSleepThread(100'000);
				SaltySD_Term();
				snprintf(PressButton, sizeof(PressButton), "Press ZL to confirm %d Hz is working.", DockedModeRefreshRateAllowedValues[i]);
			}
			return true;
		}
		if (tick) {
			if (DockedModeRefreshRateAllowedValues[i] > m_maxRefreshRate) {
				if (R_SUCCEEDED(SaltySD_Connect())) {
					SaltySD_SetAllowedDockedRefreshRates(rr, m_height);
					svcSleepThread(100'000);
					SaltySD_SetDisplayRefreshRate(60);
					svcSleepThread(100'000);
					SaltySD_Term();
				}
				SaveDockedModeAllowedSave(rr, as, m_height);
				tsl::goBack();
				tsl::changeTo<DockedManualGui>(m_maxRefreshRate, m_height);
				return true;
			}
			if (svcGetSystemTick() - tick < (delay_s * 19200000)) {
				if (keysHeld & keyCheck) {
					rr[i] = true;
					i++;
					if (R_SUCCEEDED(SaltySD_Connect())) {
						Result rc = SaltySD_SetDisplayRefreshRate(DockedModeRefreshRateAllowedValues[i]);
						svcSleepThread(100'000);
						if (R_FAILED(rc)) {
							while(i < sizeof(DockedModeRefreshRateAllowedValues) && DockedModeRefreshRateAllowedValues[i] <= m_maxRefreshRate) {
								rr[i] = false;
								i++;
								tick = svcGetSystemTick();
								svcSleepThread(100'000);
								if (R_SUCCEEDED(SaltySD_SetDisplayRefreshRate(DockedModeRefreshRateAllowedValues[i])))
									break;
							}
							SaltySD_Term();
							return true;
						}
						else SaltySD_Term();
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
	uint8_t m_maxRefreshRate;
	uint32_t m_height;
    DockedManualGui(uint8_t maxRefreshRate, uint32_t height) {
		if (maxRefreshRate < 60) maxRefreshRate = 60;
		if (!height) height = 1080;
		m_maxRefreshRate = maxRefreshRate;
		m_height = height;
		LoadDockedModeAllowedSave(rr, as, m_height);
		for (size_t i = 0; i < sizeof(DockedModeRefreshRateAllowed); i++) {
			if (DockedModeRefreshRateAllowedValues[i] > m_maxRefreshRate)
				rr[i] = false;
		}
	}

    virtual tsl::elm::Element* createUI() override {
        auto frame = new tsl::elm::OverlayFrame("FPSLocker", "Docked display manual settings");

		auto list = new tsl::elm::List();

		size_t i = 0;
		while (i < sizeof(DockedModeRefreshRateAllowedValues) && DockedModeRefreshRateAllowedValues[i] <= m_maxRefreshRate) {
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
				SaveDockedModeAllowedSave(rr, as, m_height);
				SaltySD_SetAllowedDockedRefreshRates(rr, m_height);
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
		LoadDockedModeAllowedSave(rr, as, 0);
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
				SaveDockedModeAllowedSave(rr, as, 0);
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
				SaveDockedModeAllowedSave(rr, as, 0);
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
	u32 width;
	u32 height;
	bool blockRes = false;
	uint8_t maxRefreshRate;
public:
    DockedGui() {
		mkdir("sdmc:/SaltySD/plugins/", 777);
		mkdir("sdmc:/SaltySD/plugins/FPSLocker/", 777);
		mkdir("sdmc:/SaltySD/plugins/FPSLocker/ExtDisplays/", 777);
		width = 0;
		height = 0;
		maxRefreshRate = 0;
		if (R_SUCCEEDED(SaltySD_Connect())) {
			SaltySD_GetDockedOutputResolution(&width, &height);
			SaltySD_Term();
		}
		if (!(width == 1280 && height == 720) && !(width == 1920 && height == 1080))
			blockRes = true;
		LoadDockedModeAllowedSave(rr, as, height);
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
			maxRefreshRate = highestRefreshRate;
			if (!isOLED && highestRefreshRate > 120 && height == 1080) maxRefreshRate = 120;
			snprintf(Docked_c, sizeof(Docked_c), "Reported max refresh rate: %d Hz\n%sOutput: %dx%d\n%s", highestRefreshRate, ((highestRefreshRate != maxRefreshRate) ? "V1/V2 at 1080p are blocked\nto 120 Hz\n" : ""), width, height, ((width == 1280 && height == 720) || (width == 1920 && height == 1080)) ? "" : "Nonstandard resolutions are\nunsupported!");
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
			
		}), 105);

		auto *clickableListItem1 = new tsl::elm::ListItem2("Allowed refresh rates");
		clickableListItem1->setClickListener([this](u64 keys) { 
			if ((keys & HidNpadButton_A) && !block && !blockRes) {
				tsl::changeTo<DockedManualGui>(maxRefreshRate, height);
				return true;
			}
			return false;
		});

		list->addItem(clickableListItem1);

		auto *clickableListItem2 = new tsl::elm::ListItem2("Underclock wizard");
		clickableListItem2->setClickListener([this](u64 keys) { 
			if ((keys & HidNpadButton_A) && !block && !blockRes) {
				tsl::changeTo<DockedWizardGui>(maxRefreshRate, height);
				return true;
			}
			return false;
		});

		list->addItem(clickableListItem2);

		if (maxRefreshRate > 60) {
			auto *clickableListItem5 = new tsl::elm::ListItem2("Overclock wizard");
			clickableListItem5->setClickListener([this](u64 keys) { 
				if ((keys & HidNpadButton_A) && !block && !blockRes) {
					tsl::changeTo<DockedOverWizardGui>(maxRefreshRate, height);
					return true;
				}
				return false;
			});

			list->addItem(clickableListItem5);
		}

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
			if ((keys & HidNpadButton_A) && !block && !blockRes) {
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
		u32 width = 0;
		u32 height = 0;
		if (R_SUCCEEDED(SaltySD_Connect())) {
			SaltySD_GetDockedOutputResolution(&width, &height);
			SaltySD_Term();
		}
		LoadDockedModeAllowedSave(rr, as, height);
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

class HandheldMinGui : public tsl::Gui {
public:
    HandheldMinGui() {
	}

    virtual tsl::elm::Element* createUI() override {
        auto frame = new tsl::elm::OverlayFrame("FPSLocker", "Handheld Minimum Refresh Rate");

		auto list = new tsl::elm::List();

		for (int i = 40; i <= 60; i += 5) {
			char Hz[] = "60 Hz";
			snprintf(Hz, sizeof(Hz), "%d Hz", i);
			auto *clickableListItem = new tsl::elm::ListItem2(Hz);
			clickableListItem->setClickListener([i](u64 keys) { 
				if (keys & HidNpadButton_A) {
					HandheldModeRefreshRateAllowed.min = i;
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

};

class HandheldMaxGui : public tsl::Gui {
public:
    HandheldMaxGui() {
	}

    virtual tsl::elm::Element* createUI() override {
        auto frame = new tsl::elm::OverlayFrame("FPSLocker", "Handheld Maximum Refresh Rate");

		auto list = new tsl::elm::List();

		for (int i = 60; i <= 75; i += 5) {
			char Hz[] = "60 Hz";
			snprintf(Hz, sizeof(Hz), "%d Hz", i);
			auto *clickableListItem = new tsl::elm::ListItem2(Hz);
			clickableListItem->setClickListener([i](u64 keys) { 
				if (keys & HidNpadButton_A) {
					HandheldModeRefreshRateAllowed.max = i;
					tsl::goBack();
					return true;
				}
				return false;
			});
			list->addItem(clickableListItem);	
			if (i == 70) {
				auto *clickableListItem2 = new tsl::elm::ListItem2("72 Hz");
				clickableListItem2->setClickListener([](u64 keys) { 
					if (keys & HidNpadButton_A) {
						HandheldModeRefreshRateAllowed.max = 72;
						tsl::goBack();
						return true;
					}
					return false;
				});
				list->addItem(clickableListItem2);
			}			
		}
		
		frame->setContent(list);

        return frame;
    }

};

class HandheldGui : public tsl::Gui {
private:
	char Handheld_c[256] = "";
	uint8_t model = 0;
public:
    HandheldGui() {
		mkdir("sdmc:/SaltySD/plugins/", 777);
		mkdir("sdmc:/SaltySD/plugins/FPSLocker/", 777);
		mkdir("sdmc:/SaltySD/plugins/FPSLocker/IntDisplays/", 777);
		switch(DISPLAY_A.vendorID[0]) {
			case 0:
				if (!DISPLAY_A.vendorID[2]) {
					model = 0;
				}
				break;
			case 0xB3:
				if (!DISPLAY_A.vendorID[2]) {
					model = 1;
				}
				break;
			case 0x83:
				if (DISPLAY_A.vendorID[2] == 0xF) {
					model = 2;
				}
				break;
			case 0x10:
				switch(DISPLAY_A.vendorID[1]) {
					case 0x81:
						model = 3;
						break;
					case 0x96:
						model = 4;
						break;
					default:
						model = 5;
				}
				break;
			case 0x20:
				switch(DISPLAY_A.vendorID[1]) {
					case 0x93:
						model = 6;
						break;
					case 0x94:
						model = 7;
						break;
					case 0x95:
						switch(DISPLAY_A.vendorID[2]) {
							case 0xF:
								model = 8;
								break;
							case 0x10:
								model = 9;								
						}
						break;
					case 0x96:
						switch(DISPLAY_A.vendorID[2]) {
							case 0xF:
								model = 10;
								break;
							case 0x10:
								model = 11;								
						}
						break;
					case 0x97:
						model = 12;
						break;
					case 0x98:
						model = 13;
						break;
					case 0x99:
						model = 14;
						break;
					default:
						model = 15;
				}
				break;
			case 0x30:
				switch(DISPLAY_A.vendorID[1]) {
					case 0x93:
						switch(DISPLAY_A.vendorID[2]) {
							case 0xF:
								model = 16;
								break;
							case 0x10:
								model = 17;
						}
						break;
					case 0x94:
						switch(DISPLAY_A.vendorID[2]) {
							case 0xF:
								model = 18;
								break;
							case 0x10:
								model = 19;
						}
						break;
					case 0x95:
						switch(DISPLAY_A.vendorID[2]) {
							case 0xF:
								model = 20;
								break;
							case 0x10:
								model = 21;
						}
						break;
					case 0x97:
						model = 22;
						break;
					case 0x98:
						model = 23;
						break;
					case 0x99:
						model = 24;
						break;
					default:
						model = 25;
				}
				break;
			case 0x40:
				if (DISPLAY_A.vendorID[1] == 0x94) {
					model = 26;
				}
				else model = 27;
				break;
			case 0x50:
				if (DISPLAY_A.vendorID[1] == 0x9B) {
					model = 28;
				}
				else model = 29;
				break;
		}
	}

	size_t base_height = 128;
	bool block = false;

    virtual tsl::elm::Element* createUI() override {
        auto frame = new tsl::elm::OverlayFrame("FPSLocker", "Handheld display settings");

		auto list = new tsl::elm::List();

		list->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {

			renderer->drawString(Handheld_c, false, x, y+20, 20, renderer->a(0xFFFF));
			
		}), 105);

		auto *clickableListItem1 = new tsl::elm::ListItem2("Set lowest refresh rate");
		clickableListItem1->setClickListener([this](u64 keys) { 
			if ((keys & HidNpadButton_A) && !isOLED) {
				tsl::changeTo<HandheldMinGui>();
				return true;
			}
			return false;
		});

		list->addItem(clickableListItem1);

		auto *clickableListItem2 = new tsl::elm::ListItem2("Set highest refresh rate");
		clickableListItem2->setClickListener([this](u64 keys) { 
			if ((keys & HidNpadButton_A) && !isOLED) {
				tsl::changeTo<HandheldMaxGui>();
				return true;
			}
			return false;
		});

		list->addItem(clickableListItem2);
		
		frame->setContent(list);

        return frame;
    }

	// Called once every frame to handle inputs not handled by other UI elements
	virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
		if (keysDown & HidNpadButton_B) {
			if (R_SUCCEEDED(SaltySD_Connect())) {
				SaltySD_SetMinMaxHandheldRefreshRate(HandheldModeRefreshRateAllowed.min, HandheldModeRefreshRateAllowed.max);
				SaltySD_Term();
				SaveHandheldModeAllowed();
			}
			tsl::goBack();
			return true;
		}
		snprintf(Handheld_c, sizeof(Handheld_c), "Panel:\n%s\nVendor ID: [%02X] %02X [%02X]\nMin: %d Hz, Max: %d Hz\nOLED panel according to NV: %s", HandheldDisplayModels[model], DISPLAY_A.vendorID[0], DISPLAY_A.vendorID[1], DISPLAY_A.vendorID[2], HandheldModeRefreshRateAllowed.min, HandheldModeRefreshRateAllowed.max, DISPLAY_A.isOLED ? "yes" : "no");
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
						if (refreshRate_g < HandheldModeRefreshRateAllowed.max) {
							if (R_SUCCEEDED(SaltySD_Connect())) {
								if (refreshRate_g == 70) refreshRate_g = 72;
								else refreshRate_g = (refreshRate_g - (refreshRate_g % 5)) + 5;
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
						if (refreshRate_g > HandheldModeRefreshRateAllowed.min) {
							if (R_SUCCEEDED(SaltySD_Connect())) {
								if (refreshRate_g == 75) refreshRate_g = 72;
								else if (refreshRate_g == 72) refreshRate_g = 70;
								else refreshRate_g -= 5;
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

			auto *clickableListItem5 = new tsl::elm::ListItem2("Handheld Settings");
			clickableListItem5->setClickListener([this](u64 keys) { 
				if ((keys & HidNpadButton_A)) {
					tsl::changeTo<HandheldGui>();
					return true;
				}
				return false;
			});

			list->addItem(clickableListItem5);
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