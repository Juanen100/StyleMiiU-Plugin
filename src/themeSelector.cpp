#include "themeSelector.h"
#include "globals.h"
#include <content_redirection/redirection.h>
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
    bool aocRes     = ReplaceContentInternal(basePath, "aoc", &gAocLayerHandle);

    if (!contentRes && !aocRes) {
        return false;
    }
    return true;
}

bool ReplaceContentInternal(const std::string &basePath, const std::string &subdir, CRLayerHandle *layerHandle) {
    std::string layerName = "ThemeManager /vol/" + subdir;
    std::string fullPath  = basePath + "/" + subdir;
    struct stat st {};
    if (stat(fullPath.c_str(), &st) < 0) {
        DEBUG_FUNCTION_LINE_WARN("Skip /vol/%s to %s redirection. Dir does not exist", subdir.c_str(), fullPath.c_str());
        return false;
    }

    auto res = ContentRedirection_AddFSLayer(layerHandle,
                                             layerName.c_str(),
                                             fullPath.c_str(),
                                             subdir == "aoc" ? FS_LAYER_TYPE_AOC_MERGE : FS_LAYER_TYPE_CONTENT_MERGE);
    if (res == CONTENT_REDIRECTION_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE("Redirect /vol/%s to %s", subdir.c_str(), fullPath.c_str());
    } else {
        DEBUG_FUNCTION_LINE_ERR("Failed to redirect /vol/%s to %s", subdir.c_str(), fullPath.c_str());
        return false;
    }
    return true;
}