class SetBuffers : public tsl::Gui {
public:
    SetBuffers() {}

    virtual tsl::elm::Element* createUI() override {
		auto frame = new tsl::elm::OverlayFrame(getStringID(30), " ");

		auto list = new tsl::elm::List();
		if (Shared->expectedSetBuffers == -1) list->addItem(new tsl::elm::NoteHeader(getStringID(31), true, {0xF, 0x3, 0x3, 0xF}));
		auto *clickableListItem = new tsl::elm::ListItem2(getStringID(33));
		clickableListItem->setClickListener([](u64 keys) { 
			if ((keys & HidNpadButton_A) && PluginRunning) {
				SetBuffers_save = 2;
				if (Shared->expectedSetBuffers != -1) Shared->expectedSetBuffers = 2;
				saveSettings();
				tsl::goBack();
				tsl::goBack();
				return true;
			}
			return false;
		});
		list->addItem(clickableListItem);

		if ((Shared -> API) == 3) {
			auto *clickableListItemv1 = new tsl::elm::ListItem2(getStringID(34));
			clickableListItemv1->setClickListener([](u64 keys) { 
				if ((keys & HidNpadButton_A) && PluginRunning) {
					SetBuffers_save = 3;
					if (Shared->expectedSetBuffers != -1) Shared->expectedSetBuffers = 3;
					saveSettings();
					tsl::goBack();
					tsl::goBack();
					return true;
				}
				return false;
			});
			list->addItem(clickableListItemv1);

		}
		else {
			if ((Shared -> SetActiveBuffers) > 0 && (Shared -> Buffers) >= 3) {
				auto *clickableListItem2 = new tsl::elm::ListItem2(getStringID(35));
				clickableListItem2->setClickListener([](u64 keys) { 
					if ((keys & HidNpadButton_A) && PluginRunning) {
						SetBuffers_save = 3;
						if (Shared->expectedSetBuffers != -1) Shared->expectedSetBuffers = 3;
						saveSettings();
						tsl::goBack();
						tsl::goBack();
						return true;
					}
					return false;
				});
				list->addItem(clickableListItem2);
			}
			else {
				auto *clickableListItem2 = new tsl::elm::ListItem2(getStringID(34));
				clickableListItem2->setClickListener([](u64 keys) { 
					if ((keys & HidNpadButton_A) && PluginRunning) {
						if ((Shared -> Buffers) == 4) SetBuffers_save = 3;
						else SetBuffers_save = 0;
						if (Shared->expectedSetBuffers != -1) Shared->expectedSetBuffers = 3;
						saveSettings();
						tsl::goBack();
						tsl::goBack();
						return true;
					}
					return false;
				});
				list->addItem(clickableListItem2);
			}
			
			if ((Shared -> Buffers) == 4) {
				if ((Shared -> SetActiveBuffers) > 0) {
					auto *clickableListItem3 = new tsl::elm::ListItem2(getStringID(36));
					clickableListItem3->setClickListener([](u64 keys) { 
						if ((keys & HidNpadButton_A) && PluginRunning) {
							SetBuffers_save = 4;
							if (Shared->expectedSetBuffers != -1) Shared->expectedSetBuffers = 4;
							saveSettings();
							tsl::goBack();
							tsl::goBack();
							return true;
						}
						return false;
					});
					list->addItem(clickableListItem3);	
				}
				else {
					auto *clickableListItem3 = new tsl::elm::ListItem2(getStringID(37));
					clickableListItem3->setClickListener([](u64 keys) { 
						if ((keys & HidNpadButton_A) && PluginRunning) {
							SetBuffers_save = 0;
							if (Shared->expectedSetBuffers != -1) Shared->expectedSetBuffers = 4;
							saveSettings();
							tsl::goBack();
							tsl::goBack();
							return true;
						}
						return false;
					});
					list->addItem(clickableListItem3);
				}
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
        auto frame = new tsl::elm::OverlayFrame(getStringID(38), getStringID(39));

		auto list = new tsl::elm::List();

		auto *clickableListItem = new tsl::elm::ListItem2(getStringID(40));
		clickableListItem->setClickListener([](u64 keys) { 
			if ((keys & HidNpadButton_A) && PluginRunning) {
				ZeroSyncMode = getStringID(43);
				(Shared -> ZeroSync) = 0;
				saveSettings();
				tsl::goBack();
				tsl::goBack();
				return true;
			}
			return false;
		});
		list->addItem(clickableListItem);

		auto *clickableListItem2 = new tsl::elm::ListItem2(getStringID(41));
		clickableListItem2->setClickListener([](u64 keys) { 
			if ((keys & HidNpadButton_A) && PluginRunning) {
				ZeroSyncMode = getStringID(45);
				(Shared -> ZeroSync) = 2;
				saveSettings();
				tsl::goBack();
				tsl::goBack();
				return true;
			}
			return false;
		});
		list->addItem(clickableListItem2);

		auto *clickableListItem3 = new tsl::elm::ListItem2(getStringID(42));
		clickableListItem3->setClickListener([](u64 keys) { 
			if ((keys & HidNpadButton_A) && PluginRunning) {
				ZeroSyncMode = getStringID(44);
				(Shared -> ZeroSync) = 1;
				saveSettings();
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
				sprintf(&lockInvalid[0], getStringID(46), TID, BID);
			}
			else sprintf(&lockInvalid[0], getStringID(47), configValid);
		}
		else {
			patchValid = !file_exists(&patchPath[0]);
			if (R_FAILED(patchValid)) {
				if (!FileDownloaded) {
					if (R_SUCCEEDED(configValid)) {
						sprintf(&patchChar[0], getStringID(48));
					}
					else sprintf(&patchChar[0], getStringID(49));
				}
				else {
					sprintf(&patchChar[0], getStringID(50));
				}
			}
			else sprintf(&patchChar[0], getStringID(51));
		}
		switch((Shared -> ZeroSync)) {
			case 0:
				ZeroSyncMode = getStringID(43);
				break;
			case 1:
				ZeroSyncMode = getStringID(44);
				break;
			case 2:
				ZeroSyncMode = getStringID(45);
		}
	}

	size_t base_height = 134;

    virtual tsl::elm::Element* createUI() override {
        auto frame = new tsl::elm::OverlayFrame("FPSLocker", getStringID(52));

		auto list = new tsl::elm::List();

		if ((Shared -> API)) {
			switch((Shared -> API)) {
				case 1: {
					list->addItem(new tsl::elm::CategoryHeader(getStringID(53), false));
					
					list->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
						
						renderer->drawString(&nvnBuffers[0], false, x, y+20, 20, renderer->a(0xFFFF));
							
					}), 60);

					if ((Shared -> Buffers) == 2 || (Shared -> ActiveBuffers) == 2) {
						auto *clickableListItem3 = new tsl::elm::MiniListItem(getStringID(54), ZeroSyncMode);
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
						auto *clickableListItem3 = new tsl::elm::MiniListItem(getStringID(30));
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
					list->addItem(new tsl::elm::CategoryHeader(getStringID(55), false));
					break;
				case 3: {
					list->addItem(new tsl::elm::CategoryHeader(getStringID(56), false));

					list->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
						
						renderer->drawString(&nvnBuffers[0], false, x, y+20, 20, renderer->a(0xFFFF));
							
					}), 40);

					if ((Shared -> Buffers) >= 2) {
						auto *clickableListItem3 = new tsl::elm::MiniListItem(getStringID(30));
						clickableListItem3->setClickListener([](u64 keys) { 
							if ((keys & HidNpadButton_A) && PluginRunning) {
								tsl::changeTo<SetBuffers>();
								return true;
							}
							return false;
						});
						list->addItem(clickableListItem3);
					}
				}
			}
		}

