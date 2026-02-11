# FPSLocker

An overlay that, with SaltyNX, allows you to set custom display refresh rate and FPS in Nintendo Switch retail games.

> [!NOTE]
> The tool utilizes detection of the graphics API to manipulate FPS, and in special cases, it requires using patches made specifically for each game version to achieve more than 30 FPS. Overlay has a built-in option to download configs used to make patches. Repository storing those configs can be found [HERE](https://github.com/masagrator/FPSLocker-Warehouse)<br>
Max supported YAML size is 32kB, though it can be expanded in the next updates.

> [!WARNING]
> IT IS NOT ADVISED TO USE 60 FPS CHEATS/MODS SIMULTANEOUSLY WITH THIS TOOL, EITHER YOU NEED A CHEAT OR FPSLOCKER, YOU DON'T NEED BOTH AT ONCE! THIS CAN CREATE COMPATIBILITY ISSUES LEADING TO CRASHES!

# Requirements
- [Atmosphere CFW](https://github.com/Atmosphere-NX/Atmosphere/releases)
- [My fork of SaltyNX, version 1.6.0+](https://github.com/masagrator/SaltyNX/releases)
- Tesla environment: [ovlloader](https://github.com/WerWolv/nx-ovlloader/releases) + [Tesla Menu](https://github.com/WerWolv/Tesla-Menu/releases)
- Overclocking toolset (And don't expect to run games in docked mode at locked 60 FPS without ridiculously beefy clocks, no - 1963/998/2133 clocks are not beefy enough in most cases)
- If you have Switch OLED and you plan to play docked with display that supports refresh rates higher than 60 Hz, it is adviced to install [sys-dock](https://github.com/masagrator/sys-dock/releases) and read its [README](https://github.com/masagrator/sys-dock/blob/main/README.md) to unlock 120 Hz in OLED dock + possibly other ones.

How to set up everything: [HERE](https://gist.github.com/masagrator/65fcbd5ad09243399268d145aaab899b)

# Usage

Supported languages: English, German, French, Russian, Brazilian Portuguese, Chinese Simplified, Chinese Traditional.

Overlay runs in two modes:<br>
> When the game is running

If the game is supported by SaltyNX and you installed everything correctly, you will see a menu where the first line states `NX-FPS plugin is running`.

**tl;dr**<br>
The best approach if you want to run 30 FPS games at higher FPS: 
1. Run game, connect your Switch to the internet, in FPSLocker go to `Advanced Settings`, press `Check/download config file`. If your game and version are compatible with the FPSLocker Warehouse repository, the menu will be refreshed with the option `Convert config to patch` appearing. Press on it, restart the game, and now change the FPS target in FPSLocker.
2. Go to Advanced Settings, if you see "Set/Active/Available buffers: 2/2/3", press on `Set buffering`, choose `Triple (force)`.

**Explanation of each option and information**:
- `Interval Mode` - It's used by NVN and EGL API to set vsync. Value 2 means that every frame shows at least 2x longer than by default, so at 60 Hz display you get 30 FPS max. Accepted range is 1-4. Unset value reported as 0 is treated the same way as 1.
- `Custom FPS Target` - It's used to lock the game to a certain FPS. If the game is using engine proprietary FPS locks, it may not be able to unlock more than 30 FPS without additional patches.
- `FPS` - It shows how many frames have passed in the last second for the currently running game. This is to confirm that the lock is working as expected.
- `Patch file doesn't exist.` - It shows up when overlay is 100% sure that for FPSLocker to properly work in this specific game it needs FPSLocker patch, but you don't have one. Read `tl;dr` how to get config and convert it to patch (though config may not exist for your game generally or for specific game's version you are using).
- `Increase/Decrease FPS target` - Shows up only in handheld mode. Change FPS Target by 5. Minimum is 15 FPS, max is 60 FPS.
- `Change FPS target` - Shows up only in docked mode. Shows up table with different FPS values, from 15 to 60 by default with possibility of expanding to 120 FPS.
- `Disable custom FPS target` - Removes FPS Target. Since we cannot predict what interval mode is expected at this point, it is in user's discretion to manipulate FPS to bring back correct interval before disabling FPS target.
- `Advanced settings` - submenu which consists of:
  - If game is using NVN
    - `Window Sync Wait` - this is dangerous setting that disabled can crash game, but in some can bring benefit of disabling double buffer vsync at the cost of small graphical glitches (check list of games compatible with this solution at the bottom of README). Use it with caution. It won't show if game is not using double buffer. 
    - `Set Buffering` - if game is using any other buffering than double, this option will show that will allow you to force game to run at any other buffering that is not higher than original one (so f.e. you cannot change double buffer to triple buffer). Lowering buffer is recommended only for games that have near perfect performance at 30 or 60 FPS, but suffer from bad framepacing or big input lag. If you will force double buffer in games with uneven performance, FPS drops will be very severe. In some games it can be applied only at boot of game, so after changing buffering you may be forced to restart game (such info will pop up inside menu if it's needed). <br> Explanation of `Set/Active/Available Buffers`: 
      - Set - how many buffers were set by using `nvnWindowSetNumActiveTextures`. If game is not using it, it will be 0. It can be used by games to set lower buffer value than reserved space allows. If this is detected to be used and lower than Available Buffers, you can use "(force)" variant next to default option. Without `(force)` it will reset to default settings.
      - Active - How many buffers are actually used by game. 
      - Available - How many buffers is actually provided to NVN. We can use this information to force games to utilize all buffers when they are not doing it.
  - If game is using Vulkan
    - `Set Buffering` - switch between double buffer and triple buffer. Lowering buffer is recommended only for games that have near perfect performance at 30 or 60 FPS, but suffer from bad framepacing or big input lag. If you will force double buffer in games with uneven performance, FPS drops will be very severe. It can be applied only at boot of game, so after changing buffering you must restart game.
  - `Convert config to patch file` - if proper config file exists for this game and version, you will get an option to convert it to patch file that will be loaded when you will run this game next time. Patch is saved to `SaltySD/plugins/FPSLocker/patches/*titleid_uppercase*/*buildid_uppercase*.bin`
  - `Delete patch file` - if proper config file exists for this game and version, you will get an option to delete patch file so it won't be loaded when you will run this game next time.
  - `Check/download config file` - Checks in Warehouse repository if config for this game and version exists. If exists, it is downloaded and also checked if it's the same as the one on sdcard. If it's not, overlay will remove existing patch and config file, and user must manually convert new config to patch file. 0x312 error means we got unexpected file from github. Any other error code means that something is happening with your connection or github server.
  - `Halt unfocused game` - Some games are not suspended when your Switch is in home menu. Enabling this option forces kernel to suspend game asap if game is out of focus.
- `Display settings` - submenu related to display refresh rate. Consists of:
  - `Increase refresh rate` - Shows up only in handheld mode. Change display refresh rate up to 60 Hz.
  - `Decrease refresh rate` - Shows up only in handheld mode. Change display refresh rate down to 40 Hz (for OLED to 45 Hz). 
  - `Change refresh rate` - Shows up only in docked mode. Choose display refresh rate from list.
  - `Handheld Display Sync`/`Docked Display Sync` - When turned on, all three options above are not available, display refresh rate is changed only when game is running, and matches refresh rate with FPS Target.
  - `60 HZ in HOME Menu` - if Handheld Display Sync is turned on, whenever you go to HOME Menu while game is running SaltyNX will always make sure to run it at 60 Hz in handheld.
  - `Retro Remake Mode` - this option shows only for people that use Lite with screen `InnoLux 2J055IA-27A (Rev B1)` or `Retro Remake SUPER5` (first revision only). That is because Retro Remake displays require special approach to change refresh rate, and first version of SUPER5 is spoofing ID of already existing display, which makes it impossible to detect which one is in use, so user must manually enable it if they are using SUPER5 display. All other Retro Remake displays are detected automatically.
  - `Docked Settings` - submenu related to display refresh rate of external displays. Not accessible for Lite units. Consists of:
      - `myDP link rate` - It will report `HBR` or `HBR2` mode. HBR mode doesn't allow going higher than 75 Hz at 1080p for non-OLED units, for OLED units it depends on how much DP lanes your dock supports (60 Hz is max if you want audio to work). More at the bottom of readme.
      - `Config ID` - What is the name of config file used to store settings for your currently connected display. You can find file in `SaltySD/plugins/FPSLocker/ExtDisplays` folder.
      - `Allowed refresh rates` - you can check and edit manually which refresh rates are enabled for currently connected external display. It consists of 40, 45, 50 and 55 Hz. By default 50 is turned on, everything else is turned off.
      - `Display underclock wizard` - it goes automatically through refresh rates from 40 to 55, user is asked to press required button to confirm it's working, if not pressed for 15 seconds it goes to next refresh rate. After checking all refresh rates you are moved to `Allowed refresh rates` menu to check results.
      - `Display overclock wizard` - it shows only if external display reported max refresh rate is equal or above 70 Hz. Goes automatically through refresh rates from 70 to max your display supports with cap being 120 Hz, user is asked to press required button to confirm it's working, if not pressed for 10 seconds it goes to next refresh rate. After checking all refresh rates you are moved to `Allowed refresh rates` menu to check results.
      - `Frameskip tester` - It allows to check if your display is showing currently used signal at native refresh rates. Many displays may support for example 50 Hz, but they are still displaying stuff at 60 Hz. Instructions how to use it are provided when this menu is selected. This menu is also available in handheld mode.
      - `Additional settings` - submenu with options related to how FPSLocker/FPSLocker patches are working in docked mode. Currently you can choose from:
          - `Allow patches to force 60 Hz` - some FPSLocker patches are forcing 60 Hz to fix framepacing issues with 30 FPS cutscenes. When such change happens, game is paused for 4 seconds before continuing. By default is turned on. Turning it off will apply only FPS lock without changing refresh rate and without delay.
          - `Use lowest refresh rate for unmatched FPS targets` - For example for 60 Hz display 35 FPS target may not have available refresh rate matching it. By enabling this option you will get lowest enabled refresh rate in `Allowed refresh rates` menu. This option is disabled by default, which will result in setting 60 Hz in that case.
          - `60 HZ in HOME Menu` - if Docked Display Sync is turned on, whenever you go to HOME Menu while game is running SaltyNX will always make sure to run it at 60 Hz for this particular display.

> When game is not running

You will have two submenus to choose from:
- `Games list`<br>
  It will list installed games (max 32), first option available is "All" submenu.<br>
  Inside each one you will find two options:
  - `Delete settings` - as name implies
  - `Delete patches` - it will delete file created by "Convert config to patch file" option
- `Display settings` - you can read about in previous section.
- `Force English language` - If you prefer using English, this option will force overlay to use it. It is achieved by self-modifying executable, so after updating overlay to newer release it will be turned off.

# Information about changing refresh rates in handheld mode

I want to use this space to clarify a few things.<br>

Switch OLED displays require gamma color correction after changing the refresh rate. I am modifying OLED panel registers to adjust the gamma curve to be as close to the original experience as possible. But because those registers have very big steps, it's not possible to do it perfectly, so there are small discrepancies in colors (The worst case I found is 60% brightness at 45 Hz).<br>

From all reports I got, only one LCD screen was getting an issue with small flickering in the left bottom corner when running at 40 Hz (they were using `InnoLux P062CCA-AZ2`, but there were other users who also got this display and had no issues at 40 Hz - me included). No other issues were found.<br>

Retro Remake displays require time to adjust themselves to a new signal, which is why refresh rate is applied with a delay on those displays. Too short a delay between attempts in changing refresh rate results in a black screen, which can be fixed by going to sleep mode/turning off/restarting the Switch. If you are affected, write your case in Issues so I can increase the delay in SaltyNX.

I have decided to limit LCDs and Retro Remake displays down to 40 Hz since lower refresh rates are not only not beneficial to the user, but this choice also allows for avoiding certain risks with underclocking the display too much. For Switch OLED, that limit is 45 Hz because at 40 Hz it misbehaves.<br>

LCD can be overclocked up to 70 Hz without immediately visible issues, but I have left max at 60 Hz for now. From 75 Hz, all users using original display panels were reporting issues with a glitchy image. OLED above 60 Hz is misbehaving.

If Display Sync is turned off, the custom refresh rate is not restored after sleep mode.

Changing the refresh rate affects the animations' speed of OS and Tesla overlays, making them more sluggish at lower refresh rates.

I am not taking any responsibility for damages occurring from changing the refresh rate. Each time you go to `Display settings`, you will be welcomed by a prompt with a warning that you - the user - are taking full responsibility. You must choose `Accept` to go further.

# Information about changing refresh rates in docked mode

Cap was set to 120 Hz as this is the max refresh rate supported by the OG dock and non-OLED switches, which makes it the most universal.

Many displays are locked to max 75Hz at 1080p because, from what I understand, something hinders the connection between the Switch and dock, which ends in failed HBR2 training implemented in HOS, leaving the connection in HBR mode. HBR mode with 2 lanes is where the 180 MHz limit, which is slightly above what 1080p 75 Hz expects, comes from. The issue may come from the Switch and/or the dock itself. Being in HBR mode can also result in audio not being passed to the dock above 60 Hz at 1080p. Trying to manually do HBR2 training results in resetting the signal, which HOS tries to restore, so any attempt at manual training gets blocked.

OLED is a curious case because Nintendo applied software PCIE lane bandwidth cap forcing it to be always in HBR mode. sys-dock sysmodule allows overriding those limitations.

From tests, HOS applets can get unstable at 100 Hz and higher. That means, e.g., if a currently running game wants to pop the user selection applet, this may result in the game's crash. Some games can get unstable on their own. As an example, "Batman: The Enemy Within", when being closed above a certain refresh rate, will crash.

# Thanks
Thanks to:
- ~WerWolv for creating Tesla environment
- ~cucholix + ~Monked for tests
- ~CTCaer for info about Samsung OLED panels
- ~NaGa for tests on Retro Remake SUPER5 display and providing me with a Switch OLED unit
  - and backers, including: Jorge Conceição, zany tofu, Lei Feng, brandon foster, AlM, Alex Haley, Stefano Bigio, Le Duc, Sylvixor x
- Anonymous contributor who found how to unlock full refresh rate range for Switch OLED in dock.
 
Translation:
- German: ~Lightos_
- French: ~ganonlebucher
- Russian: ~usagi, ~redraz
- Brazilian Portuguese: ~Fl4sh
- Chinese Simplified: ~Soneoy, ~Tone Darkwell
- Chinese Traditional: [david082321](https://github.com/david082321)

# Sync Wait
In those games, you can disable double buffer vsync by turning off Window Sync Wait in FPSLocker:
- Batman - The Telltale Series (Warehouse patch enables triple buffer, so there is no need to use this option)
- Pokémon Legends: Arceus
- Pokémon Legends: Z-A
- Pokémon Scarlet
- Pokémon Violet
- Sonic Frontier
- The Legend of Zelda: Tears of the Kingdom  (Warehouse patch enables triple buffer, so there is no need to use this option)
- Xenoblade Chronicles: Definitive Edition
- Xenoblade Chronicles 2
- Xenoblade Chronicles 3
- Xenoblade Chronicles X
