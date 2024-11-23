#include "utils/WUPSConfigItemThemeBool.h"
#include <wups/config_api.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <wups/config.h>
#include <wups/storage.h>

#include "globals.h"

static void WUPSConfigItemThemeBool_onCloseCallback(void *context) {
    auto *item = (ConfigItemThemeBool *) context;
    if (item->valueAtCreation != item->value && item->valueChangedCallback != nullptr) {
        ((BooleanThemeValueChangedCallback) (item->valueChangedCallback))(item, item->value);
    }
}

static inline void setTheme(ConfigItemThemeBool *item) {
    if (gCurrentThemeItem != nullptr && gCurrentThemeItem != item && !shuffleEnabled) {
        gCurrentThemeItem->value = false;
    }
    enabledThemes[0] = item->identifier;

    item->value = !item->value;

    gCurrentThemeItem = item;
}

static void WUPSConfigItemThemeBool_onInput(void *context, WUPSConfigSimplePadData input) {
    auto *item = (ConfigItemThemeBool *) context;
    if ((input.buttons_d & WUPS_CONFIG_BUTTON_A) == WUPS_CONFIG_BUTTON_A) {
        setTheme(item);
    }
}

static int32_t WUPSConfigItemThemeBool_getCurrentValueDisplay(void *context, char *out_buf, int32_t out_size) {
    auto *item = (ConfigItemThemeBool *) context;
    if (!item->value) {
        snprintf(out_buf, out_size, "  ");
    } else {
        if (shuffleEnabled) {
            snprintf(out_buf, out_size, "  Favorite");
        } else {
            if (!enabledThemes.empty() && enabledThemes[0] == item->identifier) {
                snprintf(out_buf, out_size, "  Current Theme");
            } else {
                item->value = false;
                snprintf(out_buf, out_size, "  ");
            }
        }
    }
    
    return 0;
}

static int32_t WUPSConfigItemThemeBool_getCurrentValueSelectedDisplay(void *context, char *out_buf, int32_t out_size) {
    auto *item = (ConfigItemThemeBool *) context;
    if (!item->value) {
        snprintf(out_buf, out_size, shuffleEnabled 
        ? "  Press \ue000 to add to favorites" 
        : "  Press \ue000 to set as Current Theme");
    } else {
        if(shuffleEnabled)
            snprintf(out_buf, out_size, "  Press \ue000 to remove from favorites");
        else {
            if (!enabledThemes.empty() && enabledThemes[0] == item->identifier) {
                snprintf(out_buf, out_size, "  Press \ue000 to disable Current Theme");
            } else {
                snprintf(out_buf, out_size, "  Press \ue000 to set as Current Theme");
            }
        }
    }

    return 0;
}

static void WUPSConfigItemThemeBool_restoreDefault(void *context) {
    auto *item  = (ConfigItemThemeBool *) context;
    item->value = item->defaultValue;
}

static void WUPSConfigItemThemeBool_Cleanup(ConfigItemThemeBool *item) {
    if (!item) {
        return;
    }
    free((void *) item->identifier);
    free(item);
}

static void WUPSConfigItemThemeBool_onDelete(void *context) {
    WUPSConfigItemThemeBool_Cleanup((ConfigItemThemeBool *) context);
}

extern "C" WUPSConfigAPIStatus
WUPSConfigItemThemeBool_CreateEx(const char *identifier,
                                 const char *displayName,
                                 bool defaultValue,
                                 bool currentValue,
                                 BooleanThemeValueChangedCallback callback,
                                 const char *trueValue,
                                 const char *falseValue,
                                 WUPSConfigItemHandle *outHandle) {
    if (outHandle == nullptr) {
        return WUPSCONFIG_API_RESULT_INVALID_ARGUMENT;
    }
    auto *item = (ConfigItemThemeBool *) malloc(sizeof(ConfigItemThemeBool));
    if (item == nullptr) {
        return WUPSCONFIG_API_RESULT_OUT_OF_MEMORY;
    }
    
    if (identifier != nullptr) {
        item->identifier = strdup(identifier);
    } else {
        item->identifier = nullptr;
    }

    item->defaultValue         = defaultValue;
    item->value                = currentValue;
    item->valueAtCreation      = currentValue;
    item->valueChangedCallback = (void *) callback;
    snprintf(item->trueValue, sizeof(item->trueValue), "%s", trueValue);
    snprintf(item->falseValue, sizeof(item->falseValue), "%s", falseValue);

    WUPSStorageAPI::Get(SHUFFLE_THEMES_STRING, shuffleEnabled);

    WUPSConfigAPIItemCallbacksV2 callbacks = {
            .getCurrentValueDisplay         = &WUPSConfigItemThemeBool_getCurrentValueDisplay,
            .getCurrentValueSelectedDisplay = &WUPSConfigItemThemeBool_getCurrentValueSelectedDisplay,
            .onSelected                     = nullptr,
            .restoreDefault                 = &WUPSConfigItemThemeBool_restoreDefault,
            .isMovementAllowed              = nullptr,
            .onCloseCallback                = &WUPSConfigItemThemeBool_onCloseCallback,
            .onInput                        = &WUPSConfigItemThemeBool_onInput,
            .onInputEx                      = nullptr,
            .onDelete                       = &WUPSConfigItemThemeBool_onDelete};

    WUPSConfigAPIItemOptionsV2 options = {
            .displayName = displayName,
            .context     = item,
            .callbacks   = callbacks,
    };

    if(currentValue == true && !shuffleEnabled) {
        gCurrentThemeItem = item;
    }

    WUPSConfigAPIStatus err;
    if ((err = WUPSConfigAPI_Item_Create(options, &item->handle)) != WUPSCONFIG_API_RESULT_SUCCESS) {
        WUPSConfigItemThemeBool_Cleanup(item);
        return err;
    }

    *outHandle = item->handle;
    return WUPSCONFIG_API_RESULT_SUCCESS;
}

extern "C" WUPSConfigAPIStatus
WUPSConfigItemThemeBool_AddToCategoryEx(WUPSConfigCategoryHandle cat,
                                        const char *identifier,
                                        const char *displayName,
                                        bool defaultValue,
                                        bool currentValue,
                                        BooleanThemeValueChangedCallback callback,
                                        const char *trueValue,
                                        const char *falseValue) {
    WUPSConfigItemHandle itemHandle;
    WUPSConfigAPIStatus res;
    if ((res = WUPSConfigItemThemeBool_CreateEx(identifier,
                                                displayName,
                                                defaultValue, currentValue,
                                                callback,
                                                trueValue, falseValue,
                                                &itemHandle)) != WUPSCONFIG_API_RESULT_SUCCESS) {
        return res;
    }

    if ((res = WUPSConfigAPI_Category_AddItem(cat, itemHandle)) != WUPSCONFIG_API_RESULT_SUCCESS) {

        WUPSConfigAPI_Item_Destroy(itemHandle);
        return res;
    }
    return WUPSCONFIG_API_RESULT_SUCCESS;
}

extern "C" WUPSConfigAPIStatus
WUPSConfigItemThemeBool_AddToCategory(WUPSConfigCategoryHandle cat,
                                      const char *identifier,
                                      const char *displayName,
                                      bool defaultValue,
                                      bool currentValue,
                                      BooleanThemeValueChangedCallback callback) {
    return WUPSConfigItemThemeBool_AddToCategoryEx(cat, identifier, displayName, defaultValue, currentValue, callback, "true", "false");
}
