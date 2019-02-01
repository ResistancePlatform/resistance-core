// Copyright (c) 2019 The Resistance Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PRIVATIZER_H
#define PRIVATIZER_H

#include <string>

extern void ThreadPrivatizer();
extern void OnFinishedZsendmany(const std::string &opid, bool success);

#endif // PRIVATIZER_H
