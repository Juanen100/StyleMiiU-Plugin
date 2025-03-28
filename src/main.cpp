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
#include "utils/ModifiedWUPS/WUPSConfigItemBoolean.h"

#include <vector>
#include <map>
#include <random>
#include <algorithm>
#include <ctime>
#include <chrono>
#include <functional>

WUPS_PLUGIN_NAME("StyleMiiU");
WUPS_PLUGIN_DESCRIPTION("A way to easily load custom themes");
WUPS_PLUGIN_VERSION(VERSION);
WUPS_PLUGIN_AUTHOR("Juanen100");
WUPS_PLUGIN_LICENSE("GPLv3");

WUPS_USE_WUT_DEVOPTAB();
WUPS_USE_STORAGE("style-mii-u");

bool need_to_restart = false;
bool is_wiiu_menu = false;

std::vector<std::string> enabledThemes;
std::string gFavoriteThemes;

bool shuffleEnabled = false;

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

        if (!newValue) {
            enabledThemes.clear();
            std::string lastTheme;
            std::string storedThemes;
            
            if (WUPSStorageAPI::Get("enabledThemes", storedThemes) == WUPS_STORAGE_ERROR_SUCCESS) {
                std::stringstream ss(storedThemes);
                std::string theme;
                
                while (std::getline(ss, theme, '|')) {
                    if (!theme.empty()) {
                        lastTheme = theme;
                    }
                }
            }

            if (!lastTheme.empty()) {
                enabledThemes.push_back(lastTheme);
                if ((err = WUPSStorageAPI::Store("enabledThemes", lastTheme)) != WUPS_STORAGE_ERROR_SUCCESS) {
                    DEBUG_FUNCTION_LINE_WARN("Failed to store enabled theme \"%s\": %s (%d)", lastTheme.c_str(), WUPSStorageAPI_GetStatusStr(err), err);
                }
            }
        } else {
            enabledThemes.clear();
            std::string storedThemes;
            if (WUPSStorageAPI::Get("enabledThemes", storedThemes) == WUPS_STORAGE_ERROR_SUCCESS) {
                std::stringstream ss(storedThemes);
                std::string theme;

                while (std::getline(ss, theme, '|')) {
                    if (!theme.empty()) {
                        enabledThemes.push_back(theme);
                    }
                }
            }
        }
        need_to_restart = true;
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
    std::string storedThemes;
    gShuffleThemes = shuffleEnabled;

    if(gCurrentThemeItem == nullptr){
        gCurrentThemeItem = item;
    }

    if (!gShuffleThemes) {
        enabledThemes.clear();
        enabledThemes.push_back(std::string(gCurrentThemeItem->identifier));
        storedThemes = gCurrentThemeItem->identifier;
        if(enabledThemes[0] != gCurrentTheme){
            need_to_restart = true;
        }
        gCurrentTheme = storedThemes;

        if ((err = WUPSStorageAPI::Store("enabledThemes", storedThemes)) != WUPS_STORAGE_ERROR_SUCCESS) {
            DEBUG_FUNCTION_LINE_WARN("Failed to store enabled theme \"%s\": %s (%d)", gCurrentTheme.c_str(), WUPSStorageAPI_GetStatusStr(err), err);
        }
    } else {
        if (newValue) {
            enabledThemes.push_back(std::string(item->identifier));
        } else {
            enabledThemes.erase(std::remove(enabledThemes.begin(), enabledThemes.end(), item->identifier), enabledThemes.end());
        }

        enabledThemes.erase(std::unique(enabledThemes.begin(), enabledThemes.end()), enabledThemes.end());

        std::stringstream ss;
        for (const auto& theme : enabledThemes) {
            ss << theme << "|";
        }

        std::string storedThemes = ss.str();
        if ((err = WUPSStorageAPI::Store("enabledThemes", storedThemes)) != WUPS_STORAGE_ERROR_SUCCESS) {
            DEBUG_FUNCTION_LINE_WARN("Failed to store enabled themes: %s (%d)", WUPSStorageAPI_GetStatusStr(err), err);
        }
    }
}

