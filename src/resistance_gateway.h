
#ifndef resistance_gateway__h
#define resistance_gateway__h

#include "resistance_defs.h"
#include "resistance_globals.h"

int32_t resistance_parsestatefile(struct resistance_state *sp,FILE *fp,char *symbol,char *dest);
int32_t memread(void *dest,int32_t size,uint8_t *filedata,long *fposp,long datalen);
int32_t resistance_parsestatefiledata(struct resistance_state *sp,uint8_t *filedata,long *fposp,long datalen,char *symbol,char *dest);
void *OS_loadfile(char *fname,uint8_t **bufp,long *lenp,long *allocsizep);
uint8_t *OS_fileptr(long *allocsizep,char *fname);
int32_t resistance_longestchain();
int32_t resistance_isrealtime(int32_t *resheightp);
uint64_t resistance_paxtotal();
bool resistance_passport_iteration();
int32_t resistance_paxcmp(char *symbol,int32_t resheight,uint64_t value,uint64_t checkvalue,uint64_t seed);
void pax_keyset(uint8_t *buf,uint256 txid,uint16_t vout,uint8_t type);
struct pax_transaction *resistance_paxfind(uint256 txid,uint16_t vout,uint8_t type);
struct pax_transaction *resistance_paxfinds(uint256 txid,uint16_t vout);
void resistance_gateway_deposit(char *coinaddr,uint64_t value,const char *symbol,uint64_t fiatoshis,uint8_t *rmd160,uint256 txid,uint16_t vout,uint8_t type,int32_t height,int32_t otherheight,const char *source,int32_t approved);
int32_t resistance_rwapproval(int32_t rwflag,uint8_t *opretbuf,struct pax_transaction *pax);
int32_t resistance_issued_opreturn(char *base,uint256 *txids,uint16_t *vouts,int64_t *values,int64_t *srcvalues,int32_t *resheights,int32_t *otherheights,int8_t *baseids,uint8_t *rmd160s,uint8_t *opretbuf,int32_t opretlen,int32_t isresistance);
struct pax_transaction *resistance_paxmark(int32_t height,uint256 txid,uint16_t vout,uint8_t type,int32_t mark);
void resistance_paxdelete(struct pax_transaction *pax);
const char *resistance_opreturn(int32_t height,uint64_t value,uint8_t *opretbuf,int32_t opretlen,uint256 txid,uint16_t vout,char *source);
long resistance_indfile_update(FILE *indfp,uint32_t *prevpos100p,long lastfpos,long newfpos,uint8_t func,uint32_t *indcounterp);
void resistance_stateind_set(struct resistance_state *sp,uint32_t *inds,int32_t n,uint8_t *filedata,long datalen,char *symbol,char *dest);
long resistance_stateind_validate(struct resistance_state *sp,char *indfname,uint8_t *filedata,long datalen,uint32_t *prevpos100p,uint32_t *indcounterp,char *symbol,char *dest);
int32_t resistance_faststateinit(struct resistance_state *sp,char *fname,char *symbol,char *dest);

#endif //resistance_gateway__h