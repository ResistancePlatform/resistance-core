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

#ifndef resistance_globals__h
#define resistance_globals__h

#include "resistance_defs.h"
#include "resistance_structs.h"

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

#define RESISTANCE_PAXMAX (10000 * COIN)
#define BTCFACTOR_HEIGHT 466266

#define GENESIS_NBITS 0x1f00ffff
#define RESISTANCE_MINRATIFY ((height < 90000) ? 7 : 11)
#define RESISTANCE_NOTARIES_HARDCODED 180000 // DONT CHANGE
#define RESISTANCE_MAXBLOCKS 250000 // DONT CHANGE

#define RESISTANCE_EVENT_RATIFY 'P'
#define RESISTANCE_EVENT_NOTARIZED 'N'
#define RESISTANCE_EVENT_RESHEIGHT 'K' // TODO
#define RESISTANCE_EVENT_REWIND 'B'
#define RESISTANCE_EVENT_PRICEFEED 'V'
#define RESISTANCE_EVENT_OPRETURN 'R'
#define RESISTANCE_OPRETURN_DEPOSIT 'D'
#define RESISTANCE_OPRETURN_ISSUED 'I' // assetchain
#define RESISTANCE_OPRETURN_WITHDRAW 'W' // assetchain
#define RESISTANCE_OPRETURN_REDEEMED 'X'

#define RESISTANCE_KVPROTECTED 1
#define RESISTANCE_KVBINARY 2
#define RESISTANCE_KVDURATION 1440
#define RESISTANCE_ASSETCHAIN_MAXLEN 65

#define RESISTANCE_ELECTION_GAP 2000    //((ASSETCHAINS_SYMBOL[0] == 0) ? 2000 : 100)
#define IGUANA_MAXSCRIPTSIZE 10001
#define RESISTANCE_ASSETCHAIN_MAXLEN 65
#define PRIVATIZER_MAXSECRETADDRS 777

#define MAX_CURRENCIES 32

#define PRIVATIZER_ADDR "rpS7CvbLZiXfxaXAugHznw3SyVtNRWpAFbJ"
#define PRIVATIZER_SYNCHRONIZED_BLOCKS 10
#define PRIVATIZER_INCR 9.965
#define PRIVATIZER_FEE 0.001
#define PRIVATIZER_TXFEE 0.01
#define SMALLVAL 0.000000000000001

#define PRIVATIZER_ERROR_DUPLICATEDEPOSIT -1
#define PRIVATIZER_ERROR_SECRETCANTBEDEPOSIT -2
#define PRIVATIZER_ERROR_TOOMANYSECRETS -3
#define PRIVATIZER_ERROR_NOTINWALLET -4

#define RESISTANCE_MAINNET_START 178999
#define RESISTANCE_NOTARIES_TIMESTAMP1 1525132800 // May 1st 2018 1530921600 // 7/7/2017
#define RESISTANCE_NOTARIES_HEIGHT1 ((814000 / RESISTANCE_ELECTION_GAP) * RESISTANCE_ELECTION_GAP)

extern pthread_mutex_t resistance_mutex;

extern struct pax_transaction *PAX;
extern int32_t NUM_PRICES;
extern uint32_t *PVALS;
extern struct knotaries_entry *Pubkeys;

extern struct resistance_state RESISTANCE_STATES[34];

extern int32_t IS_RESISTANCE_NOTARY,USE_EXTERNAL_PUBKEY,RESISTANCE_CHOSEN_ONE,ASSETCHAINS_SEED,RESISTANCE_ON_DEMAND,RESISTANCE_EXTERNAL_NOTARIES,RESISTANCE_PASSPORT_INITDONE,RESISTANCE_PAX,RESISTANCE_EXCHANGEWALLET,RESISTANCE_REWIND;
extern int32_t RESISTANCE_LASTMINED,prevRESISTANCE_LASTMINED,PRIVATIZER_PAUSE,ASSETCHAINS_CC;
extern std::string NOTARY_PUBKEY,ASSETCHAINS_NOTARIES,ASSETCHAINS_OVERRIDE_PUBKEY;
extern uint8_t NOTARY_PUBKEY33[33],ASSETCHAINS_OVERRIDE_PUBKEY33[33];

extern char ASSETCHAINS_SYMBOL[RESISTANCE_ASSETCHAIN_MAXLEN],ASSETCHAINS_USERPASS[4096];
extern uint16_t ASSETCHAINS_PORT;
extern uint32_t ASSETCHAIN_INIT;
extern uint32_t ASSETCHAINS_MAGIC;
extern uint64_t ASSETCHAINS_ENDSUBSIDY,ASSETCHAINS_REWARD,ASSETCHAINS_HALVING,ASSETCHAINS_DECAY,ASSETCHAINS_COMMISSION,ASSETCHAINS_STAKED,ASSETCHAINS_SUPPLY;

extern uint32_t RESISTANCE_INITDONE;
extern char RESUSERPASS[4096],BTCUSERPASS[4096];
extern uint16_t RES_PORT,BITCOIND_PORT;
extern uint64_t PENDING_RESISTANCE_TX;

extern struct resistance_kv *RESISTANCE_KV;
extern pthread_mutex_t RESISTANCE_KV_mutex;

extern int RESISTANCE_COINBASE_MATURITY;
extern CAmount RESISTANCE_MAX_MONEY;

extern char Privatizer_secretaddrs[PRIVATIZER_MAXSECRETADDRS][64],Privatizer_deposit[64];
extern int32_t Privatizer_numsecretaddrs; // if 0 -> run silent mode

extern uint64_t M1SUPPLY[];
extern int32_t Peggy_inds[539];
extern uint64_t peggy_smooth_coeffs[sizeof(Peggy_inds)/sizeof(*Peggy_inds)];
extern const char CURRENCIES[][8];
extern int32_t RESISTANCE_LONGESTCHAIN;

extern struct privatizer_item *Privatizers;

#endif //resistance_globals__h
