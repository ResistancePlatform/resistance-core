
#ifndef komodo_gateway__h
#define komodo_gateway__h

#include "komodo_defs.h"
#include "komodo_globals.h"

int32_t komodo_parsestatefile(struct komodo_state *sp,FILE *fp,char *symbol,char *dest);
int32_t memread(void *dest,int32_t size,uint8_t *filedata,long *fposp,long datalen);
int32_t komodo_parsestatefiledata(struct komodo_state *sp,uint8_t *filedata,long *fposp,long datalen,char *symbol,char *dest);
void *OS_loadfile(char *fname,uint8_t **bufp,long *lenp,long *allocsizep);
uint8_t *OS_fileptr(long *allocsizep,char *fname);
int32_t komodo_longestchain();
int32_t komodo_isrealtime(int32_t *kmdheightp);
uint64_t komodo_paxtotal();
bool komodo_passport_iteration();
int32_t komodo_paxcmp(char *symbol,int32_t kmdheight,uint64_t value,uint64_t checkvalue,uint64_t seed);
void pax_keyset(uint8_t *buf,uint256 txid,uint16_t vout,uint8_t type);
struct pax_transaction *komodo_paxfind(uint256 txid,uint16_t vout,uint8_t type);
struct pax_transaction *komodo_paxfinds(uint256 txid,uint16_t vout);
void komodo_gateway_deposit(char *coinaddr,uint64_t value,const char *symbol,uint64_t fiatoshis,uint8_t *rmd160,uint256 txid,uint16_t vout,uint8_t type,int32_t height,int32_t otherheight,const char *source,int32_t approved);
int32_t komodo_rwapproval(int32_t rwflag,uint8_t *opretbuf,struct pax_transaction *pax);
int32_t komodo_issued_opreturn(char *base,uint256 *txids,uint16_t *vouts,int64_t *values,int64_t *srcvalues,int32_t *kmdheights,int32_t *otherheights,int8_t *baseids,uint8_t *rmd160s,uint8_t *opretbuf,int32_t opretlen,int32_t iskomodo);
struct pax_transaction *komodo_paxmark(int32_t height,uint256 txid,uint16_t vout,uint8_t type,int32_t mark);
void komodo_paxdelete(struct pax_transaction *pax);
const char *komodo_opreturn(int32_t height,uint64_t value,uint8_t *opretbuf,int32_t opretlen,uint256 txid,uint16_t vout,char *source);
long komodo_indfile_update(FILE *indfp,uint32_t *prevpos100p,long lastfpos,long newfpos,uint8_t func,uint32_t *indcounterp);
void komodo_stateind_set(struct komodo_state *sp,uint32_t *inds,int32_t n,uint8_t *filedata,long datalen,char *symbol,char *dest);
long komodo_stateind_validate(struct komodo_state *sp,char *indfname,uint8_t *filedata,long datalen,uint32_t *prevpos100p,uint32_t *indcounterp,char *symbol,char *dest);
int32_t komodo_faststateinit(struct komodo_state *sp,char *fname,char *symbol,char *dest);

#endif //komodo_gateway__h