		list->addItem(new tsl::elm::CategoryHeader(getStringID(57), false));

		if (R_FAILED(configValid)) {
			base_height = 154;
		}

		list->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
			
			if (R_SUCCEEDED(configValid)) {
				
				renderer->drawString(getStringID(58), false, x, y+20, 20, renderer->a(0xFFFF));
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
			list->addItem(new tsl::elm::NoteHeader(getStringID(59), true, {0xF, 0x3, 0x3, 0xF}));
			auto *clickableListItem = new tsl::elm::MiniListItem(getStringID(60));
			clickableListItem->setClickListener([](u64 keys) { 
				if ((keys & HidNpadButton_A) && PluginRunning) {
					patchValid = LOCK::createPatch(&patchPath[0]);
					if (R_SUCCEEDED(patchValid)) {
						sprintf(&patchChar[0], getStringID(61));
					}
					else sprintf(&patchChar[0], getStringID(62), patchValid);
					return true;
				}
				return false;
			});
			list->addItem(clickableListItem);

			auto *clickableListItem2 = new tsl::elm::MiniListItem(getStringID(63));
			clickableListItem2->setClickListener([](u64 keys) { 
				if ((keys & HidNpadButton_A) && PluginRunning) {
					if (R_SUCCEEDED(patchValid)) {
						remove(&patchPath[0]);
						patchValid = 0x202;
						sprintf(&patchChar[0], getStringID(64));
					}
					return true;
				}
				return false;
			});
			list->addItem(clickableListItem2);
		}
		if (R_FAILED(configValid)) {
			list->addItem(new tsl::elm::NoteHeader(getStringID(65), true, {0xF, 0x3, 0x3, 0xF}));
		}
		auto *clickableListItem4 = new tsl::elm::MiniListItem(getStringID(66));
		clickableListItem4->setClickListener([this](u64 keys) { 
			if ((keys & HidNpadButton_A) && PluginRunning && exitPossible) {
				exitPossible = false;
				sprintf(&patchChar[0], getStringID(67));
				threadCreate(&t1, downloadPatch, NULL, NULL, 0x20000, 0x3F, 3);
				threadStart(&t1);
				return true;
			}
			return false;
		});
		list->addItem(clickableListItem4);

