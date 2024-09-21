#pragma once
#include <content_redirection/redirection.h>
#include <string>

#define VERSION                                "v1.0"
#define THEME_MANAGER_ENABLED_STRING           "themeManagerEnabled"

#define DEFAULT_THEME_MANAGER_ENABLED          true

extern bool gThemeManagerEnabled;
extern std::string gCurrentTheme;

extern CRLayerHandle gAocLayerHandle;
extern CRLayerHandle gContentLayerHandle;