// Copyright (c) 2019 The Resistance Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "assetchain.h"

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <algorithm>

#include "protocol.h"
#include "util.h"
#include "chainparamsbase.h"
#include "chainparams.h"

boost::filesystem::path GetAssetchainDefaultDataDir(const std::string &assetchainIn)
{
    namespace fs = boost::filesystem;
    std::string assetchain = assetchainIn;
    std::transform(assetchain.begin(), assetchain.end(), assetchain.begin(), ::tolower);

    // Windows < Vista: C:\Documents and Settings\Username\Application Data\Resistance\AssetChain\[assetchain]
    // Windows >= Vista: C:\Users\Username\AppData\Roaming\Resistance\AssetChain\[assetchain]
    // Mac: ~/Library/Application Support/Resistance/AssetChain/[assetchain]
    // Unix: ~/.resistance/assetchain/[assetchain]
#ifdef WIN32
    // Windows
    return GetSpecialFolderPath(CSIDL_APPDATA) / "Resistance/assetchain" / assetchain;
#else
    fs::path pathRet;
    char* pszHome = getenv("HOME");
    if (pszHome == NULL || strlen(pszHome) == 0)
        pathRet = fs::path("/");
    else
        pathRet = fs::path(pszHome);
#ifdef MAC_OSX
    // Mac
    pathRet /= "Library/Application Support";
    TryCreateDirectory(pathRet);
    return pathRet / "Resistance/assetchain" / assetchain;
#else
    // Unix
    return pathRet / ".resistance/assetchain" / assetchain;
#endif
#endif
}

boost::filesystem::path GetDefaultAssetchainConfigFile(const std::string &assetchainIn)
{
    std::string assetchain = assetchainIn;
    std::transform(assetchain.begin(), assetchain.end(), assetchain.begin(), ::tolower);
    boost::filesystem::path pathConfigFile = GetDataDir(false);
    boost::filesystem::create_directories(pathConfigFile);
    pathConfigFile = pathConfigFile / (assetchain + ".conf");

    boost::filesystem::ifstream streamConfig(pathConfigFile);
    if (!streamConfig.good()) {
        fprintf(stderr, "Warning! The config file could be completely empty!\n"
                        "Recommend to fill the options what you want, but creating an empty file %s.\n", pathConfigFile.string().c_str());
        LogPrintf("Creating empty config file %s.\n", pathConfigFile.string());
        boost::filesystem::ofstream emptyConfig(pathConfigFile);
    }

    return pathConfigFile;
}

void SetAssetchainParams(const std::string &assetchainIn, CBaseChainParams &baseParams, CChainParams &params)
{
    std::string assetchain = assetchainIn;
    std::transform(assetchain.begin(), assetchain.end(), assetchain.begin(), ::tolower);
    
    uint32_t hash = 0;
    for (std::string::size_type i = 0; i < assetchain.size(); i++)
        hash += ((uint32_t)assetchain[i]) << (8 * (i % 3));
    uint16_t portDelta = (1 + (hash % 4994)) * 2;
    
    CMessageHeader::MessageStartChars msgChars;
    memcpy(&msgChars, params.MessageStart(), sizeof(msgChars));    
    msgChars[0] += hash & 0xFF;
    msgChars[1] += (hash >> 8) & 0xFF;
    msgChars[2] += (hash >> 16) & 0xFF;
    msgChars[3] += (hash >> 24) & 0xFF;
    baseParams.SetRPCPort(baseParams.RPCPort() + portDelta);
    params.SetDefaultPort(params.GetDefaultPort() + portDelta);
    params.SetCurrencyUnits(assetchain);
    params.SetMessageStart(msgChars);

    LogPrintf("AssetChain default port %d, rpc port %d, network magic %02X:%02X:%02X:%02X\n",
        params.GetDefaultPort(),
        baseParams.RPCPort(),
        msgChars[0], msgChars[1], msgChars[2], msgChars[3]);
}
