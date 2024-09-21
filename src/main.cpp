#include "globals.h"
#include "utils/logger.h"
#include <dirent.h>
#include <themeSelector.h>
#include <fs/DirList.h>

#include <content_redirection/redirection.h>

#include <coreinit/title.h>
#include <coreinit/launch.h>
#include <sysapp/title.h>
#include <sysapp/launch.h>

#include <wups.h>
#include <wups/config.h>
#include <wups/config_api.h>
#include <wups/storage.h>
#include <wups/config/WUPSConfigItemBoolean.h>

#include <vector>
#include <map>

WUPS_PLUGIN_NAME("Theme Manager");
WUPS_PLUGIN_DESCRIPTION("A way to easily manage custom themes");
WUPS_PLUGIN_VERSION(VERSION);
WUPS_PLUGIN_AUTHOR("Juanen100");
WUPS_PLUGIN_LICENSE("GPLv3");

WUPS_USE_WUT_DEVOPTAB();
WUPS_USE_STORAGE("theme_manager");

bool need_to_restart = false;
bool is_wiiu_menu = false;

std::vector<std::string> themeNames;
std::string lastChangedTheme;

static void bool_item_callback(ConfigItemBoolean *item, bool newValue) {
    if (!item || !item->identifier) {
        DEBUG_FUNCTION_LINE_WARN("Invalid item or identifier in bool item callback");
        return;
    }
    DEBUG_FUNCTION_LINE_VERBOSE("New value in %s changed: %d", item->identifier, newValue);
    
    if (std::string_view(THEME_MANAGER_ENABLED_STRING) == item->identifier) {
        gThemeManagerEnabled = newValue;
        need_to_restart = true;
    }
    else
    {
        lastChangedTheme = item->identifier;
        gCurrentTheme = lastChangedTheme;
        DEBUG_FUNCTION_LINE("Theme selected: %s", gCurrentTheme.c_str());

        if(lastChangedTheme == gCurrentTheme){
            need_to_restart = true;
        }

        // Disable all other themes
        for (const auto& theme : themeNames) {
            if (theme != lastChangedTheme) {
                bool itemValue = false;
                WUPSStorageError err;
                if ((err = WUPSStorageAPI::Store(theme, itemValue)) != WUPS_STORAGE_ERROR_SUCCESS) {
                    DEBUG_FUNCTION_LINE_WARN("Failed to store value %d to storage item \"%s\": %s (%d)", itemValue, theme.c_str(), WUPSStorageAPI_GetStatusStr(err), err);
                }
            } 
            else 
            {
                bool itemValue = newValue;
                WUPSStorageError err;
                if ((err = WUPSStorageAPI::Store(lastChangedTheme, itemValue)) != WUPS_STORAGE_ERROR_SUCCESS) {
                    DEBUG_FUNCTION_LINE_WARN("Failed to store value %d to storage item \"%s\": %s (%d)", itemValue, theme.c_str(), WUPSStorageAPI_GetStatusStr(err), err);
                }
                if(itemValue){
                    if ((err = WUPSStorageAPI::Store("current", gCurrentTheme)) != WUPS_STORAGE_ERROR_SUCCESS) {
                        DEBUG_FUNCTION_LINE_WARN("Failed to store value %d to storage item \"%s\": %s (%d)", "current", gCurrentTheme.c_str(), WUPSStorageAPI_GetStatusStr(err), err);
                    }
                } else {
                    gCurrentTheme = "";
                    if ((err = WUPSStorageAPI::Store("current", gCurrentTheme)) != WUPS_STORAGE_ERROR_SUCCESS) {
                        DEBUG_FUNCTION_LINE_WARN("Failed to store value %d to storage item \"%s\": %s (%d)", "current", gCurrentTheme.c_str(), WUPSStorageAPI_GetStatusStr(err), err);
                    }
                }
            }
        }
    }

    WUPSStorageError err;
    if(item->identifier != gCurrentTheme) {
        if ((err = WUPSStorageAPI::Store(item->identifier, newValue)) != WUPS_STORAGE_ERROR_SUCCESS) {
            DEBUG_FUNCTION_LINE_WARN("Failed to store value %d to storage item \"%s\": %s (%d)", newValue, item->identifier, WUPSStorageAPI_GetStatusStr(err), err);
        }
    }
}


