# FPSLocker

An overlay that with companion SaltyNX plugin `NX-FPS` allows to set custom FPS in Nintendo Switch retail games.

Disclaimer: Tool is utilizing detection of graphics API to manipulate FPS and in special cases it requires using patches made per game for each version to get more than 30 FPS. You can find those patches [HERE](https://github.com/masagrator/FPSLocker-Warehouse)<br>
Max supported yaml size is 32kB, though it can be expanded in next updates.

# Requirements
- [Atmosphere CFW](https://github.com/Atmosphere-NX/Atmosphere/releases)
- [My fork of SaltyNX, version 0.6.0+](https://github.com/masagrator/SaltyNX/releases)
- [NX-FPS 1.5.1+](https://github.com/masagrator/NX-FPS/releases)
- Tesla environment: [ovlloader](https://github.com/WerWolv/nx-ovlloader/releases) + [Tesla Menu](https://github.com/WerWolv/Tesla-Menu/releases)

How to setup everything: [HERE](https://gist.github.com/masagrator/65fcbd5ad09243399268d145aaab899b)

# Usage

Overlay runs in two modes:<br>
> When game is running

If game is supported by SaltyNX and you installed everything correctly, you will see menu where first line states `NX-FPS plugin is running`.
Explanation of each line:
- `Interval Mode` - it's used by NVN API to set limiter to either 30 FPS (2) or 60 FPS (1 or 0 (0 means that game never bothered to set it, it can be also a sign that game is not utilizing NVN but EGL or Vulkan))
- `Custom FPS Target` - it's used to lock game to certain FPS. If game is using engine proprietary FPS locks, it may not be able to unlock more than 30 FPS without additional patches.
- `Big number on the right` - it shows how many frames have passed in last second for currently running game. This is to confirm that lock is working as expected.
- `Increase/Decrease FPS target` - Change FPS Target by 5. Minimum is 15 FPS, max is 60 FPS. If FPS is set above 30 FPS, it sets `interval mode` to 1. Otherwise it sets interval to 2.
- `Disable custom FPS target` - Removes FPS Target. Since we cannot predict what interval mode is expected at this point, it is in user's discretion to manipulate FPS to bring back correct interval before disabling FPS target.
- Advanced settings - submenu which consists of:
  - `Window Sync Wait` - this is dangerous setting that disabled can crash game, but in some can bring benefit of disabling double buffer vsync at the cost of small graphical glitches (check list of games compatible with this solution at the bottom of README). Use it with caution. It won't show if game is not using double buffer. 
  - `Convert config to patch file` - if proper config file exists for this game and version, you will get an option to convert it to patch file that will be loaded when you will run this game next time. Patch is saved to `SaltySD/plugins/FPSLocker/patches/*titleid_uppercase*/*buildid_uppercase*.bin`
  - `Delete patch file` - if proper config file exists for this game and version, you will get an option to delete patch file so it won't be loaded when you will run this game next time.
- `Save settings` - save profile for currently running game that will be loaded next time by plugin on boot automatically. Don't use it if you disabled Sync Wait and you didn't test it properly that it won't cause crash. Profile is saved in `SaltySD/plugins/FPSLocker/*titleid_uppercase*.dat`

> When game is not running

It will list installed games (max 32) and as first option it's available "All" submenu.<br>
Inside each one you will find two options:
- `Delete settings` - it will delete file created by "Save settings" option
- `Delete patches` - it will delete file created by "Convert config to patch file" option

# Thanks
Thanks to ~WerWolv for creating Tesla environment, and ~cucholix + ~Monked for tests.

# Sync Wait
In those games you can disable double buffer by turning off Sync Wait in FPSLocker:
- Pokemon Legends: Arceus
- Pokemon Scarlet
- Pokemon Violet
- The Legend of Zelda: Tears of the Kingdom 
- Xenoblade Chronicles: Definitive Edition
- Xenoblade Chronicles 2
- Xenoblade Chronicles 3
