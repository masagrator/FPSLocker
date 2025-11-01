class SetBuffers : public tsl::Gui {
public:
    SetBuffers() {}

    virtual tsl::elm::Element* createUI() override {
		auto frame = new tsl::elm::OverlayFrame(getStringID(Lang::Id_SetBuffering), " ");

		auto list = new tsl::elm::List();
		if (Shared->expectedSetBuffers == -1) list->addItem(new tsl::elm::NoteHeader(getStringID(Lang::Id_ItWillBeAppliedOnNextGameBoot), true, {0xF, 0x3, 0x3, 0xF}));
		auto *clickableListItem = new tsl::elm::ListItem2(getStringID(Lang::Id_Double));
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
			auto *clickableListItemv1 = new tsl::elm::ListItem2(getStringID(Lang::Id_Triple));
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
			if ((Shared -> Buffers) >= 3) {
				if ((Shared -> SetActiveBuffers) > 0) {
				auto *clickableListItem2 = new tsl::elm::ListItem2(getStringID(Lang::Id_TripleForce));
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
					auto *clickableListItem2 = new tsl::elm::ListItem2(getStringID(Lang::Id_Triple));
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
			}
			
			if ((Shared -> Buffers) == 4) {
				if ((Shared -> SetActiveBuffers) > 0) {
					auto *clickableListItem3 = new tsl::elm::ListItem2(getStringID(Lang::Id_QuadrupleForce));
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
					auto *clickableListItem3 = new tsl::elm::ListItem2(getStringID(Lang::Id_Quadruple));
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
        auto frame = new tsl::elm::OverlayFrame(getStringID(Lang::Id_NVNWindowSyncWait), getStringID(Lang::Id_Mode));

		auto list = new tsl::elm::List();

		auto *clickableListItem = new tsl::elm::ListItem2(getStringID(Lang::Id_Enabled));
		clickableListItem->setClickListener([](u64 keys) { 
			if ((keys & HidNpadButton_A) && PluginRunning) {
				ZeroSyncMode = getStringID(Lang::Id_On);
				(Shared -> ZeroSync) = 0;
				saveSettings();
				tsl::goBack();
				tsl::goBack();
				return true;
			}
			return false;
		});
		list->addItem(clickableListItem);

		auto *clickableListItem2 = new tsl::elm::ListItem2(getStringID(Lang::Id_SemiEnabled));
		clickableListItem2->setClickListener([](u64 keys) { 
			if ((keys & HidNpadButton_A) && PluginRunning) {
				ZeroSyncMode = getStringID(Lang::Id_Semi);
				(Shared -> ZeroSync) = 2;
				saveSettings();
				tsl::goBack();
				tsl::goBack();
				return true;
			}
			return false;
		});
		list->addItem(clickableListItem2);

		auto *clickableListItem3 = new tsl::elm::ListItem2(getStringID(Lang::Id_Disabled));
		clickableListItem3->setClickListener([](u64 keys) { 
			if ((keys & HidNpadButton_A) && PluginRunning) {
				ZeroSyncMode = getStringID(Lang::Id_Off);
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
				sprintf(&lockInvalid[0], getStringID(Lang::Id_GameConfigFileNotFound), TID, BID);
			}
			else sprintf(&lockInvalid[0], getStringID(Lang::Id_GameConfigError), configValid);
		}
		else {
			patchValid = !file_exists(&patchPath[0]);
			if (R_FAILED(patchValid)) {
				if (!FileDownloaded) {
					if (R_SUCCEEDED(configValid)) {
						sprintf(&patchChar[0], getStringID(Lang::Id_PatchFileDoesntExistMakeIt));
					}
					else sprintf(&patchChar[0], getStringID(Lang::Id_PatchFileDoesntExist));
				}
				else {
					sprintf(&patchChar[0], getStringID(Lang::Id_NewConfigDownloadedSuccessfully));
				}
			}
			else sprintf(&patchChar[0], getStringID(Lang::Id_PatchFileExists));
		}
		switch((Shared -> ZeroSync)) {
			case 0:
				ZeroSyncMode = getStringID(Lang::Id_On);
				break;
			case 1:
				ZeroSyncMode = getStringID(Lang::Id_Off);
				break;
			case 2:
				ZeroSyncMode = getStringID(Lang::Id_Semi);
		}
	}

	size_t base_height = 134;

    virtual tsl::elm::Element* createUI() override {
        auto frame = new tsl::elm::OverlayFrame("FPSLocker", getStringID(Lang::Id_AdvancedSettings));

		auto list = new tsl::elm::List();

		if ((Shared -> API)) {
			switch((Shared -> API)) {
				case 1: {
					list->addItem(new tsl::elm::CategoryHeader(getStringID(Lang::Id_GPUAPIInterfaceNVN), false));
					
					list->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
						
						renderer->drawString(&nvnBuffers[0], false, x, y+20, 20, renderer->a(0xFFFF));
							
					}), 60);

					if ((Shared -> Buffers) == 2 || (Shared -> ActiveBuffers) == 2) {
						auto *clickableListItem3 = new tsl::elm::MiniListItem(getStringID(Lang::Id_WindowSyncWait), ZeroSyncMode);
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
						auto *clickableListItem3 = new tsl::elm::MiniListItem(getStringID(Lang::Id_SetBuffering));
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
					list->addItem(new tsl::elm::CategoryHeader(getStringID(Lang::Id_GPUAPIInterfaceEGL), false));
					break;
				case 3: {
					list->addItem(new tsl::elm::CategoryHeader(getStringID(Lang::Id_GPUAPIInterfaceVulkan), false));

					list->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
						
						renderer->drawString(&nvnBuffers[0], false, x, y+20, 20, renderer->a(0xFFFF));
							
					}), 40);

					if ((Shared -> Buffers) >= 2) {
						auto *clickableListItem3 = new tsl::elm::MiniListItem(getStringID(Lang::Id_SetBuffering));
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

		list->addItem(new tsl::elm::CategoryHeader(getStringID(Lang::Id_FPSLockerPatches), false));

		if (R_FAILED(configValid)) {
			base_height = 154;
		}

		list->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
			
			if (R_SUCCEEDED(configValid)) {
				
				renderer->drawString(getStringID(Lang::Id_FoundValidConfigFile), false, x, y+20, 20, renderer->a(0xFFFF));
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
			list->addItem(new tsl::elm::NoteHeader(getStringID(Lang::Id_RememberToRebootTheGameAfterConversion), true, {0xF, 0x3, 0x3, 0xF}));
			auto *clickableListItem = new tsl::elm::MiniListItem(getStringID(Lang::Id_ConvertConfigToPatchFile));
			clickableListItem->setClickListener([](u64 keys) { 
				if ((keys & HidNpadButton_A) && PluginRunning) {
					patchValid = LOCK::createPatch(&patchPath[0]);
					if (R_SUCCEEDED(patchValid)) {
						sprintf(&patchChar[0], getStringID(Lang::Id_PatchFileCreatedSuccessfully));
					}
					else sprintf(&patchChar[0], getStringID(Lang::Id_ErrorWhileCreatingPatch), patchValid);
					return true;
				}
				return false;
			});
			list->addItem(clickableListItem);

			auto *clickableListItem2 = new tsl::elm::MiniListItem(getStringID(Lang::Id_DeletePatchFile));
			clickableListItem2->setClickListener([](u64 keys) { 
				if ((keys & HidNpadButton_A) && PluginRunning) {
					if (R_SUCCEEDED(patchValid)) {
						remove(&patchPath[0]);
						patchValid = 0x202;
						sprintf(&patchChar[0], getStringID(Lang::Id_PatchFileDeletedSuccessfully));
					}
					return true;
				}
				return false;
			});
			list->addItem(clickableListItem2);
		}
		if (R_FAILED(configValid)) {
			list->addItem(new tsl::elm::NoteHeader(getStringID(Lang::Id_ThisCanTakeUpTo30Seconds), true, {0xF, 0x3, 0x3, 0xF}));
		}
		auto *clickableListItem4 = new tsl::elm::MiniListItem(getStringID(Lang::Id_CheckDownloadConfigFile));
		clickableListItem4->setClickListener([this](u64 keys) { 
			if ((keys & HidNpadButton_A) && PluginRunning && exitPossible) {
				exitPossible = false;
				sprintf(&patchChar[0], getStringID(Lang::Id_CheckingWarehouseForConfig));
				threadCreate(&t1, downloadPatch, NULL, NULL, 0x20000, 0x3F, 3);
				threadStart(&t1);
				return true;
			}
			return false;
		});
		list->addItem(clickableListItem4);

		list->addItem(new tsl::elm::CategoryHeader(getStringID(Lang::Id_Misc), false));

		auto *clickableListItem5 = new tsl::elm::MiniToggleListItem(getStringID(Lang::Id_HaltUnfocusedGame), forceSuspend_save);
		clickableListItem5->setClickListener([this](u64 keys) { 
			if ((keys & HidNpadButton_A) && PluginRunning) {
				forceSuspend_save = !forceSuspend_save;
				(Shared -> forceSuspend) = forceSuspend_save;
				saveSettings();
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
					sprintf(patchAppliedChar, getStringID(Lang::Id_PatchWasLoadedToGame));
				}
				else if ((Shared -> patchApplied) == 2) {
					sprintf(patchAppliedChar, getStringID(Lang::Id_MasterWriteWasLoadedToGame));
				}
				else sprintf(patchAppliedChar, getStringID(Lang::Id_PluginDidntApplyPatchToGame));
				switch (Shared -> API) {
					case 1: {
						if (((Shared -> Buffers) >= 2 && (Shared -> Buffers) <= 4)) {
							sprintf(&nvnBuffers[0], getStringID(Lang::Id_SetActiveAvailableBuffers), (Shared -> SetActiveBuffers), (Shared -> ActiveBuffers), (Shared -> Buffers));
						}
						break;
					}
					case 3: {
						if (((Shared -> Buffers) >= 2 && (Shared -> Buffers) <= 4)) {
							sprintf(&nvnBuffers[0], getStringID(Lang::Id_ActiveBuffers), (Shared -> Buffers));
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
				sprintf(&patchChar[0], getStringID(Lang::Id_ConnectionTimeout));
			}
			else if (rc == 0x212 || rc == 0x312) {
				sprintf(&patchChar[0], getStringID(Lang::Id_ConfigIsNotAvailableRC), rc);
			}
			else if (rc == 0x404) {
				sprintf(&patchChar[0], getStringID(Lang::Id_ConfigIsNotAvailableExitNotPossibleUntilFinished));
			}
			else if (rc == 0x405) {
				sprintf(&patchChar[0], getStringID(Lang::Id_ConfigIsNotAvailableTimeout));
			}
			else if (rc == 0x406) {
				sprintf(&patchChar[0], getStringID(Lang::Id_ConfigIsNotAvailableConnectionError));
			}
			else if (rc == 0x104) {
				sprintf(&patchChar[0], getStringID(Lang::Id_NoNewConfigAvailable));
			}
			else if (rc == 0x412) {
				sprintf(&patchChar[0], getStringID(Lang::Id_InternetConnectionNotAvailable));
			}
			else if (rc == 0x1001) {
				sprintf(&patchChar[0], getStringID(Lang::Id_PatchIsNotNeededForThisGame));
			}
			else if (rc == 0x1002) {
				sprintf(&patchChar[0], getStringID(Lang::Id_ThisGameIsNotListedInWarehouse));
			}
			else if (rc == 0x1003) {
				sprintf(&patchChar[0], getStringID(Lang::Id_ThisGameIsListedInWarehouseWithDifferentVersionPatchNotNeeded));
			}
			else if (rc == 0x1004) {
				sprintf(&patchChar[0], getStringID(Lang::Id_ThisGameIsListedInWarehouseWithDifferentVersionPatchNeeded));
			}
			else if (rc == 0x1005) {
				sprintf(&patchChar[0], getStringID(Lang::Id_ThisGameIsListedInWarehouseWithDifferentVersionConfigAvailable));
			}
			else if (rc == 0x1006) {
				sprintf(&patchChar[0], getStringID(Lang::Id_ThisGameIsListedInWarehouseConfigNotAvailable));
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
				sprintf(&patchChar[0], getStringID(Lang::Id_ConnectionErrorRC), rc);
			}
		}
        return false;   // Return true here to signal the inputs have been consumed
    }
};