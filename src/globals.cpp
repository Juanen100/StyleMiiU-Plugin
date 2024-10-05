#include "globals.h"
#include "WUPSConfigItemThemeBool.h"
#include <string>
#include <sysapp/title.h>

bool gThemeManagerEnabled = DEFAULT_THEME_MANAGER_ENABLED;
bool gShuffleThemes       = DEFAULT_SHUFFLE_THEMES;
std::string gCurrentTheme = "";
const char* theme_directory_path = "fs:/vol/external01/wiiu/themes/";

std::string getTitleID(uint64_t tid)
{
    if(tid == 0x0005001010040000L){
        return "0005001010040000";
    } 
    else if(tid == 0x0005001010040100L){
        return "0005001010040100";
    }
    else if(tid == 0x0005001010040200L){
        return "0005001010040200";
    }
    return "0005001010040100";
}

std::string tid = getTitleID(_SYSGetSystemApplicationTitleId(SYSTEM_APP_ID_WII_U_MENU));
std::string dir_fallback = "fs:/vol/external01/wiiu/sdcafiine/" + tid + "/";

const char* theme_directory_path_fallback = dir_fallback.c_str();

ConfigItemThemeBool* gCurrentThemeItem = nullptr;

CRLayerHandle gContentLayerHandle = 0;