static WUPSConfigAPICallbackStatus ConfigMenuOpenedCallback(WUPSConfigCategoryHandle rootHandle) {
    try {
        WUPSConfigCategory root = WUPSConfigCategory(rootHandle);

        root.add(WUPSConfigItemBoolean::Create(THEME_MANAGER_ENABLED_STRING,
                                               "Enable StyleMiiU",
                                               DEFAULT_THEME_MANAGER_ENABLED, gThemeManagerEnabled,
                                               &bool_item_callback));

        root.add(WUPSConfigItemBoolean::Create(SHUFFLE_THEMES_STRING,
                                               "Shuffle Themes",
                                               DEFAULT_SHUFFLE_THEMES, gShuffleThemes,
                                               &bool_item_callback));
        
        auto themes = WUPSConfigCategory::Create("Available Themes");

        DIR* dir = opendir(theme_directory_path);
        if (dir != nullptr) {
            struct dirent* entry;
            while ((entry = readdir(dir)) != nullptr) {
                if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                    std::string themeDirPath = std::string(theme_directory_path) + "/" + entry->d_name;

                    if (isValidThemeDirectory(themeDirPath)) {
                        bool themeEnabled = false;

                        WUPSStorageError err;

                        if (gShuffleThemes) {
                            std::stringstream ss(gFavoriteThemes);
                            std::string theme;

                            while (std::getline(ss, theme, '|')) {
                                if (theme == entry->d_name) {
                                    themeEnabled = true;
                                    continue;
                                }
                            }
                        } else {
                            std::string enabledTheme;
                            if ((err = WUPSStorageAPI::Get("enabledThemes", enabledTheme)) == WUPS_STORAGE_ERROR_SUCCESS) {
                                if (enabledTheme == entry->d_name) {
                                    themeEnabled = true;
                                }
                            }
                        }
                        
                        auto configBool = WUPSConfigItemThemeBool::Create(entry->d_name,
                                                             entry->d_name,
                                                             false,
                                                             themeEnabled,
                                                             theme_bool_item_callback);
                        themes.add(std::move(configBool));

                        if (themeEnabled) {
                            enabledThemes.push_back(entry->d_name);
                        }
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

    std::string blank = "";
    if((err = WUPSStorageAPI::Get("enabledThemes", blank)) != WUPS_STORAGE_ERROR_SUCCESS){
        enabledThemes.push_back("");
        if((err = WUPSStorageAPI::Store("enabledThemes", blank)) != WUPS_STORAGE_ERROR_SUCCESS){
            DEBUG_FUNCTION_LINE_ERR("Failed to get or create item \"%s\": %s (%d)", "enabledThemes", WUPSStorageAPI_GetStatusStr(err), err);
        }
    } 
    else {
        if(blank == "" || blank.empty()) {
            enabledThemes.push_back("");
        }
    }
    
    struct stat st {};
    if(stat(theme_directory_path, &st) != 0){
        theme_directory_path = theme_directory_path_fallback;
    }

    if ((err = WUPSStorageAPI::SaveStorage()) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to save storage: %s (%d)", WUPSStorageAPI_GetStatusStr(err), err);
    }

    WUPSConfigAPIOptionsV1 configOptions = {.name = "StyleMiiU"};
    WUPSConfigAPIStatus configErr;
    if ((configErr = WUPSConfigAPI_Init(configOptions, ConfigMenuOpenedCallback, ConfigMenuClosedCallback)) != WUPSCONFIG_API_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to init config api: %s (%d)", WUPSConfigAPI_GetStatusStr(configErr), configErr);
    }

    gContentLayerHandle = 0;
}

void HandleThemes()
{
    const std::string currentThemePath = std::string(theme_directory_path).append(gCurrentTheme);

    std::string menPackPath;
    std::string men2PackPath;
    std::string cafeBaristaPath;

    std::function<void(const std::string&)> SearchFilesInDirectory = [&](const std::string& directoryPath) {
        DIR* dir = opendir(directoryPath.c_str());
        if (dir == nullptr) {
            return;
        }

        struct dirent* entry;
        struct stat entryInfo;

        while ((entry = readdir(dir)) != nullptr) {
            std::string entryPath = directoryPath + "/" + entry->d_name;

            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            if (stat(entryPath.c_str(), &entryInfo) == 0) {
                if (S_ISDIR(entryInfo.st_mode)) {
                    SearchFilesInDirectory(entryPath);
                } else if (S_ISREG(entryInfo.st_mode)) {
                    if (strcmp(entry->d_name, "Men.pack") == 0 && menPackPath.empty()) {
                        menPackPath = directoryPath + "/" + entry->d_name;
                    } else if (strcmp(entry->d_name, "Men2.pack") == 0 && men2PackPath.empty()) {
                        men2PackPath = directoryPath + "/" + entry->d_name;
                    } else if (strcmp(entry->d_name, "cafe_barista_men.bfsar") == 0 && cafeBaristaPath.empty()) {
                        cafeBaristaPath = directoryPath + "/" + entry->d_name;
                    }
                }
            }
        }

        closedir(dir);
    };
    
    SearchFilesInDirectory(currentThemePath);

    struct stat st {};
    std::string contentPath = currentThemePath + "/content";
    if(stat(contentPath.c_str(), &st) != 0){
        contentPath = currentThemePath + "/Content";
        if(stat(contentPath.c_str(), &st) != 0){
            contentPath = "";
        }
    }

    if (!menPackPath.empty() || !men2PackPath.empty() || !cafeBaristaPath.empty() || !contentPath.empty()) {
        ReplaceContent(menPackPath, men2PackPath, cafeBaristaPath, contentPath);
    }
    return;
}


ON_APPLICATION_START() {
    uint64_t current_title_id = OSGetTitleID();
    uint64_t wiiu_menu_tid = _SYSGetSystemApplicationTitleId(SYSTEM_APP_ID_WII_U_MENU);

    is_wiiu_menu = (current_title_id == wiiu_menu_tid);

    if(!is_wiiu_menu || !gThemeManagerEnabled) return;

    initLogging();
    WUPSStorageError err;

    if (gShuffleThemes) {
        if((err = WUPSStorageAPI::Get("enabledThemes", gFavoriteThemes)) == WUPS_STORAGE_ERROR_SUCCESS){
            std::stringstream ss(gFavoriteThemes);
            std::string theme;
            while (std::getline(ss, theme, '|')) {
                enabledThemes.push_back(theme);
            }

            if (!enabledThemes.empty()) {
                unsigned seed = static_cast<unsigned int>(time(0));
                std::mt19937 rng(seed);
                std::shuffle(enabledThemes.begin(), enabledThemes.end(), rng);
                size_t randomIndex = rng() % enabledThemes.size();
                gCurrentTheme = enabledThemes[randomIndex];

                DEBUG_FUNCTION_LINE("Randomly selected theme: %s", gCurrentTheme.c_str());
            }
        }
    }
    else {
        if ((err = WUPSStorageAPI::Get("enabledThemes", gFavoriteThemes)) == WUPS_STORAGE_ERROR_SUCCESS) {
            std::stringstream ss(gFavoriteThemes);
            std::string theme;

            if(gFavoriteThemes == "")
                return;
            
            while (std::getline(ss, theme, '|')) {
                enabledThemes.push_back(theme);
            }

            gCurrentTheme = enabledThemes[0];
        }
    }

    HandleThemes();
}

ON_APPLICATION_ENDS() {
    if (gContentLayerHandle != 0) {
        ContentRedirection_RemoveFSLayer(gContentLayerHandle);
        gContentLayerHandle = 0;
    }
    deinitLogging();
}