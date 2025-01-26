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
    std::string layerName = "StyleMiiU /vol/content";

    bool oneRedirect = false;
    if(!men.empty()){
        auto res = ContentRedirection_AddFSLayer(&gContentLayerHandle,
                                                 layerName.c_str(),
                                                 men.c_str(),
                                                 FS_LAYER_TYPE_CONTENT_MERGE,
                                                 "/vol/content/Common/Package");
        if (res == CONTENT_REDIRECTION_RESULT_SUCCESS) {
            oneRedirect = true;
        } else {
            DEBUG_FUNCTION_LINE_ERR("Failed to redirect /vol/content to %s", men.c_str());
        }
    }
    if(!men2.empty()){
        auto res = ContentRedirection_AddFSLayer(&gContentLayerHandle,
                                                 layerName.c_str(),
                                                 men2.c_str(),
                                                 FS_LAYER_TYPE_CONTENT_MERGE,
                                                 "/vol/content/Common/Package");
        if (res == CONTENT_REDIRECTION_RESULT_SUCCESS) {
            oneRedirect = true;
        } else {
            DEBUG_FUNCTION_LINE_ERR("Failed to redirect /vol/content to %s", men2.c_str());
        }
    }
    if(!cafeBarista.empty()){
        auto res = ContentRedirection_AddFSLayer(&gContentLayerHandle,
                                                 layerName.c_str(),
                                                 cafeBarista.c_str(),
                                                 FS_LAYER_TYPE_CONTENT_MERGE,
                                                 "/vol/content/Common/Sound/Men");
        if (res == CONTENT_REDIRECTION_RESULT_SUCCESS) {
            oneRedirect = true;
        } else {
            DEBUG_FUNCTION_LINE_ERR("Failed to redirect /vol/content to %s", cafeBarista.c_str());
        }
    }
    return oneRedirect;
}