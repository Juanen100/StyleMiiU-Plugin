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
#include "utils/WUPSConfigItemThemeBool.h"
#include <wups/config/WUPSConfigItemBoolean.h>

#include <vector>
#include <map>
#include <random>
#include <algorithm>
#include <ctime>
#include <chrono>

WUPS_PLUGIN_NAME("Theme Manager");
WUPS_PLUGIN_DESCRIPTION("A way to easily manage custom themes");
WUPS_PLUGIN_VERSION(VERSION);
WUPS_PLUGIN_AUTHOR("Juanen100");
WUPS_PLUGIN_LICENSE("GPLv3");

WUPS_USE_WUT_DEVOPTAB();
WUPS_USE_STORAGE("theme_manager");

bool need_to_restart = false;
bool is_wiiu_menu = false;
bool theme_item_pressed = false;

std::vector<std::string> themeNames;
std::string lastChangedTheme;

std::vector<std::string> enabledThemes;

bool isValidThemeDirectory(const std::string& path) {
    DIR* dir = opendir(path.c_str());
    if (dir == nullptr) {
        return false;
    }

    struct dirent* entry;
    struct stat entryInfo;
    bool validTheme = false;

    while ((entry = readdir(dir)) != nullptr) {
        std::string entryPath = path + "/" + entry->d_name;

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        if (stat(entryPath.c_str(), &entryInfo) == 0) {
            if (S_ISDIR(entryInfo.st_mode)) {
                if (isValidThemeDirectory(entryPath)) {
                    validTheme = true;
                    break;
                }
            } else if (S_ISREG(entryInfo.st_mode)) {
                if (strcmp(entry->d_name, "Men.pack") == 0 || 
                    strcmp(entry->d_name, "Men2.pack") == 0 || 
                    strcmp(entry->d_name, "cafe_barista_men.bfsar") == 0) {
                    validTheme = true;
                    break;
                }
            }
        }
    }

    closedir(dir);
    return validTheme;
}

static void bool_item_callback(ConfigItemBoolean *item, bool newValue) {
    if (!item || !item->identifier) {
        DEBUG_FUNCTION_LINE_WARN("Invalid item or identifier in bool item callback");
        return;
    }
    DEBUG_FUNCTION_LINE_VERBOSE("New value in %s changed: %d", item->identifier, newValue);
    
    WUPSStorageError err;
    if (std::string_view(THEME_MANAGER_ENABLED_STRING) == item->identifier) {
        gThemeManagerEnabled = newValue;
        need_to_restart = true;
    }
    else if (std::string_view(SHUFFLE_THEMES_STRING) == item->identifier)
    {
        gShuffleThemes = newValue;
        need_to_restart = true;
        if(!newValue){ //Disable all themes
            enabledThemes.clear();
            for (const auto& theme : themeNames) {
                bool itemValue = (theme == lastChangedTheme);
                if ((err = WUPSStorageAPI::Store(theme, itemValue)) != WUPS_STORAGE_ERROR_SUCCESS) {
                    DEBUG_FUNCTION_LINE_WARN("Failed to store value %d to storage item \"%s\": %s (%d)", itemValue, theme.c_str(), WUPSStorageAPI_GetStatusStr(err), err);
                }

                if (itemValue) {
                    gCurrentTheme = theme;
                    DEBUG_FUNCTION_LINE("Default theme set: %s", gCurrentTheme.c_str());
                    if ((err = WUPSStorageAPI::Store("current", gCurrentTheme)) != WUPS_STORAGE_ERROR_SUCCESS) {
                        DEBUG_FUNCTION_LINE_WARN("Failed to store value \"%s\" for storage item \"%s\": %s (%d)", gCurrentTheme.c_str(), "current", WUPSStorageAPI_GetStatusStr(err), err);
                    }
                }
            }
        }
        else {
            enabledThemes.clear();

            for (const auto& theme : themeNames) {
                bool themeEnabled = false;

                if (WUPSStorageAPI::Get(theme, themeEnabled) == WUPS_STORAGE_ERROR_SUCCESS) {
                    if (themeEnabled) {
                        enabledThemes.push_back(theme);
                    }
                } else {
                    DEBUG_FUNCTION_LINE_WARN("Failed to get theme status for \"%s\"", theme.c_str());
                }
            }

            for (const auto& theme : themeNames) {
                if (std::find(enabledThemes.begin(), enabledThemes.end(), theme) == enabledThemes.end()) {
                    bool itemValue = true;
                    if ((err = WUPSStorageAPI::Store(theme, itemValue)) != WUPS_STORAGE_ERROR_SUCCESS) {
                        DEBUG_FUNCTION_LINE_WARN("Failed to store value %d to storage item \"%s\": %s (%d)", itemValue, theme.c_str(), WUPSStorageAPI_GetStatusStr(err), err);
                    }
                    enabledThemes.push_back(theme);
                }
            }

            gCurrentTheme = !enabledThemes.empty() ? enabledThemes.front() : "";
            if ((err = WUPSStorageAPI::Store("current", gCurrentTheme)) != WUPS_STORAGE_ERROR_SUCCESS) {
                DEBUG_FUNCTION_LINE_WARN("Failed to store value \"%s\" for storage item \"%s\": %s (%d)", gCurrentTheme.c_str(), "current", WUPSStorageAPI_GetStatusStr(err), err);
            }
        }
    }

    if ((err = WUPSStorageAPI::Store(THEME_MANAGER_ENABLED_STRING, gThemeManagerEnabled)) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to get or create item \"%s\": %s (%d)", THEME_MANAGER_ENABLED_STRING, WUPSStorageAPI_GetStatusStr(err), err);
    }

    if ((err = WUPSStorageAPI::Store(SHUFFLE_THEMES_STRING, gShuffleThemes)) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to get or create item \"%s\": %s (%d)", SHUFFLE_THEMES_STRING, WUPSStorageAPI_GetStatusStr(err), err);
    }
}

