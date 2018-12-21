/******************************************************************************
 * Copyright © 2014-2017 The SuperNET Developers.                             *
 *                                                                            *
 * See the AUTHORS, DEVELOPER-AGREEMENT and LICENSE files at                  *
 * the top-level directory of this distribution for the individual copyright  *
 * holder information and the developer policies on copyright and licensing.  *
 *                                                                            *
 * Unless otherwise agreed in a custom licensing agreement, no part of the    *
 * SuperNET software, including this file may be copied, modified, propagated *
 * or distributed except according to the terms contained in the LICENSE file *
 *                                                                            *
 * Removal or modification of this copyright notice is prohibited.            *
 *                                                                            *
 ******************************************************************************/

#ifndef komodo_globals__h
#define komodo_globals__h

#include "komodo_defs.h"
#include "komodo_structs.h"

#include "amount.h"
#include "util.h"

#define USD 0
#define EUR 1
#define JPY 2
#define GBP 3
#define AUD 4
#define CAD 5
#define CHF 6
#define NZD 7
#define CNY 8
#define RUB 9
#define MXN 10
#define BRL 11
#define INR 12
#define HKD 13
#define TRY 14
#define ZAR 15

#define PLN 16
#define NOK 17
#define SEK 18
#define DKK 19
#define CZK 20
#define HUF 21
#define ILS 22
#define KRW 23

#define MYR 24
#define PHP 25
#define RON 26
#define SGD 27
#define THB 28
#define BGN 29
#define IDR 30
#define HRK 31

#define KOMODO_PAXMAX (10000 * COIN)
#define BTCFACTOR_HEIGHT 466266

#define GENESIS_NBITS 0x1f00ffff
#define KOMODO_MINRATIFY ((height < 90000) ? 7 : 11)
#define KOMODO_NOTARIES_HARDCODED 180000 // DONT CHANGE
#define KOMODO_MAXBLOCKS 250000 // DONT CHANGE

#define KOMODO_EVENT_RATIFY 'P'
#define KOMODO_EVENT_NOTARIZED 'N'
#define KOMODO_EVENT_RESHEIGHT 'K' // TODO
#define KOMODO_EVENT_REWIND 'B'
#define KOMODO_EVENT_PRICEFEED 'V'
#define KOMODO_EVENT_OPRETURN 'R'
#define KOMODO_OPRETURN_DEPOSIT 'D'
#define KOMODO_OPRETURN_ISSUED 'I' // assetchain
#define KOMODO_OPRETURN_WITHDRAW 'W' // assetchain
#define KOMODO_OPRETURN_REDEEMED 'X'

#define KOMODO_KVPROTECTED 1
#define KOMODO_KVBINARY 2
#define KOMODO_KVDURATION 1440
#define KOMODO_ASSETCHAIN_MAXLEN 65

#define KOMODO_ELECTION_GAP 2000    //((ASSETCHAINS_SYMBOL[0] == 0) ? 2000 : 100)
#define IGUANA_MAXSCRIPTSIZE 10001
#define KOMODO_ASSETCHAIN_MAXLEN 65
#define JUMBLR_MAXSECRETADDRS 777

#define MAX_CURRENCIES 32

#define JUMBLR_ADDR "rpS7CvbLZiXfxaXAugHznw3SyVtNRWpAFbJ"
#define JUMBLR_SYNCHRONIZED_BLOCKS 10
#define JUMBLR_INCR 9.965
#define JUMBLR_FEE 0.001
#define JUMBLR_TXFEE 0.01
#define SMALLVAL 0.000000000000001

#define JUMBLR_ERROR_DUPLICATEDEPOSIT -1
#define JUMBLR_ERROR_SECRETCANTBEDEPOSIT -2
#define JUMBLR_ERROR_TOOMANYSECRETS -3
#define JUMBLR_ERROR_NOTINWALLET -4

#define KOMODO_MAINNET_START 178999
#define KOMODO_NOTARIES_TIMESTAMP1 1525132800 // May 1st 2018 1530921600 // 7/7/2017
#define KOMODO_NOTARIES_HEIGHT1 ((814000 / KOMODO_ELECTION_GAP) * KOMODO_ELECTION_GAP)

extern pthread_mutex_t komodo_mutex;

extern struct pax_transaction *PAX;
extern int32_t NUM_PRICES;
extern uint32_t *PVALS;
extern struct knotaries_entry *Pubkeys;

extern struct komodo_state KOMODO_STATES[34];

extern int32_t IS_KOMODO_NOTARY,USE_EXTERNAL_PUBKEY,KOMODO_CHOSEN_ONE,ASSETCHAINS_SEED,KOMODO_ON_DEMAND,KOMODO_EXTERNAL_NOTARIES,KOMODO_PASSPORT_INITDONE,KOMODO_PAX,KOMODO_EXCHANGEWALLET,KOMODO_REWIND;
extern int32_t KOMODO_LASTMINED,prevKOMODO_LASTMINED,JUMBLR_PAUSE,ASSETCHAINS_CC;
extern std::string NOTARY_PUBKEY,ASSETCHAINS_NOTARIES,ASSETCHAINS_OVERRIDE_PUBKEY;
extern uint8_t NOTARY_PUBKEY33[33],ASSETCHAINS_OVERRIDE_PUBKEY33[33];

extern char ASSETCHAINS_SYMBOL[KOMODO_ASSETCHAIN_MAXLEN],ASSETCHAINS_USERPASS[4096];
extern uint16_t ASSETCHAINS_PORT;
extern uint32_t ASSETCHAIN_INIT;
extern uint32_t ASSETCHAINS_MAGIC;
extern uint64_t ASSETCHAINS_ENDSUBSIDY,ASSETCHAINS_REWARD,ASSETCHAINS_HALVING,ASSETCHAINS_DECAY,ASSETCHAINS_COMMISSION,ASSETCHAINS_STAKED,ASSETCHAINS_SUPPLY;

extern uint32_t KOMODO_INITDONE;
extern char RESUSERPASS[4096],BTCUSERPASS[4096];
extern uint16_t RES_PORT,BITCOIND_PORT;
extern uint64_t PENDING_KOMODO_TX;

extern struct komodo_kv *KOMODO_KV;
extern pthread_mutex_t KOMODO_KV_mutex;

extern int KOMODO_COINBASE_MATURITY;
extern CAmount KOMODO_MAX_MONEY;

extern char Jumblr_secretaddrs[JUMBLR_MAXSECRETADDRS][64],Jumblr_deposit[64];
extern int32_t Jumblr_numsecretaddrs; // if 0 -> run silent mode

extern uint64_t M1SUPPLY[];
extern int32_t Peggy_inds[539];
extern uint64_t peggy_smooth_coeffs[sizeof(Peggy_inds)/sizeof(*Peggy_inds)];
extern const char CURRENCIES[][8];
extern int32_t KOMODO_LONGESTCHAIN;

extern struct jumblr_item *Jumblrs;

#endif //komodo_globals__h
