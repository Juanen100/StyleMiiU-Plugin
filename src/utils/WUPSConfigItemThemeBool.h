#pragma once

#include <wups/config.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ConfigItemThemeBool {
    WUPSConfigItemHandle handle;
    const char *identifier;
    bool defaultValue;
    bool valueAtCreation;
    bool value;
    char trueValue[32];
    char falseValue[32];
    void *valueChangedCallback;
} ConfigItemThemeBool;

typedef void (*BooleanThemeValueChangedCallback)(ConfigItemThemeBool *, bool);

WUPSConfigAPIStatus
WUPSConfigItemThemeBool_CreateEx(const char *identifier,
                                 const char *displayName,
                                 bool defaultValue,
                                 bool currentValue,
                                 BooleanThemeValueChangedCallback callback,
                                 const char *trueValue,
                                 const char *falseValue,
                                 WUPSConfigItemHandle *outHandle);


/**
 * @brief Adds a theme-related configuration item to the specified category.
 *
 * This function adds a theme-related configuration item to the given category. The item is displayed with a specified display name.
 * The default value and current value of the item are set to the provided values. A callback function is called whenever
 * the value of the item changes.
 *
 * @param cat The handle of the category to add the item to.
 * @param identifier Optional identifier for the item. Can be NULL.
 * @param displayName The display name of the item.
 * @param defaultValue The default value of the item.
 * @param currentValue The current value of the item.
 * @param callback A callback function that will be called when the config menu closes and the value of the item has been changed.
 * @return True if the item was added successfully, false otherwise.
 */
WUPSConfigAPIStatus
WUPSConfigItemThemeBool_AddToCategory(WUPSConfigCategoryHandle cat,
                                      const char *identifier,
                                      const char *displayName,
                                      bool defaultValue,
                                      bool currentValue,
                                      BooleanThemeValueChangedCallback callback);

/**
 * @brief Adds a theme-related configuration item to the specified category.
 *
 * This function adds a theme-related configuration item to the given category. The item is displayed with a specified display name.
 * The default value and current value of the item are set to the provided values. A callback function is called whenever
 * the value of the item changes.
 *
 * @param cat The handle of the category to add the item to.
 * @param identifier Optional identifier for the item. Can be NULL.
 * @param displayName The display name of the item.
 * @param defaultValue The default value of the item.
 * @param currentValue The current value of the item.
 * @param callback A callback function that will be called when the config menu closes and the value of the item has been changed.
 * @param trueValue The string representation of the true value.
 * @param falseValue The string representation of the false value.
 * @return True if the item was successfully added to the category, false otherwise.
 */
WUPSConfigAPIStatus
WUPSConfigItemThemeBool_AddToCategoryEx(WUPSConfigCategoryHandle cat,
                                        const char *identifier,
                                        const char *displayName,
                                        bool defaultValue,
                                        bool currentValue,
                                        BooleanThemeValueChangedCallback callback,
                                        const char *trueValue,
                                        const char *falseValue);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#include <wups/config/WUPSConfigItem.h>
#include <wups/config_api.h>
#include <optional>
#include <stdexcept>
#include <string>


class WUPSConfigItemThemeBool : public WUPSConfigItem {
public:
    static std::optional<WUPSConfigItemThemeBool> CreateEx(std::optional<std::string> identifier,
                                                           std::string_view displayName,
                                                           bool defaultValue,
                                                           bool currentValue,
                                                           BooleanThemeValueChangedCallback callback,
                                                           std::string_view trueValue,
                                                           std::string_view falseValue,
                                                           WUPSConfigAPIStatus &err) noexcept;

    static WUPSConfigItemThemeBool CreateEx(std::optional<std::string> identifier,
                                            std::string_view displayName,
                                            bool defaultValue,
                                            bool currentValue,
                                            BooleanThemeValueChangedCallback callback,
                                            std::string_view trueValue,
                                            std::string_view falseValue);

    static std::optional<WUPSConfigItemThemeBool> Create(std::optional<std::string> identifier,
                                                         std::string_view displayName,
                                                         bool defaultValue,
                                                         bool currentValue,
                                                         BooleanThemeValueChangedCallback callback,
                                                         WUPSConfigAPIStatus &err) noexcept;

    static WUPSConfigItemThemeBool Create(std::optional<std::string> identifier,
                                          std::string_view displayName,
                                          bool defaultValue,
                                          bool currentValue,
                                          BooleanThemeValueChangedCallback callback);

private:
    explicit WUPSConfigItemThemeBool(WUPSConfigItemHandle itemHandle) : WUPSConfigItem(itemHandle) {
    }
};
#endif