static void theme_bool_item_callback(ConfigItemThemeBool *item, bool newValue) {
    if (!item || !item->identifier) {
        DEBUG_FUNCTION_LINE_WARN("Invalid item or identifier in bool item callback");
        return;
    }
    DEBUG_FUNCTION_LINE_VERBOSE("New value in %s changed: %d", item->identifier, newValue);
    
    WUPSStorageError err;
    enabledThemes.clear();

    for (const auto& theme : themeNames) {
        bool themeEnabled = false;

        if (WUPSStorageAPI::Get(theme, themeEnabled) == WUPS_STORAGE_ERROR_SUCCESS) {
            if (themeEnabled) {
                enabledThemes.push_back(theme);
            }
        } else {
            DEBUG_FUNCTION_LINE_WARN("Failed to get theme status for \"%s\"", theme.c_str());
        }
    }

    if (newValue) {
        enabledThemes.push_back(std::string(item->identifier));
    }

    lastChangedTheme = item->identifier;
    gCurrentTheme = lastChangedTheme;
    DEBUG_FUNCTION_LINE("Theme selected: %s", gCurrentTheme.c_str());

    if(lastChangedTheme == gCurrentTheme){
        need_to_restart = true;
    }

    for (const auto& theme : themeNames) {
        if (!gShuffleThemes) {
            bool itemValue = (theme == lastChangedTheme) ? newValue : false;
            if ((err = WUPSStorageAPI::Store(theme, itemValue)) != WUPS_STORAGE_ERROR_SUCCESS) {
                DEBUG_FUNCTION_LINE_WARN("Failed to store value %d to storage item \"%s\": %s (%d)", itemValue, theme.c_str(), WUPSStorageAPI_GetStatusStr(err), err);
            }
            if (!itemValue && theme == gCurrentTheme) {
                gCurrentTheme = "";
            }
        } else {
            if (theme == lastChangedTheme) {
                bool itemValue = newValue;
                if ((err = WUPSStorageAPI::Store(theme, itemValue)) != WUPS_STORAGE_ERROR_SUCCESS) {
                    DEBUG_FUNCTION_LINE_WARN("Failed to store value %d to storage item \"%s\": %s (%d)", itemValue, theme.c_str(), WUPSStorageAPI_GetStatusStr(err), err);
                }
                if (itemValue) {
                    if (std::find(enabledThemes.begin(), enabledThemes.end(), theme) == enabledThemes.end()) {
                        enabledThemes.push_back(theme);
                    }
                } else {
                    enabledThemes.erase(std::remove(enabledThemes.begin(), enabledThemes.end(), theme), enabledThemes.end());
                }
            }
        }
    }

    if (lastChangedTheme == gCurrentTheme) {
        if ((err = WUPSStorageAPI::Store("current", gCurrentTheme)) != WUPS_STORAGE_ERROR_SUCCESS) {
            DEBUG_FUNCTION_LINE_WARN("Failed to store value %d to storage item \"%s\": %s (%d)", "current", gCurrentTheme.c_str(), WUPSStorageAPI_GetStatusStr(err), err);
        }
    }

    if(item->identifier != gCurrentTheme) {
        if ((err = WUPSStorageAPI::Store(item->identifier, newValue)) != WUPS_STORAGE_ERROR_SUCCESS) {
            DEBUG_FUNCTION_LINE_WARN("Failed to store value %d to storage item \"%s\": %s (%d)", newValue, item->identifier, WUPSStorageAPI_GetStatusStr(err), err);
        }
    }
}

