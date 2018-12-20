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

uint32_t komodo_heightstamp(int32_t height);
void komodo_stateupdate(int32_t height,uint8_t notarypubs[][33],uint8_t numnotaries,uint8_t notaryid,uint256 txhash,uint64_t voutmask,uint8_t numvouts,uint32_t *pvals,uint8_t numpvals,int32_t kheight,uint32_t ktime,uint64_t opretvalue,uint8_t *opretbuf,uint16_t opretlen,uint16_t vout);
void komodo_init(int32_t height);
int32_t komodo_notarizeddata(int32_t nHeight,uint256 *notarized_hashp,uint256 *notarized_desttxidp);
void komodo_init(int32_t height);
int32_t komodo_chosennotary(int32_t *notaryidp,int32_t height,uint8_t *pubkey33,uint32_t timestamp);
int32_t komodo_isrealtime(int32_t *resheightp);
uint64_t komodo_paxtotal();
int32_t komodo_longestchain();
int32_t komodo_bannedset(int32_t *indallvoutsp,uint256 *array,int32_t max);

#define KOMODO_ELECTION_GAP 2000    //((ASSETCHAINS_SYMBOL[0] == 0) ? 2000 : 100)
#define IGUANA_MAXSCRIPTSIZE 10001
#define KOMODO_ASSETCHAIN_MAXLEN 65
#define JUMBLR_MAXSECRETADDRS 777

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

#endif //komodo_globals__h
