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
        auto frame = new tsl::elm::OverlayFrame("FPSLocker", getStringID(89));

		auto list = new tsl::elm::List();

		list->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
            if (!state) {
                renderer->drawString(getStringID(90), false, x, y+20, 20, renderer->a(0xFFFF));
            }
			else if (!block) {
				renderer->fillScreen(0xF000);
				renderer->drawRect(x+x_1, y+y_1, block_width, block_height, renderer->a(0xFFFF));
				renderer->drawString(getStringID(91), false, x+20, y+height+20, 20, renderer->a(0xFFFF));
			}
			else {
				renderer->drawString(getStringID(92), false, x, y+20, 20, renderer->a(0xFFFF));
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
			if ((svcGetSystemTick() - tick) > ((systemtickfrequency / *refreshRate_shared) * 2)) {
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
	std::string Docked_c = getStringID(93);
	char PressButton[128] = "";
	DockedModeRefreshRateAllowed rr;
	DockedModeRefreshRateAllowed rr_default;
	DockedAdditionalSettings as;
	uint8_t highestRefreshRate = 60;
	s32 height = 1080;
    DockedWizardGui(uint8_t highestRefreshRate_impl) {
		snprintf(PressButton, sizeof(PressButton), getStringID(94), 40);
		if (highestRefreshRate_impl >= 70) highestRefreshRate = highestRefreshRate_impl;
		s32 width = 0;
		ommGetDefaultDisplayResolution(&width, &height);
		LoadDockedModeAllowedSave(rr_default, as, nullptr, height == 720);
		memcpy(&rr, &rr_default, sizeof(rr));
		memset(&rr, 1, 5);
		tick = 0;
        i = 0;
	}

    virtual tsl::elm::Element* createUI() override {
        auto frame = new tsl::elm::OverlayFrame("FPSLocker", getStringID(95));

		auto list = new tsl::elm::List();

		list->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {

			renderer->drawString(Docked_c.c_str(), false, x, y+20, 20, renderer->a(0xFFFF));

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
			s32 width = 0;
			s32 height_now = height;
			ommGetDefaultDisplayResolution(&width, &height_now);
			if (mode != ApmPerformanceMode_Boost || height != height_now) {
				smExit();
				tsl::goBack();
				return true;
			}
		}
		smExit();
		if (keysDown & HidNpadButton_B) {
			if (R_SUCCEEDED(SaltySD_Connect())) {
				SaltySD_SetAllowedDockedRefreshRates(rr_default, height == 720);
				remove("sdmc:/SaltySD/test.flag");
				svcSleepThread(100'000);
				if (tick) {
					SaltySD_SetDisplayRefreshRate(60);
					svcSleepThread(100'000);
				}
				SaltySD_Term();
			}
			tsl::goBack();
			return true;
		}
		s32 width = 0;
		s32 height = 0;
		if (R_SUCCEEDED(ommGetDefaultDisplayResolution(&width, &height))) {
			if (height != 720 && height != 1080) {
				snprintf(PressButton, sizeof(PressButton), getStringID(96), (uint16_t)height);
				return true;
			}
		}
		if (check && PluginRunning && (Shared -> pluginActive)) {
				snprintf(PressButton, sizeof(PressButton), getStringID(97));
				return true;			
		}
		static u64 keyCheck = HidNpadButton_ZL;
		if ((keysHeld & HidNpadButton_X) && !tick) {
			tick = svcGetSystemTick();
			if (R_SUCCEEDED(SaltySD_Connect())) {
				SaltySD_SetAllowedDockedRefreshRates(rr, height == 720);
				FILE* file = fopen("sdmc:/SaltySD/test.flag", "wb");
				if (file) fclose(file);
				svcSleepThread(100'000);
				SaltySD_SetDisplayRefreshRate(40);
				svcSleepThread(100'000);
				SaltySD_Term();
				snprintf(PressButton, sizeof(PressButton), getStringID(98));
			}
			return true;
		}
		if (tick) {
			if (i > 3) {
				if (R_SUCCEEDED(SaltySD_Connect())) {
					SaltySD_SetAllowedDockedRefreshRates(rr, height == 720);
					remove("sdmc:/SaltySD/test.flag");
					svcSleepThread(100'000);
					SaltySD_Term();
				}
				SaveDockedModeAllowedSave(rr, as, height == 720);

				tsl::goBack();
				tsl::changeTo<DockedManualGui>(highestRefreshRate);
				return true;
			}
			if (svcGetSystemTick() - tick < (15 * systemtickfrequency)) {
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
						snprintf(PressButton, sizeof(PressButton), getStringID(99), DockedModeRefreshRateAllowedValues[i]);
					}
					if (i % 3 == 0) {
						keyCheck = HidNpadButton_Y;
						snprintf(PressButton, sizeof(PressButton), getStringID(100), DockedModeRefreshRateAllowedValues[i]);
					}
					if (i % 2 == 0) {
						keyCheck = HidNpadButton_ZR;
						snprintf(PressButton, sizeof(PressButton), getStringID(101), DockedModeRefreshRateAllowedValues[i]);
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
					snprintf(PressButton, sizeof(PressButton), getStringID(99), DockedModeRefreshRateAllowedValues[i]);
				}
				if (i % 3 == 0) {
					keyCheck = HidNpadButton_Y;
					snprintf(PressButton, sizeof(PressButton), getStringID(100), DockedModeRefreshRateAllowedValues[i]);
				}
				if (i % 2 == 0) {
					keyCheck = HidNpadButton_ZR;
					snprintf(PressButton, sizeof(PressButton), getStringID(101), DockedModeRefreshRateAllowedValues[i]);
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
	char Docked_c[768];

	char PressButton[128];
	DockedModeRefreshRateAllowed rr;
	DockedModeRefreshRateAllowed rr_default;
	DockedAdditionalSettings as;
	uint8_t m_maxRefreshRate;
	uint16_t delay_s = 10;
	bool block = false;
	s32 height = 1080;
    DockedOverWizardGui(uint8_t maxRefreshRate) {
		snprintf(PressButton, sizeof(PressButton), getStringID(94), 70);
		if (maxRefreshRate > DockedModeRefreshRateAllowedValues[sizeof(DockedModeRefreshRateAllowedValues) - 1])
			maxRefreshRate = DockedModeRefreshRateAllowedValues[sizeof(DockedModeRefreshRateAllowedValues) - 1];
		m_maxRefreshRate = maxRefreshRate;
		s32 width = 0;
		ommGetDefaultDisplayResolution(&width, &height);
		LoadDockedModeAllowedSave(rr_default, as, nullptr, height == 720);
		memcpy(&rr, &rr_default, sizeof(rr));
		tick = 0;
		i = 5;
		uint8_t times = 0;
		for (size_t x = 5; x < sizeof(DockedModeRefreshRateAllowedValues); x++) {
			if (DockedModeRefreshRateAllowedValues[x] > maxRefreshRate)
				rr[x] = false;
			else {
				rr[x] = true;
				times++;
			}
		}
		snprintf(Docked_c, sizeof(Docked_c), getStringID(102), maxRefreshRate, delay_s * times);
	}

    virtual tsl::elm::Element* createUI() override {
        auto frame = new tsl::elm::OverlayFrame("FPSLocker", getStringID(103));

		auto list = new tsl::elm::List();

		list->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {

			renderer->drawString(Docked_c, false, x, y+20, 20, renderer->a(0xFFFF));

			renderer->drawString(PressButton, false, x, y+260, 20, renderer->a(0xFFFF));
			
		}), 270);		
		
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
			s32 width = 0;
			s32 height_now = height;
			ommGetDefaultDisplayResolution(&width, &height_now);
			if (height != height_now || mode != ApmPerformanceMode_Boost) {
				smExit();
				tsl::goBack();
				return true;
			}
		}
		smExit();
		if (keysDown & HidNpadButton_B) {
			if (R_SUCCEEDED(SaltySD_Connect())) {
				SaltySD_SetAllowedDockedRefreshRates(rr_default, height == 720);
				remove("sdmc:/SaltySD/test.flag");
				svcSleepThread(100'000);
				if (tick) {
					SaltySD_SetDisplayRefreshRate(60);
					svcSleepThread(100'000);
				}
				SaltySD_Term();
			}
			tsl::goBack();
			return true;
		}
		s32 width = 0;
		s32 height = 0;
		if (R_SUCCEEDED(ommGetDefaultDisplayResolution(&width, &height))) {
			if (height != 1080 && height != 720) {
				snprintf(PressButton, sizeof(PressButton), getStringID(96), (uint16_t)height);
				return true;
			}
		}
		if (check && PluginRunning && (Shared -> pluginActive)) {
			snprintf(PressButton, sizeof(PressButton), getStringID(97));
			return true;
		}
		static u64 keyCheck = HidNpadButton_ZL;
		if ((keysHeld & HidNpadButton_X) && !tick) {
			tick = svcGetSystemTick();
			if (R_SUCCEEDED(SaltySD_Connect())) {
				SaltySD_SetAllowedDockedRefreshRates(rr, height == 720);
				FILE* file = fopen("sdmc:/SaltySD/test.flag", "wb");
				if (file) fclose(file);
				svcSleepThread(100'000);
				SaltySD_SetDisplayRefreshRate(DockedModeRefreshRateAllowedValues[i]);
				svcSleepThread(100'000);
				SaltySD_Term();
				snprintf(PressButton, sizeof(PressButton), getStringID(98), DockedModeRefreshRateAllowedValues[i]);
			}
			return true;
		}
		if (tick) {
			if (DockedModeRefreshRateAllowedValues[i] > m_maxRefreshRate) {
				if (R_SUCCEEDED(SaltySD_Connect())) {
					SaltySD_SetAllowedDockedRefreshRates(rr, height == 720);
					remove("sdmc:/SaltySD/test.flag");
					svcSleepThread(100'000);
					SaltySD_SetDisplayRefreshRate(60);
					svcSleepThread(100'000);
					SaltySD_Term();
				}
				SaveDockedModeAllowedSave(rr, as, height == 720);
				tsl::goBack();
				tsl::changeTo<DockedManualGui>(m_maxRefreshRate);
				return true;
			}
			if (svcGetSystemTick() - tick < (delay_s * systemtickfrequency)) {
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
						snprintf(PressButton, sizeof(PressButton), getStringID(99), DockedModeRefreshRateAllowedValues[i]);
					}
					if (i % 3 == 0) {
						keyCheck = HidNpadButton_Y;
						snprintf(PressButton, sizeof(PressButton), getStringID(100), DockedModeRefreshRateAllowedValues[i]);
					}
					if (i % 2 == 0) {
						keyCheck = HidNpadButton_ZR;
						snprintf(PressButton, sizeof(PressButton), getStringID(101), DockedModeRefreshRateAllowedValues[i]);
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
					snprintf(PressButton, sizeof(PressButton), getStringID(99), DockedModeRefreshRateAllowedValues[i]);
				}
				if (i % 3 == 0) {
					keyCheck = HidNpadButton_Y;
					snprintf(PressButton, sizeof(PressButton), getStringID(100), DockedModeRefreshRateAllowedValues[i]);
				}
				if (i % 2 == 0) {
					keyCheck = HidNpadButton_ZR;
					snprintf(PressButton, sizeof(PressButton), getStringID(101), DockedModeRefreshRateAllowedValues[i]);
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
	uint8_t maxRefreshRate = 60;
	s32 height = 1080;
    DockedManualGui(uint8_t maxRefreshRate_impl) {
		if (maxRefreshRate_impl >= 70) maxRefreshRate = maxRefreshRate_impl;
		s32 width = 0;
		ommGetDefaultDisplayResolution(&width, &height);
		LoadDockedModeAllowedSave(rr, as, nullptr, height == 720);
	}

    virtual tsl::elm::Element* createUI() override {
		char string_temp[128];
		snprintf(string_temp, sizeof(string_temp), getStringID(104), height);
        auto frame = new tsl::elm::OverlayFrame("FPSLocker", string_temp);

		auto list = new tsl::elm::List();

		for (size_t i = 0; i < 4; i++) {
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
		}
		for (size_t i = 5; i < sizeof(DockedModeRefreshRateAllowedValues); i++) {
			if (maxRefreshRate < DockedModeRefreshRateAllowedValues[i]) break;
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
			s32 width = 0;
			s32 height_now = height;
			ommGetDefaultDisplayResolution(&width, &height_now);
			if (mode != ApmPerformanceMode_Boost || height != height_now) {
				tsl::goBack();
				return true;
			}
		}
		smExit();
		if (keysDown & HidNpadButton_B) {
			if (R_SUCCEEDED(SaltySD_Connect())) {
				SaveDockedModeAllowedSave(rr, as, height == 720);
				SaltySD_SetAllowedDockedRefreshRates(rr, height == 720);
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
	s32 height = 1080;
    DockedAdditionalGui() {
		s32 width = 0;
		ommGetDefaultDisplayResolution(&width, &height);
		LoadDockedModeAllowedSave(rr, as, nullptr, height == 720);
	}

    virtual tsl::elm::Element* createUI() override {
        auto frame = new tsl::elm::OverlayFrame("FPSLocker", getStringID(105));

		auto list = new tsl::elm::List();

		auto *clickableListItem4 = new tsl::elm::ToggleListItem(getStringID(106), !as.dontForce60InDocked);
		clickableListItem4->setClickListener([this](u64 keys) { 
			if (keys & HidNpadButton_A) {
				as.dontForce60InDocked = !as.dontForce60InDocked;
				if (R_SUCCEEDED(SaltySD_Connect())) {
					SaltySD_SetDontForce60InDocked(as.dontForce60InDocked);
					SaltySD_Term();
				}
				SaveDockedModeAllowedSave(rr, as, height == 720);
				return true;
			}
			return false;
		});

		list->addItem(clickableListItem4);

		auto *clickableListItem5 = new tsl::elm::ToggleListItem(getStringID(107), as.fpsTargetWithoutRRMatchLowest);
		clickableListItem5->setClickListener([this](u64 keys) { 
			if (keys & HidNpadButton_A) {
				as.fpsTargetWithoutRRMatchLowest = !as.fpsTargetWithoutRRMatchLowest;
				if (R_SUCCEEDED(SaltySD_Connect())) {
					SaltySD_SetMatchLowestRR(as.fpsTargetWithoutRRMatchLowest);
					SaltySD_Term();
				}
				SaveDockedModeAllowedSave(rr, as, height == 720);
				return true;
			}
			return false;
		});

		list->addItem(clickableListItem5);

		auto *clickableListItem6 = new tsl::elm::ToggleListItem(getStringID(130), as.displaySyncDockedOutOfFocus60);
		clickableListItem6->setClickListener([this](u64 keys) { 
			if (keys & HidNpadButton_A) {
				as.displaySyncDockedOutOfFocus60 = !as.displaySyncDockedOutOfFocus60;
				if (R_SUCCEEDED(SaltySD_Connect())) {
					SaltySD_SetDisplaySyncRefreshRate60WhenOutOfFocus(true, as.displaySyncDockedOutOfFocus60);
					SaltySD_Term();
				}
				SaveDockedModeAllowedSave(rr, as, height == 720);
				return true;
			}
			return false;
		});

		list->addItem(clickableListItem6);
		
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
	uint8_t highestRefreshRate;
	uint8_t linkRate;
	ApmPerformanceMode mode = ApmPerformanceMode_Invalid;
	bool block = false;
	s32 height = 0;
public:
    DockedGui() {
		s32 width = 0;
		ommGetDefaultDisplayResolution(&width, &height);
		mkdir("sdmc:/SaltySD/plugins/FPSLocker/", 777);
		mkdir("sdmc:/SaltySD/plugins/FPSLocker/ExtDisplays/", 777);
		int crc32 = 0;
		LoadDockedModeAllowedSave(rr, as, &crc32, height == 720);
		highestRefreshRate = 60;
		linkRate = 10;
		getDockedHighestRefreshRate(&highestRefreshRate, &linkRate);
		char linkMode[5] = "HBR";
		if (linkRate == 30) strcpy(linkMode, "HBR3");
		else if (linkRate == 20) strcpy(linkMode, "HBR2");
		else if (linkRate == 6) strcpy(linkMode, "RBR");
		else if (linkRate != 10) strcpy(linkMode, getStringID(108));
		snprintf(Docked_c, sizeof(Docked_c), getStringID(109), highestRefreshRate, linkMode, crc32);
		tsl::hlp::doWithSmSession([this]{
			if (R_SUCCEEDED(apmInitialize())) {
				mode = ApmPerformanceMode_Invalid;
				apmGetPerformanceMode(&mode);
				if (mode != ApmPerformanceMode_Boost) {
					block = true;
					strcpy(Docked_c, getStringID(114));
				}
				apmExit();
			}
		});
	}

	size_t base_height = 128;

    virtual tsl::elm::Element* createUI() override {
        auto frame = new tsl::elm::OverlayFrame("FPSLocker", getStringID(110));

		auto list = new tsl::elm::List();

		list->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {

			renderer->drawString(Docked_c, false, x, y+20, 20, renderer->a(0xFFFF));
			if (!block) {
				if (linkRate < 20) renderer->drawString("\uE14C", false, x+220, y+40, 20, renderer->a(0xF00F));
				else renderer->drawString("\uE14B", false, x+220, y+40, 20, renderer->a(0xF0F0));
			}

			
		}), 85);

		if (mode == ApmPerformanceMode_Boost && (height == 720 || height == 1080)) {
			char string_temp[128];
			snprintf(string_temp, sizeof(string_temp), getStringID(111), height);
			auto *clickableListItem1 = new tsl::elm::ListItem2(string_temp);
			clickableListItem1->setClickListener([this](u64 keys) { 
				if ((keys & HidNpadButton_A) && !block) {
					tsl::changeTo<DockedManualGui>(highestRefreshRate);
					return true;
				}
				return false;
			});

			list->addItem(clickableListItem1);

			auto *clickableListItem2 = new tsl::elm::ListItem2(getStringID(95));
			clickableListItem2->setClickListener([this](u64 keys) { 
				if ((keys & HidNpadButton_A) && !block) {
					tsl::changeTo<DockedWizardGui>(highestRefreshRate);
					return true;
				}
				return false;
			});

			list->addItem(clickableListItem2);

			if (highestRefreshRate >= 70) {
				char string_temp[128];
				snprintf(string_temp, sizeof(string_temp), getStringID(112), height);
				auto *clickableListItem22 = new tsl::elm::ListItem2(string_temp);
				clickableListItem22->setClickListener([this](u64 keys) {
					if ((keys & HidNpadButton_A) && !block) {
						tsl::changeTo<DockedOverWizardGui>(highestRefreshRate);
						return true;
					}
					return false;
				});

				list->addItem(clickableListItem22);
			}

			auto *clickableListItem4 = new tsl::elm::ListItem2(getStringID(113));
			clickableListItem4->setClickListener([this](u64 keys) { 
				if ((keys & HidNpadButton_A) && !block) {
					tsl::changeTo<DockedAdditionalGui>();
					return true;
				}
				return false;
			});

			list->addItem(clickableListItem4);
		}

		auto *clickableListItem3 = new tsl::elm::ListItem2(getStringID(89));
		clickableListItem3->setClickListener([this](u64 keys) { 
			if ((keys & HidNpadButton_A)) {
				tsl::changeTo<DockedFrameskipGui>();
				return true;
			}
			return false;
		});

		list->addItem(clickableListItem3);
		
		frame->setContent(list);

        return frame;
    }

	virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
		if (!block) {
			s32 width = 0;
			s32 height_impl = 0;
			if (R_SUCCEEDED(ommGetDefaultDisplayResolution(&width, &height_impl))) {
				if (height != height_impl) {
					tsl::goBack();
					tsl::changeTo<DockedGui>();
					return true;
				}
			}
			smInitialize();
			if (R_SUCCEEDED(apmInitialize())) {
				ApmPerformanceMode mode = ApmPerformanceMode_Invalid;
				apmGetPerformanceMode(&mode);
				apmExit();
				if (mode != ApmPerformanceMode_Boost) {
					tsl::goBack();
					tsl::changeTo<DockedGui>();
					return true;
				}
			}
			smExit();
		}

		return false;
	}
};

class DockedRefreshRateChangeGui : public tsl::Gui {
public:
	DockedModeRefreshRateAllowed rr;
	DockedAdditionalSettings as;
	DockedRefreshRateChangeGui () {
		s32 width = 0;
		s32 height = 1080;
		ommGetDefaultDisplayResolution(&width, &height);
		LoadDockedModeAllowedSave(rr, as, nullptr, height == 720);
	}

	// Called when this Gui gets loaded to create the UI
	// Allocate all elements on the heap. libtesla will make sure to clean them up when not needed anymore
	virtual tsl::elm::Element* createUI() override {
		// A OverlayFrame is the base element every overlay consists of. This will draw the default Title and Subtitle.
		// If you need more information in the header or want to change it's look, use a HeaderOverlayFrame.
		auto frame = new tsl::elm::OverlayFrame("FPSLocker", getStringID(115));

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
							if (Shared) Shared->displaySync.ds.docked = refreshRate_g != 0;
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
	char refreshRate_c[40] = "";
	bool isDocked = false;
	ApmPerformanceMode entry_mode = ApmPerformanceMode_Invalid;
	bool isPossiblyRetroRemake = false;
	bool RetroRemakeMode = false;
public:
    DisplayGui() {
		if (isLite) {
			entry_mode = ApmPerformanceMode_Normal;
			if (R_SUCCEEDED(SaltySD_Connect())) {
				SaltySD_isPossiblyRetroRemake(&isPossiblyRetroRemake);
				SaltySD_Term();
				if (isPossiblyRetroRemake) {
					if (file_exists("sdmc:/SaltySD/flags/retro.flag") == true)
						RetroRemakeMode = true;
				}
			}
		}
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
        auto frame = new tsl::elm::OverlayFrame("FPSLocker", getStringID(Lang::Id_DisplaySettings));

		auto list = new tsl::elm::List();

		list->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {

			renderer->drawString(refreshRate_c, false, x, y+20, 20, renderer->a(0xFFFF));

		}), 90);

		if (entry_mode == ApmPerformanceMode_Normal && !displaySync.ds.handheld) {
			auto *clickableListItem = new tsl::elm::ListItem2(getStringID(116)); //Increase refresh rate
			clickableListItem->setClickListener([this](u64 keys) { 
				if (keys & HidNpadButton_A) {
					if ((refreshRate_g >= (isOLED ? supportedHandheldRefreshRatesOLED[0] : supportedHandheldRefreshRates[0])) && (refreshRate_g < (isOLED ? supportedHandheldRefreshRatesOLED[sizeof(supportedHandheldRefreshRatesOLED)-1] : supportedHandheldRefreshRates[sizeof(supportedHandheldRefreshRates)-1]))) {
						if (R_SUCCEEDED(SaltySD_Connect())) {
							refreshRate_g += 5;
							SaltySD_SetDisplayRefreshRate(refreshRate_g);
							SaltySD_Term();
							if (Shared) Shared->displaySync.ds.handheld = refreshRate_g != 0;
						}
					}
					return true;
				}
				return false;
			});

			list->addItem(clickableListItem);

			auto *clickableListItem2 = new tsl::elm::ListItem2(getStringID(117)); //Decrease refresh rate
			clickableListItem2->setClickListener([this](u64 keys) { 
				if (keys & HidNpadButton_A) {
					if (refreshRate_g > (isOLED ? supportedHandheldRefreshRatesOLED[0] : supportedHandheldRefreshRates[0])) {
						if (R_SUCCEEDED(SaltySD_Connect())) {
							refreshRate_g -= 5;
							SaltySD_SetDisplayRefreshRate(refreshRate_g);
							if (Shared) Shared->displaySync.ds.handheld = refreshRate_g != 0;
							SaltySD_Term();
						}
					}
					return true;
				}
				return false;
			});

			list->addItem(clickableListItem2);
		}
		else if (entry_mode == ApmPerformanceMode_Boost && displaySync.ds.docked) {
			auto *clickableListItem2 = new tsl::elm::ListItem2(getStringID(115)); //Change refresh rate
			clickableListItem2->setClickListener([](u64 keys) { 
				if (keys & HidNpadButton_A) {
					tsl::changeTo<DockedRefreshRateChangeGui>();
					return true;
				}
				return false;
			});	
			list->addItem(clickableListItem2);	
		}

		if (!oldSalty) {
			list->addItem(new tsl::elm::CategoryHeader(getStringID(118), true));
			auto *clickableListItem3 = new tsl::elm::ToggleListItem(getStringID(119), displaySync.ds.handheld); //Handheld Display Sync toggle
			clickableListItem3->setClickListener([this](u64 keys) { 
				if (keys & HidNpadButton_A) {
					if (R_SUCCEEDED(SaltySD_Connect())) {
						SaltySD_SetDisplaySync(!displaySync.ds.handheld);
						svcSleepThread(100'000);
						u64 PID = 0;
						Result rc = pmdmntGetApplicationProcessId(&PID);
						if (R_SUCCEEDED(rc) && Shared) {
							if (!displaySync.ds.handheld == true && (Shared -> FPSlocked) < 40) {
								if (entry_mode == ApmPerformanceMode_Normal) SaltySD_SetDisplayRefreshRate(60);
								Shared->displaySync.ds.handheld = false;
							}
							else if (!displaySync.ds.handheld) {
								if (entry_mode == ApmPerformanceMode_Normal) SaltySD_SetDisplayRefreshRate(Shared -> FPSlocked);
								Shared->displaySync.ds.handheld = (Shared -> FPSlocked) > 0;
							}
							else {
								Shared->displaySync.ds.handheld = false;
							}
						}
						else if (!displaySync.ds.handheld == true && (R_FAILED(rc) || !PluginRunning)) {
							if (entry_mode == ApmPerformanceMode_Normal) SaltySD_SetDisplayRefreshRate(60);
						}
						SaltySD_Term();
						displaySync.ds.handheld = !displaySync.ds.handheld;
					}
					if (entry_mode == ApmPerformanceMode_Normal) {
						tsl::goBack();
						tsl::changeTo<DisplayGui>();
					}
					return true;
				}
				return false;
			});

			list->addItem(clickableListItem3);

			if (!isLite) {

				auto *clickableListItem6 = new tsl::elm::ToggleListItem(getStringID(128), displaySync.ds.docked); //Docked Display Sync toggle
				clickableListItem6->setClickListener([this](u64 keys) { 
					if (keys & HidNpadButton_A) {
						if (R_SUCCEEDED(SaltySD_Connect())) {
							SaltySD_SetDisplaySyncDocked(!displaySync.ds.docked);
							svcSleepThread(100'000);
							u64 PID = 0;
							Result rc = pmdmntGetApplicationProcessId(&PID);
							if (R_SUCCEEDED(rc) && Shared) {
								if (!displaySync.ds.docked == true && (Shared -> FPSlockedDocked) < 40) {
									if (entry_mode == ApmPerformanceMode_Boost) SaltySD_SetDisplayRefreshRate(60);
									displaySync.ds.docked = false;
								}
								else if (!displaySync.ds.docked) {
									if (entry_mode == ApmPerformanceMode_Boost) SaltySD_SetDisplayRefreshRate((Shared -> FPSlockedDocked));
									displaySync.ds.docked = (Shared -> FPSlockedDocked) > 0;
								}
								else {
									displaySync.ds.docked = false;
								}
							}
							else if (!displaySync.ds.docked == true && (R_FAILED(rc) || !PluginRunning)) {
								if (entry_mode == ApmPerformanceMode_Boost) SaltySD_SetDisplayRefreshRate(60);
							}
							SaltySD_Term();
							displaySync.ds.docked = !displaySync.ds.docked;
						}
					if (entry_mode == ApmPerformanceMode_Boost) {
						tsl::goBack();
						tsl::changeTo<DisplayGui>();
					}
						return true;
					}
					return false;
				});

				list->addItem(clickableListItem6);
			
				auto *clickableListItem4 = new tsl::elm::ListItem2(getStringID(120)); //Docked settings
				clickableListItem4->setClickListener([this](u64 keys) { 
					if ((keys & HidNpadButton_A)) {
						tsl::changeTo<DockedGui>();
						return true;
					}
					return false;
				});

				list->addItem(clickableListItem4);
			}

			if (isPossiblyRetroRemake) {
				auto *clickableListItem5 = new tsl::elm::ToggleListItem(getStringID(121), RetroRemakeMode); //Retro Remake Mode toogle
				clickableListItem5->setClickListener([this](u64 keys) { 
					if (keys & HidNpadButton_A) {
						if (!RetroRemakeMode) {
							FILE* file = fopen("sdmc:/SaltySD/flags/retro.flag", "wb");
							if (!file) {
								mkdir("sdmc:/SaltySD/flags/", 777);
								file = fopen("sdmc:/SaltySD/flags/retro.flag", "wb");
							}
							if (file) fclose(file);
						}
						else {
							remove("sdmc:/SaltySD/flags/retro.flag");
						}
						RetroRemakeMode = !RetroRemakeMode;
						return true;
					}
					return false;
				});

				list->addItem(clickableListItem5);
			}

			if (displaySync.ds.handheld) {
				list->addItem(new tsl::elm::CategoryHeader(getStringID(129), true));
				auto *clickableListItem6 = new tsl::elm::ToggleListItem(getStringID(130), displaySyncOutOfFocus60); //HH 60Hz in Home Menu
				clickableListItem6->setClickListener([this](u64 keys) { 
					if (keys & HidNpadButton_A) {
						if (R_SUCCEEDED(SaltySD_Connect())) {
							SaltySD_SetDisplaySyncRefreshRate60WhenOutOfFocus(false, !displaySyncOutOfFocus60);
							displaySyncOutOfFocus60 = !displaySyncOutOfFocus60;
							svcSleepThread(100'000);
							SaltySD_Term();
						}
						return true;
					}
					return false;
				});
				list->addItem(clickableListItem6);
			}
		}
		
		frame->setContent(list);

        return frame;
    }

	virtual void update() override {
		refreshRate_g = *refreshRate_shared;
		snprintf(refreshRate_c, sizeof(refreshRate_c), getStringID(122), refreshRate_g);
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
	std::string Warning = getStringID(123);
public:
    WarningDisplayGui() {}

	size_t base_height = 128;

    virtual tsl::elm::Element* createUI() override {
        auto frame = new tsl::elm::OverlayFrame("FPSLocker", getStringID(124));

		auto list = new tsl::elm::List();

		auto how_many_lines = 1 + std::ranges::count(Warning, '\n');

		list->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
			renderer->drawString(Warning.c_str(), false, x, y+20, 20, renderer->a(0xFFFF));
		}), 10 + (how_many_lines * 20));

		auto *clickableListItem1 = new tsl::elm::ListItem2(getStringID(125));
		clickableListItem1->setClickListener([this](u64 keys) { 
			if (keys & HidNpadButton_A) {
				tsl::goBack();
				return true;
			}
			return false;
		});

		list->addItem(clickableListItem1);

		auto *clickableListItem2 = new tsl::elm::ListItem2(getStringID(126));
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