static WUPSConfigAPICallbackStatus ConfigMenuOpenedCallback(WUPSConfigCategoryHandle rootHandle) {
    try {
        WUPSConfigCategory root = WUPSConfigCategory(rootHandle);

        root.add(WUPSConfigItemBoolean::Create(THEME_MANAGER_ENABLED_STRING,
                                               "Enable Theme Manager",
                                               DEFAULT_THEME_MANAGER_ENABLED, gThemeManagerEnabled,
                                               &bool_item_callback));

        root.add(WUPSConfigItemBoolean::Create(SHUFFLE_THEMES_STRING,
                                               "Shuffle Themes",
                                               DEFAULT_SHUFFLE_THEMES, gShuffleThemes,
                                               &bool_item_callback));
        
        auto themes = WUPSConfigCategory::Create("Available Themes");

        DIR* checkDir = opendir(theme_directory_path);
        if(!checkDir){
            theme_directory_path = theme_directory_path_fallback;
        }
        
        DIR* dir = opendir(theme_directory_path);
        if (dir != nullptr) {
            struct dirent* entry;
            while ((entry = readdir(dir)) != nullptr) {
                if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                    std::string themeDirPath = std::string(theme_directory_path) + "/" + entry->d_name;

                    if (isValidThemeDirectory(themeDirPath)) {
                        bool themeEnabled = false;

                        WUPSStorageError err;
                        if ((err = WUPSStorageAPI::Get(entry->d_name, themeEnabled)) != WUPS_STORAGE_ERROR_SUCCESS) {
                            DEBUG_FUNCTION_LINE_WARN("Failed to get storage item \"%s\": %s (%d)", entry->d_name, WUPSStorageAPI_GetStatusStr(err), err);
                            themeEnabled = false;
                            WUPSStorageAPI::Store(entry->d_name, themeEnabled);
                        }

                        themes.add(WUPSConfigItemThemeBool::Create(entry->d_name,
                                                             entry->d_name,
                                                             false,
                                                             themeEnabled,
                                                             theme_bool_item_callback));

                        if (themeEnabled && gShuffleThemes) {
                            enabledThemes.push_back(entry->d_name);
                        }

                        themeNames.push_back(entry->d_name);
                    }
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
    NotificationModule_InitLibrary();
    ContentRedirectionStatus error;
    if ((error = ContentRedirection_InitLibrary()) != CONTENT_REDIRECTION_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to init ContentRedirection. Error %s %d", ContentRedirection_GetStatusStr(error), error);
        OSFatal("Failed to init ContentRedirection.");
    }

    WUPSStorageError err;
    if ((err = WUPSStorageAPI::GetOrStoreDefault(THEME_MANAGER_ENABLED_STRING, gThemeManagerEnabled, DEFAULT_THEME_MANAGER_ENABLED)) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to get or create item \"%s\": %s (%d)", THEME_MANAGER_ENABLED_STRING, WUPSStorageAPI_GetStatusStr(err), err);
    }

    if ((err = WUPSStorageAPI::GetOrStoreDefault(SHUFFLE_THEMES_STRING, gShuffleThemes, DEFAULT_SHUFFLE_THEMES)) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to get or create item \"%s\": %s (%d)", SHUFFLE_THEMES_STRING, WUPSStorageAPI_GetStatusStr(err), err);
    }

    DIR* checkDir = opendir(theme_directory_path);
    if(!checkDir){
        theme_directory_path = theme_directory_path_fallback;
    }

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

                if(themeExists && gShuffleThemes){
                    enabledThemes.push_back(entry->d_name);
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
}

void HandleThemesInDirectory(const std::string& directoryPath, std::map<std::string, std::string>& themeTitlePath)
{
    DirList dirList(directoryPath, nullptr, DirList::Dirs);
    dirList.SortList();

    for (int index = 0; index < dirList.GetFilecount(); index++) {
        std::string curFile = dirList.GetFilename(index);

        if (curFile == "." || curFile == "..") {
            continue;
        }

        if (curFile == "content") {
            themeTitlePath[curFile] = std::string(theme_directory_path).append(gCurrentTheme);
            return;
        }

        std::string subDirPath = directoryPath + "/" + curFile;
        HandleThemesInDirectory(subDirPath, themeTitlePath);
    }

    DirList fileList(directoryPath, nullptr, DirList::Files);
    fileList.SortList();

    for (int index = 0; index < fileList.GetFilecount(); index++) {
        std::string curFile = fileList.GetFilename(index);

        if (curFile == "Men.pack" || curFile == "Men2.pack" || curFile == "cafe_barista_men.bfsar") {
            themeTitlePath[curFile] = std::string(theme_directory_path).append(gCurrentTheme);
            DEBUG_FUNCTION_LINE_VERBOSE("Found %s at %s", curFile.c_str(), std::string(theme_directory_path).append(gCurrentTheme).c_str());
        }
    }
}

void HandleThemes()
{
    std::map<std::string, std::string> themeTitlePath;
    const std::string themeTitleIDPath = std::string(theme_directory_path).append(gCurrentTheme);

    HandleThemesInDirectory(themeTitleIDPath, themeTitlePath);

    if (themeTitlePath.empty()) {
        return;
    } else {
        ReplaceContent(themeTitlePath.begin()->second, themeTitlePath.begin()->first);
        return;
    }
}


ON_APPLICATION_START() {
    uint64_t current_title_id = OSGetTitleID();
    uint64_t wiiu_menu_tid = _SYSGetSystemApplicationTitleId(SYSTEM_APP_ID_WII_U_MENU);

    is_wiiu_menu = (current_title_id == wiiu_menu_tid);

    if(!is_wiiu_menu) return;
    initLogging();
    WUPSStorageError err;

    if ((err = WUPSStorageAPI::Get("current", gCurrentTheme)) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_WARN("Failed to get storage item \"%s\": %s (%d)", "current", WUPSStorageAPI_GetStatusStr(err), err);
        gCurrentTheme = "";
    }
    
    if (gThemeManagerEnabled) {
        if (gShuffleThemes && !enabledThemes.empty()) {
            unsigned seed = static_cast<unsigned int>(time(0));
            std::mt19937 rng(seed);
            std::shuffle(enabledThemes.begin(), enabledThemes.end(), rng);

            size_t randomIndex = rng() % enabledThemes.size();
            gCurrentTheme = enabledThemes[randomIndex];

            DEBUG_FUNCTION_LINE("Randomly selected theme: %s", gCurrentTheme.c_str());

            if ((err = WUPSStorageAPI::Store("current", gCurrentTheme)) != WUPS_STORAGE_ERROR_SUCCESS) {
                DEBUG_FUNCTION_LINE_WARN("Failed to store value \"%s\" for storage item \"%s\": %s (%d)", gCurrentTheme.c_str(), "current", WUPSStorageAPI_GetStatusStr(err), err);
            }
        }
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
    NotificationModule_DeInitLibrary();
    deinitLogging();
}