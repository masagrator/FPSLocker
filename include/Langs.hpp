extern SetLanguage language;

namespace Lang {
    enum Id {
        //Main Menu
        Id_DeleteSettings,
        Id_DeletePatches,
        Id_SaltyNXIsNotWorking,
        Id_CantDetectNXFPSPluginOnSdcard,
        Id_GameIsNotRunning,
        Id_All,
        Id_Everything,
        Id_GamesList,
        Id_DisplaySettings,
        Id_GameWasClosedOverlayDisabled,
        Id_GameIsNotRunningOverlayDisabled,
        Id_GameIsRunning,
        Id_NXFPSIsNotRunning,
        Id_NXFPSIsRunningWaitingForFrame,
        Id_RestartOverlayToCheckAgain,
        Id_NXFPSIsRunning,
        Id_PatchIsNotForcing60Hz,
        Id_IncreaseFPSTarget,
        Id_DecreaseFPSTarget,
        Id_ChangeFPSTarget,
        Id_DisableCustomFPSTarget,
        Id_AdvancedSettings,
        Id_IntervalMode0,
        Id_IntervalModeFloatFPS,
        Id_IntervalModeIntegerFPS,
        Id_IntervalModeWrong,
        Id_CustomFPSTargetDisabled,
        Id_CustomFPSTarget,

        //Advanced Settings
        Id_SetBuffering,
        Id_ItWillBeAppliedOnNextGameBoot,
        Id_Double,
        Id_Triple,
        Id_TripleForce,
        Id_QuadrupleForce,
        Id_Quadruple,
        Id_NVNWindowSyncWait,
        Id_Mode,
        Id_Enabled,
        Id_SemiEnabled,
        Id_Disabled,
        Id_On,
        Id_Off,
        Id_Semi,
        Id_GameConfigFileNotFound,
        Id_GameConfigError,
        Id_PatchFileDoesntExistMakeIt,
        Id_PatchFileDoesntExist,
        Id_NewConfigDownloadedSuccessfully,
        Id_PatchFileExists,
        Id_GPUAPIInterfaceNVN,
        Id_WindowSyncWait,
        Id_GPUAPIInterfaceEGL,
        Id_GPUAPIInterfaceVulkan,
        Id_FPSLockerPatches,
        Id_FoundValidConfigFile,
        Id_RememberToRebootTheGameAfterConversion,
        Id_ConvertConfigToPatchFile,
        Id_PatchFileCreatedSuccessfully,
        Id_ErrorWhileCreatingPatch,
        Id_DeletePatchFile,
        Id_PatchFileDeletedSuccessfully,
        Id_ThisCanTakeUpTo30Seconds,
        Id_CheckDownloadConfigFile,
        Id_CheckingWarehouseForConfig,
        Id_Misc,
        Id_HaltUnfocusedGame,
        Id_PatchWasLoadedToGame,
        Id_MasterWriteWasLoadedToGame,
        Id_PluginDidntApplyPatchToGame,
        Id_SetActiveAvailableBuffers,
        Id_ActiveBuffers,
        Id_ConnectionTimeout,
        Id_ConfigIsNotAvailableRC,
        Id_ConfigIsNotAvailableExitNotPossibleUntilFinished,
        Id_ConfigIsNotAvailableTimeout,
        Id_ConfigIsNotAvailableConnectionError,
        Id_NoNewConfigAvailable,
        Id_InternetConnectionNotAvailable,
        Id_PatchIsNotNeededForThisGame,
        Id_ThisGameIsNotListedInWarehouse,
        Id_ThisGameIsListedInWarehouseWithDifferentVersionPatchNotNeeded,
        Id_ThisGameIsListedInWarehouseWithDifferentVersionPatchNeeded,
        Id_ThisGameIsListedInWarehouseWithDifferentVersionConfigAvailable,
        Id_ThisGameIsListedInWarehouseConfigNotAvailable,
        Id_ConnectionErrorRC,

        //Display Settings
        Id_FrameskipTester,

        Id_HowToUseIt,
        //
        //
        //
        //
        //
        //
        //
        //
        //
        //
        //
        //
        //
        //
        //
        //
        //
        //
        //
        //
        //
        
        Id_PressToExit,
        Id_RenderingTakesTooLong,

        Id_ThisMenuWillGoThroughTo55Hz,
        //
        //
        //
        //
        //

        Id_ToStartPressX,
        Id_DisplayUnderclockWizard,
        Id_NotSupportedAtdp,
        Id_CloseGameFirst,
        Id_PressZlToConfirm,
        Id_PressXToConfirm,
        Id_PressYToConfirm,
        Id_PressZrToConfirm,

        Id_ThisMenuWillGoThroughUpTodHz,
        //
        //
        //
        //
        //
        //
        //

        Id_DisplayOverclockWizard,
        Id_DockedDisplayManualSettings,
        Id_DockedDisplayAdditionalSettings,
        Id_AllowPatchesToForce60Hz,
        Id_UseLowestRefreshRate,
        Id_NoData,
        Id_MaxRefreshRateAvailable,
        Id_DockedDisplaySettings,

        Id_AllowedRefreshRates,
        Id_pOverclockWizard,
        Id_AdditionalSettings,

        Id_YouAreInDockedMode,
        //
        //

        Id_ChangeRefreshRate,
        Id_IncreaseRefreshRate,
        Id_DecreaseRefreshRate,
        Id_MatchRefreshRateWithFPSTarget,
        Id_HandheldDisplaySync,
        Id_DockedSettings,
        Id_RetroRemakeMode,
        Id_DisplayRefreshRate,

        Id_ThisIsExperimentalFunction,
        //
        //
        //
        //
        //

        Id_DisplaySettingsWarning,
        Id_Decline,
        Id_Accept,
        Id_ForceEnglishLanguage,
        Id_DockedDisplaySync,
        Id_HandheldOnly,
        Id_60HzInHOMEMenu,
        
        Id_total_number
    };
}

const char* getStringID(std::size_t id);
const char* getTeslaStringID(std::size_t id);