#include "themeSelector.h"
#include "globals.h"
#include <theme_redirection/redirection.h>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <malloc.h>
#include <map>
#include <string>
#include <utils/logger.h>
#include <wups/storage.h>

bool ReplaceContent(const std::string men, const std::string men2, const std::string cafeBarista) {
    std::string layerName = "StyleMiiU /vol/" + subdir;

    bool oneRedirect = false;
    if(!_menPackPath.empty()){
        auto res = ContentRedirection_AddFSLayer(&gContentLayerHandle,
                                                 layerName.c_str(),
                                                 _menPackPath.c_str(),
                                                 FS_LAYER_TYPE_CONTENT_MERGE,
                                                 "/vol/content/Common/Package");
        if (res == CONTENT_REDIRECTION_RESULT_SUCCESS) {
            DEBUG_FUNCTION_LINE("Redirect /vol/%s to %s", subdir.c_str(), _menPackPath.c_str());
            oneRedirect = true;
        } else {
            DEBUG_FUNCTION_LINE_ERR("Failed to redirect /vol/%s to %s", subdir.c_str(), _menPackPath.c_str());
        }
    }
    if(!_men2PackPath.empty()){
        auto res = ContentRedirection_AddFSLayer(&gContentLayerHandle,
                                                 layerName.c_str(),
                                                 _men2PackPath.c_str(),
                                                 FS_LAYER_TYPE_CONTENT_MERGE,
                                                 "/vol/content/Common/Package");
        if (res == CONTENT_REDIRECTION_RESULT_SUCCESS) {
            DEBUG_FUNCTION_LINE("Redirect /vol/%s to %s", subdir.c_str(), _men2PackPath.c_str());
            oneRedirect = true;
        } else {
            DEBUG_FUNCTION_LINE_ERR("Failed to redirect /vol/%s to %s", subdir.c_str(), _men2PackPath.c_str());
        }
    }
    if(!_cafeBaristaMenPath.empty()){
        auto res = ContentRedirection_AddFSLayer(&gContentLayerHandle,
                                                 layerName.c_str(),
                                                 _cafeBaristaMenPath.c_str(),
                                                 FS_LAYER_TYPE_CONTENT_MERGE,
                                                 "/vol/content/Common/Sound/Men");
        if (res == CONTENT_REDIRECTION_RESULT_SUCCESS) {
            DEBUG_FUNCTION_LINE("Redirect /vol/%s to %s", subdir.c_str(), _cafeBaristaMenPath.c_str());
            oneRedirect = true;
        } else {
            DEBUG_FUNCTION_LINE_ERR("Failed to redirect /vol/%s to %s", subdir.c_str(), _cafeBaristaMenPath.c_str());
        }
    }
    return oneRedirect;
}