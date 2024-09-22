#pragma once
#include <content_redirection/redirection.h>
#include <string>

#define VERSION                                "v1.0.1"
#define THEME_MANAGER_ENABLED_STRING           "themeManagerEnabled"
#define SHUFFLE_THEMES_STRING                  "suffleThemes"

#define DEFAULT_THEME_MANAGER_ENABLED          true
#define DEFAULT_SHUFFLE_THEMES                 false

extern bool gThemeManagerEnabled;
extern bool gShuffleThemes;
extern std::string gCurrentTheme;
extern const char* theme_directory_path;

extern CRLayerHandle gAocLayerHandle;
extern CRLayerHandle gContentLayerHandle;