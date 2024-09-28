#include "WUPSConfigItemThemeBool.h" 

std::optional<WUPSConfigItemThemeBool> WUPSConfigItemThemeBool::CreateEx(std::optional<std::string> identifier,
                                                                         std::string_view displayName,
                                                                         bool defaultValue, bool currentValue,
                                                                         BooleanThemeValueChangedCallback callback,
                                                                         std::string_view trueValue, std::string_view falseValue,
                                                                         WUPSConfigAPIStatus &err) noexcept {
    WUPSConfigItemHandle itemHandle;
    if ((err = WUPSConfigItemThemeBool_CreateEx(identifier ? identifier->data() : nullptr,
                                                displayName.data(),
                                                defaultValue, currentValue,
                                                callback,
                                                trueValue.data(), falseValue.data(),
                                                &itemHandle)) != WUPSCONFIG_API_RESULT_SUCCESS) {
        return std::nullopt;
    }
    return WUPSConfigItemThemeBool(itemHandle);
}

WUPSConfigItemThemeBool WUPSConfigItemThemeBool::CreateEx(std::optional<std::string> identifier,
                                                          std::string_view displayName,
                                                          bool defaultValue, bool currentValue,
                                                          BooleanThemeValueChangedCallback callback,
                                                          std::string_view trueValue, std::string_view falseValue) {
    WUPSConfigAPIStatus err;
    auto result = CreateEx(std::move(identifier), displayName, defaultValue, currentValue, callback, trueValue, falseValue, err);
    if (!result) {
        throw std::runtime_error(std::string("Failed to create WUPSConfigItemThemeBool: ").append(WUPSConfigAPI_GetStatusStr(err)));
    }
    return std::move(*result);
}

std::optional<WUPSConfigItemThemeBool> WUPSConfigItemThemeBool::Create(std::optional<std::string> identifier,
                                                                       std::string_view displayName,
                                                                       bool defaultValue, bool currentValue,
                                                                       BooleanThemeValueChangedCallback callback,
                                                                       WUPSConfigAPIStatus &err) noexcept {
    return CreateEx(std::move(identifier), displayName, defaultValue, currentValue, callback, "Press \ue000 to set as default theme", "Set as default theme", err);
}

WUPSConfigItemThemeBool WUPSConfigItemThemeBool::Create(std::optional<std::string> identifier,
                                                        std::string_view displayName,
                                                        bool defaultValue, bool currentValue,
                                                        BooleanThemeValueChangedCallback callback) {
    WUPSConfigAPIStatus err = WUPSCONFIG_API_RESULT_UNKNOWN_ERROR;
    auto res                = Create(std::move(identifier), displayName, defaultValue, currentValue, callback, err);
    if (!res) {
        throw std::runtime_error(std::string("Failed to create WUPSConfigItemThemeBool: ").append(WUPSConfigAPI_GetStatusStr(err)));
    }
    return std::move(*res);
}
