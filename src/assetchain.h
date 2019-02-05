// Copyright (c) 2019 The Resistance Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ASSETCHAIN_H
#define ASSETCHAIN_H

#include <string>

#include <boost/filesystem/path.hpp>

class CBaseChainParams;
class CChainParams;

extern boost::filesystem::path GetAssetchainDefaultDataDir(const std::string &assetchainIn);
extern boost::filesystem::path GetDefaultAssetchainConfigFile(const std::string &assetchainIn);
extern void SetAssetchainParams(const std::string &assetchainIn, CBaseChainParams &baseParams, CChainParams &params);

#endif // ASSETCHAIN_H