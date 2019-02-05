// Copyright (c) 2019 The Resistance Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PRIVATIZER_H
#define PRIVATIZER_H

#include <univalue.h>

#include <string>

extern void ThreadPrivatizer();
extern void OnFinishedZsendmany(const std::string &opid, bool success);
extern UniValue GetPrivatizerAsUniValue(const std::string &publicAddress, const std::string &privateAddress);
extern UniValue GetPrivatizerInfoAsUniValue(const std::string &publicAddress, const std::string &privateAddress);
extern UniValue GetPrivatizersAsUniValue();
extern UniValue GetPrivatizerInfosAsUniValue();
extern void AddPrivatizer(const std::string &publicAddress, const std::string &privateAddress, bool overwrite);
extern void RemovePrivatizer(const std::string &publicAddress, const std::string &privateAddress, bool matchAll);

#endif // PRIVATIZER_H
