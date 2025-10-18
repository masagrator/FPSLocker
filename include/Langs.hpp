extern SetLanguage language;

namespace Lang {
    enum Id {
        Id_DeleteSettings,
        Id_DeletePatches,
        Id_SaltyNXIsNotWorking,
        Id_CantDetectNXFPSPluginOnSdcard,
        Id_GameIsNotRunning,
        Id_All,
        Id_Everything,
        Id_GamesList,
        Id_DisplaySettings,
        Id_ChangeFPSTarget,
        Id_GameWasClosedOverlayDisabled,
    };
}

const char* getStringID(size_t id);
const char* getTeslaStringID(std::size_t id);