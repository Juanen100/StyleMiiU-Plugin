# StyleMiiU
This is an Aroma plugin to safely apply custom themes!

Developed intending to straight forward as much as possible the process of applying custom themes to your Wii U Home Menu without running the risk of bricking your console.

**No system file is modified by this plugin.**

Codebase is heavily based on SDCafiine's base.
## Installation
(`[ENVIRONMENT]` is a placeholder for the actual environment name.)
1. Copy the file `stylemiiu.wps` into `SD:/wiiu/environments/[ENVIRONMENT]/plugins`.
2. Requires the [WiiUPluginLoaderBackend](https://github.com/wiiu-env/WiiUPluginLoaderBackend) and [ContentRedirectionModule](https://github.com/wiiu-env/ContentRedirectionModule) in `SD:/wiiu/environments/[ENVIRONMENT]/modules`.

If you happen to have installed an older version of this plugin, delete the previous `theme_manager.wps` and rename `sd:/wiiu/environments/[ENVIRONMENT]/plugins/config/theme_manager.json` to `style-mii-u.json`

## Usage
This plugin can detect Men.pack, Men2.pack and cafe_barista_men.bfsar automatically to make the theme have any folder structure you desire. SDCaffine's file structure is still compatible with this plugin. 

Usually themes will load from `SD:/wiiu/themes/` but, if it detects valid themes in the `SD:/wiiu/sdcafiine/` folder and `SD:/wiiu/themes/` folder doesn't exist, it will load the themes from the SDCafiine's folder.

A step to step guide on how to use it:
1. Open the WUPS menu (**L + ↓ + SELECT**) and enter the **StyleMiiU** menu. Make sure StyleMiiU is enabled, otherwise it won't do anything.
2. Enter the **Available Themes** option and you should see the folders that are in `SD:/wiiu/themes/` (Or `SD:/wiiu/sdcafiine/` in case no themes folder exist), showing up empty if no theme is present. Once there, set whatever theme you want to Current Theme. If no theme it's set, the Wii U's default home menu will be loaded.
3. Once you exit the WUPS menu, the Home Menu should restart and once it gets restarted, your Home Menu will now have the theme you chose applied to it.

Additional options:
- Shuffle themes: This option tries to mimic the 3DS' option of shuffling themes, you can use it by simply activating the option and then in **Available Themes**, select the themes you want to shuffle through. The themes will change **every time you restart the console or enter and exit to the menu**. If you disable this option, then the last theme you selected will become the default theme for it to fallback on.

## Building
Requires the [libthemeredirection](https://github.com/Juanen100/libthemeredirection) and [wut](https://github.com/devkitPro/wut)

Should be as easy as running `make` and let it do its thing.
