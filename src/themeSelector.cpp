#include "themeSelector.h"
#include "globals.h"
#include <content_redirection/redirection.h>
#include <notifications/notifications.h>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <malloc.h>
#include <map>
#include <string>
#include <utils/logger.h>
#include <wups/storage.h>

bool ReplaceContentInternal(const std::string &basePath, const std::string &subdir, CRLayerHandle *layerHandle);

bool ReplaceContent(const std::string &basePath, const std::string &modpack) {
    bool contentRes = ReplaceContentInternal(basePath, "content", &gContentLayerHandle);
    return contentRes;
}

bool ReplaceContentInternal(const std::string &basePath, const std::string &subdir, CRLayerHandle *layerHandle) {
    std::string layerName = "ThemeManager /vol/" + subdir;
    std::string fullPath  = basePath + "/" + subdir;
    struct stat st {};

    bool subdirExists = (stat(fullPath.c_str(), &st) == 0);

    if (subdirExists) {
        auto res = ContentRedirection_AddFSLayer(layerHandle,
                                                 layerName.c_str(),
                                                 fullPath.c_str(),
                                                 FS_LAYER_TYPE_CONTENT_MERGE);
        if (res == CONTENT_REDIRECTION_RESULT_SUCCESS) {
            DEBUG_FUNCTION_LINE("Redirect /vol/%s to %s", subdir.c_str(), fullPath.c_str());
        } else {
            DEBUG_FUNCTION_LINE_ERR("Failed to redirect /vol/%s to %s", subdir.c_str(), fullPath.c_str());
            return false;
        }
        return true;
    } else {
        DEBUG_FUNCTION_LINE_WARN("Skip /vol/%s to %s redirection. Dir does not exist", subdir.c_str(), fullPath.c_str());
    }

    std::string filePath = basePath;

    DIR* dir = opendir(filePath.c_str());
    if (!dir) {
        DEBUG_FUNCTION_LINE_ERR("Failed to open basePath directory: %s", filePath.c_str());
        return false;
    }

    bool redirectionSuccess = false;
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string fileName(entry->d_name);
        if (fileName == "." || fileName == "..") {
            continue;
        }

        if (stat(filePath.c_str(), &st) < 0) {
            continue;
        }

        layerName = "ThemeManager /vol/" + subdir;

        std::string virtualPath;
        if (fileName == "cafe_barista_men.bfsar") {
            virtualPath = "/Common/Sound/Men";
        }
        else {
            virtualPath = "/Common/Package";
        }
        layerName = layerName + virtualPath;

        auto res = ContentRedirection_AddFSLayer(layerHandle, 
                                                 layerName.c_str(), 
                                                 filePath.c_str(), 
                                                 FS_LAYER_TYPE_CONTENT_MERGE);
        if (res == CONTENT_REDIRECTION_RESULT_SUCCESS) {
            DEBUG_FUNCTION_LINE("Redirected %s to %s", (subdir + virtualPath).c_str(), filePath.c_str());
            std::string tehREs = "Redirected "+ layerName + " to " + filePath;
            NotificationModule_AddInfoNotificationEx(tehREs.c_str(),
                                 95,
                                 {255, 255, 255, 255},
                                 {32, 32, 160, 255},
                                 nullptr,
                                 nullptr,
                                 true);
            redirectionSuccess = true;
        } else {
            DEBUG_FUNCTION_LINE_ERR("Failed to redirect %s", fileName.c_str());
        }
    }

    closedir(dir);
    const char* result = redirectionSuccess ? "true" : "false";
    NotificationModule_AddInfoNotificationEx(result,
                                             95,
                                             {255, 255, 255, 255},
                                             {32, 32, 160, 255},
                                             nullptr,
                                             nullptr,
                                             true);
    return redirectionSuccess;
}