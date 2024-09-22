#include "globals.h"

bool gThemeManagerEnabled = DEFAULT_THEME_MANAGER_ENABLED;
bool gShuffleThemes       = DEFAULT_SHUFFLE_THEMES;
std::string gCurrentTheme = "";
const char* theme_directory_path = "fs:/vol/external01/wiiu/themes/";

CRLayerHandle gContentLayerHandle = 0;
CRLayerHandle gAocLayerHandle     = 0;
