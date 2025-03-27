# StyleMiiU
This is an Aroma plugin to safely apply custom themes!

Developed intending to straight forward as much as possible the process of applying custom themes to your Wii U Home Menu without running the risk of bricking your console.

**No system file is modified by this plugin.**

Codebase is heavily based on SDCafiine's base.
## Installation
(`[ENVIRONMENT]` is a placeholder for the actual environment name.)
1. Copy the file `stylemiiu.wps` into `SD:/wiiu/environments/[ENVIRONMENT]/plugins`.
2. Requires the [WiiUPluginLoaderBackend](https://github.com/wiiu-env/WiiUPluginLoaderBackend) and [ContentRedirectionModule](https://github.com/wiiu-env/ContentRedirectionModule) in `SD:/wiiu/environments/[ENVIRONMENT]/modules`.

If you happen to have installed an older version of this plugin, delete the previous `theme_manager.wps` and rename `SD:/wiiu/environments/[ENVIRONMENT]/plugins/config/theme_manager.json` to `style-mii-u.json`

## Usage
This plugin can detect Men.pack, Men2.pack and cafe_barista_men.bfsar automatically to make the theme have any folder structure you desire. SDCaffine's file structure is still compatible with this plugin. *Additional files like AllMessage.szs do still need to be on their corresponding folder structure, like this: `SD:/wiiu/themes/[ThemeName]/content/[TheLanguageToPatch]/Message/AllMessage.szs`*

Usually themes will load from `SD:/wiiu/themes/` but, if it detects valid themes in the `SD:/wiiu/sdcafiine/` folder and `SD:/wiiu/themes/` folder doesn't exist, it will load the themes from the SDCafiine's folder. **However, it is recommended to move them to the `SD:/wiiu/themes/` folder to prevent issues between SDCafiine and StyleMiiU as it may cause issues when loading themes.**

A step to step guide on how to use it:
1. Open the WUPS menu (**L + ↓ + SELECT**) and enter the **StyleMiiU** menu. Make sure StyleMiiU is enabled, otherwise it won't do anything.
2. Enter the **Available Themes** option and you should see the folders that are in `SD:/wiiu/themes/` (Or `SD:/wiiu/sdcafiine/` in case no themes folder exist), showing up empty if no theme is present. Once there, set whatever theme you want to Current Theme. If no theme it's set, the Wii U's default home menu will be loaded.
3. Once you exit the WUPS menu, the Home Menu should restart and once it gets restarted, your Home Menu will now have the theme you chose applied to it.

Additional options:
- Shuffle themes: This option tries to mimic the 3DS' option of shuffling themes, you can use it by simply activating the option and then in **Available Themes**, select the themes you want to shuffle through. The themes will change **every time you restart the console or enter and exit to the menu**. If you disable this option, then the last theme you selected will become the default theme for it to fallback on.

## Troubleshooting
The plugin will sometimes not work properly when certain race conditions are triggered. The most commons ones being described further in this file.

### Bootlooping on Wii U Menu splash
The first reason might be you have both SDCafiine and StyleMiiU trying to patch a theme to the Wii U menu, which can cause issues. I'd recommend moving the themes from SDCafiine to StyleMiiU's themes folder if you want to use this plugin instead of SDCafiine.

This problem might also show up due to the fact that you're using a theme that hasn't been tested or properly made to support an European/PAL console. European/PAL consoles have more language packs built onto the menu menaning the file size on themes needs to be smaller to work properly. This is more likely to happen with animated themes.

### My theme isn't loading
The most common issue would be having bad or corrupted Men.pack, Men2.pack or cafe_barista_men.bfsar files which doesn't allow them from loading. This is highly unlikely to happen and the reason might also be the same as the bootlooping problem on EU consoles.


## Building
Requires the [libcontentredirection](https://github.com/wiiu-env/libcontentredirection) and [wut](https://github.com/devkitPro/wut)

Should be as easy as running `make` and let it do its thing.