static WUPSConfigAPICallbackStatus ConfigMenuOpenedCallback(WUPSConfigCategoryHandle rootHandle) {
    uint64_t current_title_id = OSGetTitleID();
    uint64_t wiiu_menu_tid = _SYSGetSystemApplicationTitleId(SYSTEM_APP_ID_WII_U_MENU);

    is_wiiu_menu = (current_title_id == wiiu_menu_tid);
    try {
        WUPSConfigCategory root = WUPSConfigCategory(rootHandle);

        root.add(WUPSConfigItemBoolean::Create(THEME_MANAGER_ENABLED_STRING,
                                               "Enable Theme Manager (Console will be restarted)",
                                               DEFAULT_THEME_MANAGER_ENABLED, gThemeManagerEnabled,
                                               &bool_item_callback));
        
        auto themes = WUPSConfigCategory::Create("Available Themes");

        const char* theme_directory_path = "fs:/vol/external01/wiiu/themes/";
        DIR* dir = opendir(theme_directory_path);
        if (dir != nullptr) {
            struct dirent* entry;
            while ((entry = readdir(dir)) != nullptr) {
                if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                    bool themeEnabled = false;
                    WUPSStorageError err;

                    if ((err = WUPSStorageAPI::Get(entry->d_name, themeEnabled)) != WUPS_STORAGE_ERROR_SUCCESS) {
                        DEBUG_FUNCTION_LINE_WARN("Failed to get storage item \"%s\": %s (%d)", entry->d_name, WUPSStorageAPI_GetStatusStr(err), err);
                        themeEnabled = false;
                        WUPSStorageAPI::Store(entry->d_name, themeEnabled);
                    }

                    themes.add(WUPSConfigItemBoolean::Create(entry->d_name,
                                                              entry->d_name,
                                                              false,
                                                              themeEnabled,
                                                              bool_item_callback));
                    themeNames.push_back(entry->d_name);
                }
            }
            closedir(dir);
        } else {
            DEBUG_FUNCTION_LINE_ERR("Failed to open theme directory: %s\n", theme_directory_path);
            return WUPSCONFIG_API_CALLBACK_RESULT_ERROR;
        }

        root.add(std::move(themes));

    } catch (std::exception &e) {
        DEBUG_FUNCTION_LINE_ERR("Exception: %s\n", e.what());
        return WUPSCONFIG_API_CALLBACK_RESULT_ERROR;
    }
    return WUPSCONFIG_API_CALLBACK_RESULT_SUCCESS;
}

static void ConfigMenuClosedCallback() {
    WUPSStorageError err;
    if ((err = WUPSStorageAPI::SaveStorage()) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to close storage: %s (%d)", WUPSStorageAPI_GetStatusStr(err), err);
    }
    if(need_to_restart && is_wiiu_menu)
    {
        OSForceFullRelaunch();
        SYSLaunchMenu();
        need_to_restart = false;
    }
}

