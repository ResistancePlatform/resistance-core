// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://www.opensource.org/licenses/mit-license.php .

#ifndef BITCOIN_SUPPORT_CLEANSE_H
#define BITCOIN_SUPPORT_CLEANSE_H

#ifdef __GNUC__
#include <string.h>

static inline void memory_cleanse(void *ptr, size_t len)
{
	memset(ptr, 0, len);
	__asm__ __volatile__("" : : "r"(ptr) : "memory");
}
#else
#include <stdlib.h>

void memory_cleanse(void *ptr, size_t len);
#endif

#endif // BITCOIN_SUPPORT_CLEANSE_H
