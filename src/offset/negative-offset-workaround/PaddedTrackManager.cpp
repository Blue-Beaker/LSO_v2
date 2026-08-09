#include "PaddedTrackManager.hpp"
#include "CacheStorage.hpp"

PaddedTrackManager PaddedTrackManager::get(){
    static PaddedTrackManager m_instance;
    return m_instance;
}

PaddedResult PaddedTrackManager::getPaddedResult(int totalOffset, gd::string path){
    return PaddedTrackManager::getPaddedResult(0,totalOffset,path);
}

PaddedResult PaddedTrackManager::getPaddedResult(int songKey, int totalOffset, gd::string path){
    PaddedResult result;
    result.resultingPath=path;

    if(lso::utils::isFilePadded(path)){
        result.isPadded=true;
        return result;
    }
    // Not a padded file - Redirect to padded file if it exists, otherwise fallback to original.
    if(songKey==0){
        songKey = extractSongIdFromPath(path);
    }
    auto paddedPath = getPaddedPath(songKey, totalOffset, path);

    std::error_code ec;
    bool exists = std::filesystem::exists(paddedPath, ec);
    LOG_MOD_DEBUG("queueStartMusic: originalPath={}, paddedPath='{}', exists={}", path, paddedPath, exists);
    if(exists){
        result.isPadded = true;
        result.resultingPath = paddedPath.string();
    };
    return result;
}