#include "globals.h"
#include "WUPSConfigItemThemeBool.h"
#include <string>
#include <sysapp/title.h>

bool gThemeManagerEnabled = DEFAULT_THEME_MANAGER_ENABLED;
bool gShuffleThemes       = DEFAULT_SHUFFLE_THEMES;
std::string gCurrentTheme = "";
const char* theme_directory_path = "fs:/vol/external01/wiiu/themes/";

ConfigItemThemeBool* gCurrentThemeItem = nullptr;

CRLayerHandle gContentLayerHandle = 0;