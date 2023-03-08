# FPSLocker

An overlay that with companion SaltyNX plugin `NX-FPS` allows to set custom FPS in Nintendo Switch retail games.

Disclaimer: Tool is utilizing detection of graphics API to manipulate FPS. For now it doesn't support manipulating any engine specific locks or timings. So your experience may vary.

# Requirements
- [Atmosphere CFW](https://github.com/Atmosphere-NX/Atmosphere/releases)
- [My fork of SaltyNX, version 0.5.0+](https://github.com/masagrator/SaltyNX/releases)
- [NX-FPS 1.0+](https://github.com/masagrator/NX-FPS/releases)
- Tesla environment: [ovlloader](https://github.com/WerWolv/nx-ovlloader/releases) + [Tesla Menu](https://github.com/WerWolv/Tesla-Menu/releases)

# Usage

Run game and open FPSLocker overlay. If game is supported by SaltyNX and you installed everything correctly, you will see menu where first line states `NX-FPS plugin is running`.
Explanation of each line:
- `Interval Mode` - it's used by NVN API to set limiter to either 30 FPS (2) or 60 FPS (1 or 0 (0 means that game never bothered to set it, it can be also a sign that game is not utilizing NVN but EGL or Vulkan))
- `Custom FPS Target` - it's used to lock game to certain FPS. If game is using engine proprietary FPS locks, it may not be able to unlock more than 30 FPS without additional patches.
- `Big number on the right` - it shows how many frames have passed in last second for currently running game. This is to confirm that lock is working as expected.
- `Increase/Decrease FPS target` - Change FPS Target by 5. Minimum is 15 FPS, max is 60 FPS. If FPS is set above 30 FPS, it sets `interval mode` to 1. Otherwise it sets interval to 2.
- `Disable custom FPS target` - Removes FPS Target. Since we cannot predict what interval mode is expected at this point, it is in user's discretion to manipulate FPS to bring back correct interval before disabling FPS target.
- `Sync Wait (!)` - this is dangerous setting that disabled in most cases will crash game (for example Witcher 3 and Breath of The Wild), but in some can bring benefit of disabling double buffer at the cost of small graphical glitches (for example Xenoblade Chronicles 3). Use it with caution.
- `Save settings` - save profile for currently running game that will be loaded next time by plugin on boot automatically. Don't use it if you disabled Sync Wait and you didn't test it properly that it won't cause crash, otherwise you will be forced to delete saved profile manually. Profile is saved in `SaltySD/plugins/FPSLocker/*titleid_uppercase*.dat`

# Thanks
Thanks to ~WerWolv for creating Tesla environment, and ~cucholix + ~Monked for tests.
