#include <array>

namespace ENG {
	const std::array strings = {
		//Main Menu
		"Delete settings",
		"Delete patches",
		"SaltyNX is not working!",
		"Can't detect NX-FPS plugin on sdcard!",
		"Game is not running!",
		"All",
		"Everything",
		"Games list",
		"Display settings",
		"Game was closed! Overlay disabled!",
		"Game is not running! Overlay disabled!",
		"Game is running.",
		"NX-FPS is not running!",
		"NX-FPS is running, waiting for frame.",
		"Restart overlay to check again.",
		"NX-FPS is running.",
		"Patch is now forcing 60 Hz.",
		"Increase FPS target",
		"Decrease FPS target",
		"Change FPS target",
		"Disable custom FPS target",
		"Advanced settings",
		"Interval Mode:\n\uE019 0 (Unused)",
		"Interval Mode:\n\uE019 %d (%.1f FPS)",
		"Interval Mode:\n\uE019 %d (%d FPS)",
		"Interval Mode:\n\uE019 %d (Wrong)",
		"Custom FPS Target:\n\uE019 Disabled",
		"Custom FPS Target:\n\uE019 %d",

		//Advanced Settings
		"Set Buffering",
		"It will be applied on next game boot.",
		"Double",
		"Triple",
		"Triple (force)",
		"Quadruple (force)",
		"Quadruple",
		"NVN Window Sync Wait",
		"Mode",
		"Enabled",
		"Semi-Enabled",
		"Disabled",
		"On",
		"Off",
		"Semi",
		"Game config file not found\nTID: %016lX\nBID: %016lX",
		"Game config error: 0x%X",
		"Patch file doesn't exist.\nUse \"Convert config to patch file\"\nto make it!",
		"Patch file doesn't exist.",
		"New config downloaded successfully.\nUse \"Convert config to patch file\"\nto make it applicable!",
		"Patch file exists.",
		"GPU API Interface: NVN",
		"Window Sync Wait",
		"GPU API Interface: EGL",
		"GPU API Interface: Vulkan",
		"FPSLocker Patches",
		"Found valid config file!",
		"Remember to reboot the game after conversion!",
		"Convert config to patch file",
		"Patch file created successfully.\nRestart the game and change\nFPS Target to apply the patch!",
		"Error while creating patch: 0x%x",
		"Delete patch file",
		"Patch file deleted successfully.",
		"This can take up to %d seconds.",
		"Check/download config file",
		"Checking Warehouse for config...",
		"Misc",
		"Halt unfocused game",
		"Patch was loaded to game.",
		"Master Write was loaded to game.",
		"Plugin didn't apply patch to game.",
		"Set/Active/Available buffers: %d/%d/%d",
		"Active buffers: %d",
		"Connection timeout!",
		"Config is not available! RC: 0x%x",
		"Config is not available!\nChecking Warehouse for more info...",
		"Config is not available!\nChecking Warehouse for more info...\nTimeout! It took too long to check.",
		"Config is not available!\nChecking Warehouse for more info...\nConnection error!",
		"No new config available.",
		"Internet connection not available!",
		"Patch is not needed for this game!",
		"This game is not listed in Warehouse!",
		"This game is listed in Warehouse,\nbut with different version.\n%s doesn't need a patch,\nyour version maybe doesn't need it too!",
		"This game is listed in Warehouse,\nbut with different version.\n%s recommends patch,\nbut config is not available even for it!",
		"This game is listed in Warehouse,\nbut with different version.\n%s has config available!",
		"This game is listed in Warehouse\nwith patch recommended for this\nversion, but config is not available!",
		"Connection error! RC: 0x%x",
		
		//Display Settings
		"Frameskip tester",

		"How to use it:\n"
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
		"to divide evenly frametimes.",

		"Press \uE0E1 to exit",
		"Rendering takes too long!\nClose game, go to home screen,\ntry again.",
		
		"This menu will go through all\n"
		"supported refresh rates below 60 Hz:\n"
		"40, 45, 50, 55. Press button you are\n"
		"asked for to confirm that it works.\n"
		"If nothing is pressed in 15 seconds,\n"
		"it will check next refresh rate.",

		"To start press X.",
		"Display underclock wizard",
		"Not supported at %dp!",
		"Close game first!",
		"Press ZL to confirm %d Hz is working.",
		"Press X to confirm %d Hz is working.",
		"Press Y to confirm %d Hz is working.",
		"Press ZR to confirm %d Hz is working.",

		"This menu will go through all\n"
		"supported refresh rates above 60 Hz\n"
		"up to %d Hz.\n\n"
		"Press button you are asked for\n"
		"to confirm that it works.\n"
		"If nothing is pressed in 10 seconds,\n"
		"it will check next refresh rate.\n"
		"This can take up to %d seconds.",

		"Display overclock wizard",
		"Docked %dp display manual settings",
		"Docked display additional settings",
		"Allow patches to force 60 Hz",
		"Use lowest refresh rate for unmatched FPS targets",
		"n/d",
		"Max refresh rate available: %u Hz\nmyDP link rate: %s\nConfig ID: %08X",
		"Docked display settings",

		"Allowed %dp refresh rates",
		"%dp overclock wizard",
		"Additional settings",

		"You are not in docked mode.\n"
		"Go back, put your Switch to dock\n"
		"and come back.",

		"Change Refresh Rate",
		"Increase Refresh Rate",
		"Decrease Refresh Rate",
		"Match refresh rate with FPS Target.",
		"Handheld Display Sync",
		"Docked Settings",
		"Retro Remake Mode",
		"Display Refresh Rate: %d Hz",

		"THIS IS EXPERIMENTAL FUNCTION!\n\n"
		"It can cause irreparable damage\n"
		"to your display.\n\n"
		"By pressing Accept you are taking\n"
		"full responsibility for anything\n"
		"that can occur because of this tool.",

		"Display settings warning",
		"Decline",
		"Accept",
		"Force English language",
		"Docked Display Sync",
		"Handheld only",
		"60 Hz in HOME Menu"
	};

	const std::array teslaStrings = {
	   "Back",
	   "OK" 
	};

	static_assert(teslaStrings.size() == 2);
}