#pragma once
#include <content_redirection/redirection.h>
#include <string>

#define VERSION                                "v0.1"
#define THEME_MANAGER_ENABLED_STRING           "themeManagerEnabled"
#define CURRENT_THEME_STRING                   "currentTheme" 

#define DEFAULT_THEME_MANAGER_ENABLED          true
#define DEFAULT_CURRENT_THEME                  "default"

extern bool gThemeManagerEnabled;
extern std::string gCurrentTheme;

extern CRLayerHandle gAocLayerHandle;
extern CRLayerHandle gContentLayerHandle;