		list->addItem(new tsl::elm::CategoryHeader(getStringID(68), false));

		auto *clickableListItem5 = new tsl::elm::MiniToggleListItem(getStringID(69), forceSuspend_save);
		clickableListItem5->setClickListener([this](u64 keys) { 
			if ((keys & HidNpadButton_A) && PluginRunning) {
				forceSuspend_save = !forceSuspend_save;
				(Shared -> forceSuspend) = forceSuspend_save;
				return true;
			}
			return false;
		});
		list->addItem(clickableListItem5);		

		frame->setContent(list);

        return frame;
    }

	virtual void update() override {
		static uint8_t i = 10;

		if (PluginRunning) {
			if (i > 9) {
				if ((Shared -> patchApplied) == 1) {
					sprintf(patchAppliedChar, getStringID(70));
				}
				else if ((Shared -> patchApplied) == 2) {
					sprintf(patchAppliedChar, getStringID(71));
				}
				else sprintf(patchAppliedChar, getStringID(72));
				switch (Shared -> API) {
					case 1: {
						if (((Shared -> Buffers) >= 2 && (Shared -> Buffers) <= 4)) {
							sprintf(&nvnBuffers[0], getStringID(73), (Shared -> SetActiveBuffers), (Shared -> ActiveBuffers), (Shared -> Buffers));
						}
						break;
					}
					case 3: {
						if (((Shared -> Buffers) >= 2 && (Shared -> Buffers) <= 4)) {
							sprintf(&nvnBuffers[0], getStringID(74), (Shared -> Buffers));
						}
						break;
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
				sprintf(&patchChar[0], getStringID(75));
			}
			else if (rc == 0x212 || rc == 0x312) {
				sprintf(&patchChar[0], getStringID(76), rc);
			}
			else if (rc == 0x404) {
				sprintf(&patchChar[0], getStringID(77));
			}
			else if (rc == 0x405) {
				sprintf(&patchChar[0], getStringID(78));
			}
			else if (rc == 0x406) {
				sprintf(&patchChar[0], getStringID(79));
			}
			else if (rc == 0x104) {
				sprintf(&patchChar[0], getStringID(80));
			}
			else if (rc == 0x412) {
				sprintf(&patchChar[0], getStringID(81));
			}
			else if (rc == 0x1001) {
				sprintf(&patchChar[0], getStringID(82));
			}
			else if (rc == 0x1002) {
				sprintf(&patchChar[0], getStringID(83));
			}
			else if (rc == 0x1003) {
				sprintf(&patchChar[0], getStringID(84));
			}
			else if (rc == 0x1004) {
				sprintf(&patchChar[0], getStringID(85));
			}
			else if (rc == 0x1005) {
				sprintf(&patchChar[0], getStringID(86));
			}
			else if (rc == 0x1006) {
				sprintf(&patchChar[0], getStringID(87));
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
				sprintf(&patchChar[0], getStringID(88), rc);
			}
		}
        return false;   // Return true here to signal the inputs have been consumed
    }
};