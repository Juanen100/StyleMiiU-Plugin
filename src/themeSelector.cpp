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

bool ReplaceContent(const std::string men, const std::string men2, const std::string cafeBarista, const std::string contentExtraPath) {
    std::string layerName = "StyleMiiU /vol/content";

    bool oneRedirect = false;
    if(!men.empty()){
        auto res = ContentRedirection_AddFSLayerEx(&gContentLayerHandle,
                                                 layerName.c_str(),
                                                 "/vol/content/Common/Package/Men.pack",
                                                 men.c_str(),
                                                 FS_LAYER_TYPE_EX_REPLACE_FILE);
        if (res == CONTENT_REDIRECTION_RESULT_SUCCESS) {
            oneRedirect = true;
        } else {
            DEBUG_FUNCTION_LINE_ERR("Failed to redirect /vol/content to %s", men.c_str());
        }
    }
    if(!men2.empty()){
        auto res = ContentRedirection_AddFSLayerEx(&gContentLayerHandle,
                                                 layerName.c_str(),
                                                 "/vol/content/Common/Package/Men2.pack",
                                                 men2.c_str(),
                                                 FS_LAYER_TYPE_EX_REPLACE_FILE);
        if (res == CONTENT_REDIRECTION_RESULT_SUCCESS) {
            oneRedirect = true;
        } else {
            DEBUG_FUNCTION_LINE_ERR("Failed to redirect /vol/content to %s", men2.c_str());
        }
    }
    if(!cafeBarista.empty()){
        auto res = ContentRedirection_AddFSLayerEx(&gContentLayerHandle,
                                                 layerName.c_str(),
                                                 "/vol/content/Common/Sound/Men/cafe_barista_men.bfsar",
                                                 cafeBarista.c_str(),
                                                 FS_LAYER_TYPE_EX_REPLACE_FILE);
        if (res == CONTENT_REDIRECTION_RESULT_SUCCESS) {
            oneRedirect = true;
        } else {
            DEBUG_FUNCTION_LINE_ERR("Failed to redirect /vol/content to %s", cafeBarista.c_str());
        }
    }
    if(!contentExtraPath.empty()) {
        auto res = ContentRedirection_AddFSLayerEx(&gContentLayerHandle,
                                                    layerName.c_str(),
                                                    "/vol/content",
                                                    contentExtraPath.c_str(),
                                                    FS_LAYER_TYPE_EX_MERGE_DIRECTORY);

        if (res == CONTENT_REDIRECTION_RESULT_SUCCESS) {
            oneRedirect = true;
        } else {
            DEBUG_FUNCTION_LINE_ERR("Failed to redirect /vol/content to %s", cafeBarista.c_str());
        }
    }
    return oneRedirect;
}