INITIALIZE_PLUGIN() {
    ContentRedirectionStatus error;
    if ((error = ContentRedirection_InitLibrary()) != CONTENT_REDIRECTION_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to init ContentRedirection. Error %s %d", ContentRedirection_GetStatusStr(error), error);
        OSFatal("Failed to init ContentRedirection.");
    }

    WUPSStorageError err;
    if ((err = WUPSStorageAPI::GetOrStoreDefault(THEME_MANAGER_ENABLED_STRING, gThemeManagerEnabled, DEFAULT_THEME_MANAGER_ENABLED)) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to get or create item \"%s\": %s (%d)", THEME_MANAGER_ENABLED_STRING, WUPSStorageAPI_GetStatusStr(err), err);
    }

    const char* theme_directory_path = "fs:/vol/external01/wiiu/themes/";
    DIR* dir = opendir(theme_directory_path);
    if (dir != nullptr) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                bool themeExists = false;
                if ((err = WUPSStorageAPI::Get(entry->d_name, themeExists)) != WUPS_STORAGE_ERROR_SUCCESS) {
                    DEBUG_FUNCTION_LINE_ERR("Failed to get storage item \"%s\": %s (%d)", entry->d_name, WUPSStorageAPI_GetStatusStr(err), err);
                    continue;
                }

                if (!themeExists) {
                    bool defaultValue = false;
                    if ((err = WUPSStorageAPI::Store(entry->d_name, defaultValue)) != WUPS_STORAGE_ERROR_SUCCESS) {
                        DEBUG_FUNCTION_LINE_WARN("Failed to store new theme \"%s\": %s (%d)", entry->d_name, WUPSStorageAPI_GetStatusStr(err), err);
                    } else {
                        themeNames.push_back(entry->d_name);
                    }
                }
            }
        }
        closedir(dir);
    } else {
        DEBUG_FUNCTION_LINE_ERR("Failed to open theme directory: %s\n", theme_directory_path);
    }


    if ((err = WUPSStorageAPI::SaveStorage()) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to save storage: %s (%d)", WUPSStorageAPI_GetStatusStr(err), err);
    }

    WUPSConfigAPIOptionsV1 configOptions = {.name = "Theme Manager"};
    WUPSConfigAPIStatus configErr;
    if ((configErr = WUPSConfigAPI_Init(configOptions, ConfigMenuOpenedCallback, ConfigMenuClosedCallback)) != WUPSCONFIG_API_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to init config api: %s (%d)", WUPSConfigAPI_GetStatusStr(configErr), configErr);
    }

    gContentLayerHandle = 0;
    gAocLayerHandle     = 0;
}

void HandleThemes()
{
    char TitleIDString[17];
    snprintf(TitleIDString, 17, "%016llX", OSGetTitleID());
    std::map<std::string, std::string> themeTitlePath;
    const std::string themeTitleIDPath    = std::string("fs:/vol/external01/wiiu/themes/").append(gCurrentTheme);
    DirList themeTitleDirList(themeTitleIDPath, nullptr, DirList::Dirs);

    themeTitleDirList.SortList();

    for (int index = 0; index < themeTitleDirList.GetFilecount(); index++) {
        std::string curFile = themeTitleDirList.GetFilename(index);

        if (curFile == "." || curFile == "..") {
            continue;
        }

        const std::string &packageName = curFile;
        themeTitlePath[packageName]      = (themeTitleIDPath);
        DEBUG_FUNCTION_LINE_VERBOSE("Found %s  %s", packageName.c_str(), themeTitlePath[packageName].c_str());
    }

    if (themeTitlePath.empty()) {
        return;
    } else{
        ReplaceContent(themeTitlePath.begin()->second, themeTitlePath.begin()->first);
        return;
    }
}


ON_APPLICATION_START() {
    initLogging();
    WUPSStorageError err;
    if ((err = WUPSStorageAPI::Get("current", gCurrentTheme)) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_WARN("Failed to get storage item \"%s\": %s (%d)", "current", WUPSStorageAPI_GetStatusStr(err), err);
    }
    if (gThemeManagerEnabled) {
        HandleThemes();
    } else {
        DEBUG_FUNCTION_LINE("Theme Manager is disabled");
    }
}

ON_APPLICATION_ENDS() {
    if (gContentLayerHandle != 0) {
        ContentRedirection_RemoveFSLayer(gContentLayerHandle);
        gContentLayerHandle = 0;
    }
    if (gAocLayerHandle != 0) {
        ContentRedirection_RemoveFSLayer(gAocLayerHandle);
        gAocLayerHandle = 0;
    }
    deinitLogging();
}