# FPSLocker

An overlay that with SaltyNX allows to set custom display refresh rate and FPS in Nintendo Switch retail games.

Disclaimer: Tool is utilizing detection of graphics API to manipulate FPS and in special cases it requires using patches made per game for each version to get more than 30 FPS. You can find those patches [HERE](https://github.com/masagrator/FPSLocker-Warehouse)<br>
Max supported yaml size is 32kB, though it can be expanded in next updates.

**DON'T USE 60 FPS CHEATS SIMULTANEOUSLY WITH THIS TOOL, EITHER YOU NEED A CHEAT OR FPSLOCKER, YOU DON'T NEED BOTH AT ONCE! THIS CAN CREATE COMPATIBILITY ISSUES LEADING TO CRASHES!**

# Requirements
- [Atmosphere CFW](https://github.com/Atmosphere-NX/Atmosphere/releases)
- [My fork of SaltyNX, version 1.2.0+](https://github.com/masagrator/SaltyNX/releases)
- Tesla environment: [ovlloader](https://github.com/WerWolv/nx-ovlloader/releases) + [Tesla Menu](https://github.com/WerWolv/Tesla-Menu/releases)

How to setup everything: [HERE](https://gist.github.com/masagrator/65fcbd5ad09243399268d145aaab899b)

# Usage

Overlay runs in two modes:<br>
> When game is running

If game is supported by SaltyNX and you installed everything correctly, you will see menu where first line states `NX-FPS plugin is running`.
Explanation of each line:
- `Interval Mode` - it's used by NVN API to set limiter to either 30 FPS (2) or 60 FPS (1). 
- `Custom FPS Target` - it's used to lock game to certain FPS. If game is using engine proprietary FPS locks, it may not be able to unlock more than 30 FPS without additional patches.
- `Big number on the right` - it shows how many frames have passed in last second for currently running game. This is to confirm that lock is working as expected.
- `Increase/Decrease FPS target` - Shows up only in handheld mode. Change FPS Target by 5. Minimum is 15 FPS, max is 60 FPS.
- `Change FPS target` - Shows up only in docked mode. Shows up table with different FPS values, from 15 to 60.
- `Disable custom FPS target` - Removes FPS Target. Since we cannot predict what interval mode is expected at this point, it is in user's discretion to manipulate FPS to bring back correct interval before disabling FPS target.
- `Advanced settings` - submenu which consists of:
  - If game is using NVN
    - `Window Sync Wait` - this is dangerous setting that disabled can crash game, but in some can bring benefit of disabling double buffer vsync at the cost of small graphical glitches (check list of games compatible with this solution at the bottom of README). Use it with caution. It won't show if game is not using double buffer. 
    - `Set Buffering` - if game is using any other buffering than double, this option will show that will allow you to force game to run at any other buffering that is not higher than original one (so f.e. you cannot change double buffer to triple buffer). Lowering buffer is recommended only for games that have near perfect performance at 30 or 60 FPS, but suffer from bad framepacing or big input lag. If you will force double buffer in games with uneven performance, FPS drops will be very severe. It can be applied only at boot of game, so after changing buffering you must save settings in FPSLocker and restart game. <br> Explanation of `Set/Active/Available Buffers`: 
      - Set - how many buffers were set by using `nvnWindowSetNumActiveTextures`. If game is not using it, it will be 0. It can be used by games to set lower buffer value than reserved space allows. If this is detected to be used and lower than Available Buffers, you can use "(force)" variant next to default option. Without `(force)` it will reset to default settings.
      - Active - How many buffers are actually used by game. 
      - Available - How many buffers is actually provided to NVN. We can use this information to force games to utilize all buffers when they are not doing it.
  - `Convert config to patch file` - if proper config file exists for this game and version, you will get an option to convert it to patch file that will be loaded when you will run this game next time. Patch is saved to `SaltySD/plugins/FPSLocker/patches/*titleid_uppercase*/*buildid_uppercase*.bin`
  - `Delete patch file` - if proper config file exists for this game and version, you will get an option to delete patch file so it won't be loaded when you will run this game next time.
  - `Check/download config file` - Checks in Warehouse repository if config for this game and version exists. If exists, it is downloaded and also checked if it's the same as the one on sdcard. If it's not, overlay will remove existing patch and config file, and user must manually convert new config to patch file. 0x312 error means we got unexpected file from github. Any other error code means that something is happening with your connection or github server.
- `Save settings` - save profile for currently running game that will be loaded next time by plugin on boot automatically. Don't use it if you disabled Sync Wait and you didn't test it properly that it won't cause crash. Profile is saved in `SaltySD/plugins/FPSLocker/*titleid_uppercase*.dat`
- `Display settings` - submenu related to display refresh rate. Consists of:
  - `Increase refresh rate` - Shows up only in handheld mode. Change display refresh rate up to 60 Hz. In OLED units it's blocked.
  - `Decrease refresh rate` - Shows up only in handheld mode. Change display refresh rate down to 40 Hz. In OLED units it's blocked.
  - `Change refresh rate` - Shows up only in docked mode. Choose display refresh rate from list.
  - `Display Sync` - When turned on, both options above are not available, display refresh rate is changed only when game is running, and matches refresh rate with FPS Target. In OLED units it works only in docked mode.
  - `Docked Settings` - submenu related to display refresh rate of external displays. Not accessible for Lite units. Consists of:
      - `Allowed refresh rates` - you can check and edit manually which refresh rates are enabled for currently connected external display. It consists of 40, 45, 50 and 55 Hz. By default 50 is turned on, everything else is turned off.
      - `Refresh rate wizard` - it goes automatically to next refresh rates, user is asked to press required button to confirm it's working, if not pressed for 15 seconds it goes to next refresh rate. After checking all refresh rates you are moved to `Allowed refresh rates` menu to check results.
      - `Frameskip tester` - It allows to check if your display is showing currently used signal at native refresh rates. Many displays may support for example 50 Hz, but they are still displaying stuff at 60 Hz. Instructions how to use it are provided when this menu is selected. This menu is also available in handheld mode.
      - `Additional settings` - submenu with options related to how FPSLocker/FPSLocker patches are working in docked mode. Currently you can choose from:
          - `Allow patches to force 60 Hz` - some FPSLocker patches are forcing 60 Hz to fix framepacing issues with 30 FPS cutscenes. When such change happens, game is paused for 4 seconds before continuing. By default is turned on. Turning it off will apply only FPS lock without changing refresh rate. This won't disable that 4 second delay.
          - `Use lowest refresh rate for unmatched FPS targets` - For example for 60 Hz display 35 FPS target doesn't have avaialble refresh rate matching it. By enabling this option you will get lowest enabled refresh rate in `Allowed refresh rates` menu. This option is disabled by default, which will result in setting 60 Hz in that case.

> When game is not running

You will have two submenus to choose from (if you are using OLED model, you will be redirected automatically to Games list):
- `Games list`<br>
  It will list installed games (max 32) and as first option it's available "All" submenu.<br>
  Inside each one you will find two options:
  - `Delete settings` - it will delete file created by "Save settings" option
  - `Delete patches` - it will delete file created by "Convert config to patch file" option
- `Display settings` - you can read about in previous section.

# Information about changing refresh rates

I want to use this space to clarify few things.<br>

OLED screens were getting often issues with green tint and ghosting below 60 Hz. 45 Hz works for some people properly only above certain brightness. I have decided to block OLED model completely until somebody will find more reliable way to change refresh rate for those displays.<br>

From all reports I got, only one LCD screen was getting issue with small flickering in left bottom corner when running at 40 Hz (they were using `InnoLux P062CCA-AZ2`, but there were other users who also got this display and had no issues at 40 Hz - me included). No other issues were found.<br>

I have decided to limit down to 40 Hz since lower refresh rates not only were not beneficial to user, also this allows avoid certain risks with underclocking display too much.<br>

LCD can be overclocked up to 70 Hz without immediately visible issues but I have leaved max at 60 Hz since I have decided that there are no visible benefits running display at 70 Hz which would justify tinkering with existing patches to patch games breaking above 60 FPS. From 75 Hz all users using original display panels were reporting issues with glitchy image. OLED above 65 Hz was freezing and only way to got it back working again was lowering refresh rate to 65 Hz and lower.

This works only for builtin display, you cannot change refresh rate in docked mode. 
If Display Sync is turned off, custom refresh rate is not restored after sleep mode.

Changing refresh rate affects animations speed of OS and Tesla overlays, making them more sluggigsh at lower refresh rates.

Adding support for custom display refresh rate required changing how game configs are written, so you are advised to redownload all FPSLocker Warehouse configs if you want them to properly support custom refresh rates.

I am not taking any responsibility for damages occuring from changing refresh rate. Each time you will go to `Display settings`, you will be welcomed by prompt with warning that you - user - are taking full responsibility. You must choose `Accept` to go further.

# Thanks
Thanks to ~WerWolv for creating Tesla environment, and ~cucholix + ~Monked for tests.

# Sync Wait
In those games you can disable double buffer vsync by turning off Window Sync Wait in FPSLocker:
- Batman - The Telltale Series (Warehouse patch enables triple buffer, so there is no need to use this option)
- Pokemon Legends: Arceus
- Pokemon Scarlet
- Pokemon Violet
- Sonic Frontier
- The Legend of Zelda: Tears of the Kingdom  (Warehouse patch enables triple buffer, so there is no need to use this option)
- Xenoblade Chronicles: Definitive Edition
- Xenoblade Chronicles 2
- Xenoblade Chronicles 3
