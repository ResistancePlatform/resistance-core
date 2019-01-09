
#include "resistance_gateway.h"

#include "resistance_utils.h"
#include "resistance_privatizer.h"
#include "resistance_events.h"
#include "resistance_kv.h"
#include "net.h"
#include "main.h"

#include <vector>
#include <boost/foreach.hpp>

int32_t resistance_parsestatefile(struct resistance_state *sp,FILE *fp,char *symbol,char *dest)
{
    static int32_t errs;
    int32_t func,ht,notarized_height,num,matched=0,MoMdepth; uint256 MoM,notarized_hash,notarized_desttxid; uint8_t pubkeys[64][33];
    if ( (func= fgetc(fp)) != EOF )
    {
        if ( ASSETCHAINS_SYMBOL[0] == 0 && strcmp(symbol,"RES") == 0 )
            matched = 1;
        else matched = (strcmp(symbol,ASSETCHAINS_SYMBOL) == 0);
        if ( fread(&ht,1,sizeof(ht),fp) != sizeof(ht) )
            errs++;
        if ( 0 && ASSETCHAINS_SYMBOL[0] != 0 && func != 'T' )
            LogPrintf("[%s] matched.%d fpos.%ld func.(%d %c) ht.%d\n",ASSETCHAINS_SYMBOL,matched,ftell(fp),func,func,ht);
        if ( func == 'P' )
        {
            if ( (num= fgetc(fp)) <= 64 )
            {
                if ( fread(pubkeys,33,num,fp) != num )
                    errs++;
                else
                {
                    //LogPrintf("updated %d pubkeys at %s ht.%d\n",num,symbol,ht);
                    if ( (RESISTANCE_EXTERNAL_NOTARIES != 0 && matched != 0) || (strcmp(symbol,"RES") == 0 && RESISTANCE_EXTERNAL_NOTARIES == 0) )
                        resistance_eventadd_pubkeys(sp,symbol,ht,num,pubkeys);
                }
            } else error("illegal num.%d\n",num);
        }
        else if ( func == 'N' || func == 'M' )
        {
            if ( fread(&notarized_height,1,sizeof(notarized_height),fp) != sizeof(notarized_height) )
                errs++;
            if ( fread(&notarized_hash,1,sizeof(notarized_hash),fp) != sizeof(notarized_hash) )
                errs++;
            if ( fread(&notarized_desttxid,1,sizeof(notarized_desttxid),fp) != sizeof(notarized_desttxid) )
                errs++;
            if ( func == 'M' )
            {
                if ( fread(&MoM,1,sizeof(MoM),fp) != sizeof(MoM) )
                    errs++;
                if ( fread(&MoMdepth,1,sizeof(MoMdepth),fp) != sizeof(MoMdepth) )
                    errs++;
                if ( 1 && ASSETCHAINS_SYMBOL[0] != 0 && sp != 0 )
                    LogPrintf("%s load[%s.%d -> %s] NOTARIZED %d %s MoM.%s %d\n",ASSETCHAINS_SYMBOL,symbol,sp->NUM_NPOINTS,dest,notarized_height,notarized_hash.ToString().c_str(),MoM.ToString().c_str(),MoMdepth);
            }
            else
            {
                memset(&MoM,0,sizeof(MoM));
                MoMdepth = 0;
            }
            //if ( matched != 0 ) global independent states -> inside *sp
            resistance_eventadd_notarized(sp,symbol,ht,dest,notarized_hash,notarized_desttxid,notarized_height);//,MoM,MoMdepth);
        }
        else if ( func == 'U' ) // deprecated
        {
            uint8_t n,nid; uint256 hash; uint64_t mask;
            n = fgetc(fp);
            nid = fgetc(fp);
            //LogPrintf("U %d %d\n",n,nid);
            if ( fread(&mask,1,sizeof(mask),fp) != sizeof(mask) )
                errs++;
            if ( fread(&hash,1,sizeof(hash),fp) != sizeof(hash) )
                errs++;
            //if ( matched != 0 )
            //    resistance_eventadd_utxo(sp,symbol,ht,nid,hash,mask,n);
        }
        else if ( func == 'K' )
        {
            int32_t kheight;
            if ( fread(&kheight,1,sizeof(kheight),fp) != sizeof(kheight) )
                errs++;
            //if ( matched != 0 ) global independent states -> inside *sp
            //LogPrintf("%s.%d load[%s] ht.%d\n",ASSETCHAINS_SYMBOL,ht,symbol,kheight);
            resistance_eventadd_resheight(sp,symbol,ht,kheight,0);
        }
        else if ( func == 'T' )
        {
            int32_t kheight,ktimestamp;
            if ( fread(&kheight,1,sizeof(kheight),fp) != sizeof(kheight) )
                errs++;
            if ( fread(&ktimestamp,1,sizeof(ktimestamp),fp) != sizeof(ktimestamp) )
                errs++;
            //if ( matched != 0 ) global independent states -> inside *sp
            //LogPrintf("%s.%d load[%s] ht.%d t.%u\n",ASSETCHAINS_SYMBOL,ht,symbol,kheight,ktimestamp);
            resistance_eventadd_resheight(sp,symbol,ht,kheight,ktimestamp);
        }
        else if ( func == 'R' )
        {
            uint16_t olen,v; uint64_t ovalue; uint256 txid; uint8_t opret[16384];
            if ( fread(&txid,1,sizeof(txid),fp) != sizeof(txid) )
                errs++;
            if ( fread(&v,1,sizeof(v),fp) != sizeof(v) )
                errs++;
            if ( fread(&ovalue,1,sizeof(ovalue),fp) != sizeof(ovalue) )
                errs++;
            if ( fread(&olen,1,sizeof(olen),fp) != sizeof(olen) )
                errs++;
            if ( olen < sizeof(opret) )
            {
                if ( fread(opret,1,olen,fp) != olen )
                    errs++;
                if ( 0 && ASSETCHAINS_SYMBOL[0] != 0 && matched != 0 )
                {
                    int32_t i;  for (i=0; i<olen; i++)
                        LogPrintf("%02x",opret[i]);
                    LogPrintf(" %s.%d load[%s] opret[%c] len.%d %.8f\n",ASSETCHAINS_SYMBOL,ht,symbol,opret[0],olen,(double)ovalue/COIN);
                }
                resistance_eventadd_opreturn(sp,symbol,ht,txid,ovalue,v,opret,olen); // global shared state -> global PAX
            }
            else
            {
                int32_t i;
                for (i=0; i<olen; i++)
                    fgetc(fp);
                //LogPrintf("illegal olen.%u\n",olen);
            }
        }
        else if ( func == 'D' )
        {
            error("unexpected function D[%d]\n",ht);
        }
        else if ( func == 'V' )
        {
            int32_t numpvals; uint32_t pvals[128];
            numpvals = fgetc(fp);
            if ( numpvals*sizeof(uint32_t) <= sizeof(pvals) && fread(pvals,sizeof(uint32_t),numpvals,fp) == numpvals )
            {
                //if ( matched != 0 ) global shared state -> global PVALS
                //LogPrintf("%s load[%s] prices %d\n",ASSETCHAINS_SYMBOL,symbol,ht);
                resistance_eventadd_pricefeed(sp,symbol,ht,pvals,numpvals);
                //LogPrintf("load pvals ht.%d numpvals.%d\n",ht,numpvals);
            } else error("error loading pvals[%d]\n",numpvals);
        }
        else error("[%s] %s illegal func.(%d %c)\n",ASSETCHAINS_SYMBOL,symbol,func,func);
        return(func);
    } else return(-1);
}

int32_t memread(void *dest,int32_t size,uint8_t *filedata,long *fposp,long datalen)
{
    if ( *fposp+size <= datalen )
    {
        memcpy(dest,&filedata[*fposp],size);
        (*fposp) += size;
        return(size);
    }
    return(-1);
}

int32_t resistance_parsestatefiledata(struct resistance_state *sp,uint8_t *filedata,long *fposp,long datalen,char *symbol,char *dest)
{
    static int32_t errs;
    int32_t func= -1,ht,notarized_height,MoMdepth,num,matched=0; uint256 MoM,notarized_hash,notarized_desttxid; uint8_t pubkeys[64][33]; long fpos = *fposp;
    if ( fpos < datalen )
    {
        func = filedata[fpos++];
        if ( ASSETCHAINS_SYMBOL[0] == 0 && strcmp(symbol,"RES") == 0 )
            matched = 1;
        else matched = (strcmp(symbol,ASSETCHAINS_SYMBOL) == 0);
        if ( memread(&ht,sizeof(ht),filedata,&fpos,datalen) != sizeof(ht) )
            errs++;
        if ( func == 'P' )
        {
            if ( (num= filedata[fpos++]) <= 64 )
            {
                if ( memread(pubkeys,33*num,filedata,&fpos,datalen) != 33*num )
                    errs++;
                else
                {
                    //LogPrintf("updated %d pubkeys at %s ht.%d\n",num,symbol,ht);
                    if ( (RESISTANCE_EXTERNAL_NOTARIES != 0 && matched != 0) || (strcmp(symbol,"RES") == 0 && RESISTANCE_EXTERNAL_NOTARIES == 0) )
                        resistance_eventadd_pubkeys(sp,symbol,ht,num,pubkeys);
                }
            } else error("illegal num.%d\n",num);
        }
        else if ( func == 'N' || func == 'M' )
        {
            if ( memread(&notarized_height,sizeof(notarized_height),filedata,&fpos,datalen) != sizeof(notarized_height) )
                errs++;
            if ( memread(&notarized_hash,sizeof(notarized_hash),filedata,&fpos,datalen) != sizeof(notarized_hash) )
                errs++;
            if ( memread(&notarized_desttxid,sizeof(notarized_desttxid),filedata,&fpos,datalen) != sizeof(notarized_desttxid) )
                errs++;
            if ( func == 'M' )
            {
                if ( memread(&MoM,sizeof(MoM),filedata,&fpos,datalen) != sizeof(MoM) )
                    errs++;
                if ( memread(&MoMdepth,sizeof(MoMdepth),filedata,&fpos,datalen) != sizeof(MoMdepth) )
                    errs++;
                if ( 1 && ASSETCHAINS_SYMBOL[0] != 0 && sp != 0 )
                    LogPrintf("%s load[%s.%d -> %s] NOTARIZED %d %s MoM.%s %d\n",ASSETCHAINS_SYMBOL,symbol,sp->NUM_NPOINTS,dest,notarized_height,notarized_hash.ToString().c_str(),MoM.ToString().c_str(),MoMdepth);
            }
            else
            {
                memset(&MoM,0,sizeof(MoM));
                MoMdepth = 0;
            }
            resistance_eventadd_notarized(sp,symbol,ht,dest,notarized_hash,notarized_desttxid,notarized_height);//,MoM,MoMdepth);
        }
        else if ( func == 'U' ) // deprecated
        {
            uint8_t n,nid; uint256 hash; uint64_t mask;
            n = filedata[fpos++];
            nid = filedata[fpos++];
            //LogPrintf("U %d %d\n",n,nid);
            if ( memread(&mask,sizeof(mask),filedata,&fpos,datalen) != sizeof(mask) )
                errs++;
            if ( memread(&hash,sizeof(hash),filedata,&fpos,datalen) != sizeof(hash) )
                errs++;
        }
        else if ( func == 'K' )
        {
            int32_t kheight;
            if ( memread(&kheight,sizeof(kheight),filedata,&fpos,datalen) != sizeof(kheight) )
                errs++;
             resistance_eventadd_resheight(sp,symbol,ht,kheight,0);
        }
        else if ( func == 'T' )
        {
            int32_t kheight,ktimestamp;
            if ( memread(&kheight,sizeof(kheight),filedata,&fpos,datalen) != sizeof(kheight) )
                errs++;
            if ( memread(&ktimestamp,sizeof(ktimestamp),filedata,&fpos,datalen) != sizeof(ktimestamp) )
                errs++;
            //if ( matched != 0 ) global independent states -> inside *sp
            //LogPrintf("%s.%d load[%s] ht.%d t.%u\n",ASSETCHAINS_SYMBOL,ht,symbol,kheight,ktimestamp);
            resistance_eventadd_resheight(sp,symbol,ht,kheight,ktimestamp);
        }
        else if ( func == 'R' )
        {
            uint16_t olen,v; uint64_t ovalue; uint256 txid; uint8_t opret[16384];
            if ( memread(&txid,sizeof(txid),filedata,&fpos,datalen) != sizeof(txid) )
                errs++;
            if ( memread(&v,sizeof(v),filedata,&fpos,datalen) != sizeof(v) )
                errs++;
            if ( memread(&ovalue,sizeof(ovalue),filedata,&fpos,datalen) != sizeof(ovalue) )
                errs++;
            if ( memread(&olen,sizeof(olen),filedata,&fpos,datalen) != sizeof(olen) )
                errs++;
            if ( olen < sizeof(opret) )
            {
                if ( memread(opret,olen,filedata,&fpos,datalen) != olen )
                    errs++;
                if ( 0 && ASSETCHAINS_SYMBOL[0] != 0 && matched != 0 )
                {
                    int32_t i;  for (i=0; i<olen; i++)
                        LogPrintf("%02x",opret[i]);
                    LogPrintf(" %s.%d load[%s] opret[%c] len.%d %.8f\n",ASSETCHAINS_SYMBOL,ht,symbol,opret[0],olen,(double)ovalue/COIN);
                }
                resistance_eventadd_opreturn(sp,symbol,ht,txid,ovalue,v,opret,olen); // global shared state -> global PAX
            } else
            {
                int32_t i;
                for (i=0; i<olen; i++)
                    filedata[fpos++];
                //LogPrintf("illegal olen.%u\n",olen);
            }
        }
        else if ( func == 'D' )
        {
            error("unexpected function D[%d]\n",ht);
        }
        else if ( func == 'V' )
        {
            int32_t numpvals; uint32_t pvals[128];
            numpvals = filedata[fpos++];
            if ( numpvals*sizeof(uint32_t) <= sizeof(pvals) && memread(pvals,(int32_t)(sizeof(uint32_t)*numpvals),filedata,&fpos,datalen) == numpvals*sizeof(uint32_t) )
            {
                //if ( matched != 0 ) global shared state -> global PVALS
                //LogPrintf("%s load[%s] prices %d\n",ASSETCHAINS_SYMBOL,symbol,ht);
                resistance_eventadd_pricefeed(sp,symbol,ht,pvals,numpvals);
                //LogPrintf("load pvals ht.%d numpvals.%d\n",ht,numpvals);
            } else error("error loading pvals[%d]\n",numpvals);
        }
        else error("[%s] %s illegal func.(%d %c)\n",ASSETCHAINS_SYMBOL,symbol,func,func);
        *fposp = fpos;
        return(func);
    }
    return(-1);
}

void *OS_loadfile(char *fname,uint8_t **bufp,long *lenp,long *allocsizep)
{
    FILE *fp;
    long  filesize,buflen = *allocsizep;
    uint8_t *buf = *bufp;
    *lenp = 0;
    if ( (fp= fopen(fname,"rb")) != 0 )
    {
        fseek(fp,0,SEEK_END);
        filesize = ftell(fp);
        if ( filesize == 0 )
        {
            fclose(fp);
            *lenp = 0;
            LogPrintf("OS_loadfile null size.(%s)\n",fname);
            return(0);
        }
        if ( filesize > buflen )
        {
            *allocsizep = filesize;
            *bufp = buf = (uint8_t *)realloc(buf,(long)*allocsizep+64);
        }
        rewind(fp);
        if ( buf == 0 )
            error("Null buf ???\n");
        else
        {
            if ( fread(buf,1,(long)filesize,fp) != (unsigned long)filesize )
                error("error reading filesize.%ld\n",(long)filesize);
            buf[filesize] = 0;
        }
        fclose(fp);
        *lenp = filesize;
        //LogPrintf("loaded.(%s)\n",buf);
    } //else LogPrintf("OS_loadfile couldnt load.(%s)\n",fname);
    return(buf);
}

uint8_t *OS_fileptr(long *allocsizep,char *fname)
{
    long filesize = 0; uint8_t *buf = 0; void *retptr;
    *allocsizep = 0;
    retptr = OS_loadfile(fname,&buf,&filesize,allocsizep);
    return((uint8_t *)retptr);
}

extern void CopyNodeStats(std::vector<CNodeStats>& vstats);

int32_t resistance_longestchain()
{
    int32_t ht,n=0,num=0,maxheight=0,height = 0;
    LOCK(cs_main);
    std::vector<CNodeStats> vstats;
    CopyNodeStats(vstats);
    BOOST_FOREACH(const CNodeStats& stats, vstats)
    {
        CNodeStateStats statestats;
        bool fStateStats = GetNodeStateStats(stats.nodeid,statestats);
        ht = 0;
        if ( stats.nStartingHeight > ht )
            ht = stats.nStartingHeight;
        if ( statestats.nSyncHeight > ht )
            ht = statestats.nSyncHeight;
        if ( statestats.nCommonHeight > ht )
            ht = statestats.nCommonHeight;
        if ( maxheight == 0 || ht > maxheight*1.01 )
            maxheight = ht, num = 1;
        else if ( ht > maxheight*0.99 )
            num++;
        n++;
        if ( ht > height )
            height = ht;
    }
    if ( num > (n >> 1) )
    {
        RESISTANCE_LONGESTCHAIN = height;
        return(height);
    }
    RESISTANCE_LONGESTCHAIN = 0;
    return(0);
}

int32_t resistance_isrealtime(int32_t *resheightp)
{
    struct resistance_state *sp; CBlockIndex *pindex;
    if ( (sp= resistance_stateptrget((char *)"RES")) != 0 )
        *resheightp = sp->CURRENT_HEIGHT;
    else *resheightp = 0;
    if ( (pindex= chainActive.Tip()) != 0 && pindex->nHeight >= (int32_t)resistance_longestchain() )
        return(1);
    else return(0);
}

uint64_t resistance_paxtotal()
{
    struct pax_transaction *pax,*pax2,*tmp,*tmp2; char symbol[RESISTANCE_ASSETCHAIN_MAXLEN],dest[RESISTANCE_ASSETCHAIN_MAXLEN],*str; int32_t i,ht; int64_t checktoshis; uint64_t seed,total = 0; struct resistance_state *basesp;
    if ( RESISTANCE_PASSPORT_INITDONE == 0 ) //RESISTANCE_PAX == 0 ||
        return(0);
    if ( resistance_isrealtime(&ht) == 0 )
        return(0);
    else
    {
        HASH_ITER(hh,PAX,pax,tmp)
        {
            if ( pax->marked != 0 )
                continue;
            if ( pax->type == 'A' || pax->type == 'D' || pax->type == 'X' )
                str = pax->symbol;
            else str = pax->source;
            basesp = resistance_stateptrget(str);
            if ( basesp != 0 && pax->didstats == 0 )
            {
                if ( pax->type == 'I' && (pax2= resistance_paxfind(pax->txid,pax->vout,'D')) != 0 )
                {
                    if ( pax2->fiatoshis != 0 )
                    {
                        pax->satoshis = pax2->satoshis;
                        pax->fiatoshis = pax2->fiatoshis;
                        basesp->issued += pax->fiatoshis;
                        pax->didstats = 1;
                        if ( strcmp(str,ASSETCHAINS_SYMBOL) == 0 )
                            LogPrintf("########### %p issued %s += %.8f resheight.%d %.8f other.%d\n",basesp,str,dstr(pax->fiatoshis),pax->height,dstr(pax->satoshis),pax->otherheight);
                        pax2->marked = pax->height;
                        pax->marked = pax->height;
                    }
                }
                else if ( pax->type == 'W' )
                {
                    //bitcoin_address(coinaddr,addrtype,rmd160,20);
                    if ( (checktoshis= resistance_paxprice(&seed,pax->height,pax->source,(char *)"RES",(uint64_t)pax->fiatoshis)) != 0 )
                    {
                        if ( resistance_paxcmp(pax->source,pax->height,pax->satoshis,checktoshis,seed) != 0 )
                        {
                            pax->marked = pax->height;
                            //LogPrintf("WITHDRAW.%s mark <- %d %.8f != %.8f\n",pax->source,pax->height,dstr(checktoshis),dstr(pax->satoshis));
                        }
                        else if ( pax->validated == 0 )
                        {
                            pax->validated = pax->satoshis = checktoshis;
                            //int32_t j; for (j=0; j<32; j++)
                            //    LogPrintf("%02x",((uint8_t *)&pax->txid)[j]);
                            //if ( strcmp(str,ASSETCHAINS_SYMBOL) == 0 )
                            //    LogPrintf(" v%d %p got WITHDRAW.%s res.%d ht.%d %.8f -> %.8f/%.8f\n",pax->vout,pax,pax->source,pax->height,pax->otherheight,dstr(pax->fiatoshis),dstr(pax->satoshis),dstr(checktoshis));
                        }
                    }
                }
            }
        }
    }
    resistance_stateptr(symbol,dest);
    HASH_ITER(hh,PAX,pax,tmp)
    {
        pax->ready = 0;
        if ( 0 && pax->type == 'A' )
            LogPrintf("%p pax.%s <- %s marked.%d %.8f -> %.8f validated.%d approved.%d\n",pax,pax->symbol,pax->source,pax->marked,dstr(pax->satoshis),dstr(pax->fiatoshis),pax->validated != 0,pax->approved != 0);
        if ( pax->marked != 0 )
            continue;
        if ( strcmp(symbol,pax->symbol) == 0 || pax->type == 'A' )
        {
            if ( pax->marked == 0 )
            {
                if ( resistance_is_issuer() != 0 )
                {
                    if ( pax->validated != 0 && pax->type == 'D' )
                    {
                        total += pax->fiatoshis;
                        pax->ready = 1;
                    }
                }
                else if ( pax->approved != 0 && pax->type == 'A' )
                {
                    if ( pax->validated != 0 )
                    {
                        total += pax->satoshis;
                        pax->ready = 1;
                    }
                    else
                    {
                        seed = 0;
                        checktoshis = resistance_paxprice(&seed,pax->height,pax->source,(char *)"RES",(uint64_t)pax->fiatoshis);
                        //LogPrintf("paxtotal PAX_fiatdest ht.%d price %s %.8f -> RES %.8f vs %.8f\n",pax->height,pax->symbol,(double)pax->fiatoshis/COIN,(double)pax->satoshis/COIN,(double)checktoshis/COIN);
                        //LogPrintf(" v%d %.8f k.%d ht.%d\n",pax->vout,dstr(pax->satoshis),pax->height,pax->otherheight);
                        if ( seed != 0 && checktoshis != 0 )
                        {
                            if ( checktoshis == pax->satoshis )
                            {
                                total += pax->satoshis;
                                pax->validated = pax->satoshis;
                                pax->ready = 1;
                            } else pax->marked = pax->height;
                        }
                    }
                }
                if ( 0 && pax->ready != 0 )
                    LogPrintf("%p (%c) pax.%s marked.%d %.8f -> %.8f validated.%d approved.%d ready.%d ht.%d\n",pax,pax->type,pax->symbol,pax->marked,dstr(pax->satoshis),dstr(pax->fiatoshis),pax->validated != 0,pax->approved != 0,pax->ready,pax->height);
            }
        }
    }
    //LogPrintf("paxtotal %.8f\n",dstr(total));
    return(total);
}

bool resistance_passport_iteration()
{
    static long lastpos[34]; static char userpass[33][1024]; static uint32_t lasttime,callcounter;
    int32_t maxseconds = 10;
    FILE *fp; uint8_t *filedata; long fpos,datalen,lastfpos; int32_t baseid,limit,n,ht,isrealtime,expired,refid,blocks,longest; struct resistance_state *sp,*refsp; char *retstr,fname[512],*base,symbol[RESISTANCE_ASSETCHAIN_MAXLEN],dest[RESISTANCE_ASSETCHAIN_MAXLEN]; uint32_t buf[3],starttime; cJSON *infoobj,*result; uint64_t RTmask = 0;
    expired = 0;
    while ( RESISTANCE_INITDONE == 0 )
    {
        LogPrintf("[%s] PASSPORT iteration waiting for RESISTANCE_INITDONE\n",ASSETCHAINS_SYMBOL);
        MilliSleep(3000);
        return false;
    }
    refsp = resistance_stateptr(symbol,dest);
    if ( ASSETCHAINS_SYMBOL[0] == 0 )
    {
        refid = 33;
        limit = 10000000;
        privatizer_iteration();
    }
    else
    {
        limit = 10000000;
        refid = resistance_baseid(ASSETCHAINS_SYMBOL)+1; // illegal base -> baseid.-1 -> 0
        if ( refid == 0 )
        {
            RESISTANCE_PASSPORT_INITDONE = 1;
            return true;
        }
    }
    /*if ( RESISTANCE_PAX == 0 )
     {
     RESISTANCE_PASSPORT_INITDONE = 1;
     return;
     }*/
    starttime = (uint32_t)time(NULL);
    if ( callcounter++ < 1 )
        limit = 10000;
    lasttime = starttime;
    for (baseid=32; baseid>=0; baseid--)
    {
        if ( time(NULL) >= starttime+maxseconds )
            break;
        sp = 0;
        isrealtime = 0;
        base = (char *)CURRENCIES[baseid];
        //LogPrintf("PASSPORT %s baseid+1 %d refid.%d\n",ASSETCHAINS_SYMBOL,baseid+1,refid);
        if ( baseid+1 != refid ) // only need to import state from a different coin
        {
            if ( baseid == 32 ) // only care about RES's state
            {
                refsp->RTmask &= ~(1LL << baseid);
                resistance_statefname(fname,baseid<32?base:(char *)"",(char *)"resistancestate");
                resistance_nameset(symbol,dest,base);
                sp = resistance_stateptrget(symbol);
                n = 0;
                if ( lastpos[baseid] == 0 && (filedata= OS_fileptr(&datalen,fname)) != 0 )
                {
                    fpos = 0;
                    LogPrintf("%s processing %s %ldKB\n",ASSETCHAINS_SYMBOL,fname,datalen/1024);
                    while ( resistance_parsestatefiledata(sp,filedata,&fpos,datalen,symbol,dest) >= 0 )
                        lastfpos = fpos;
                    LogPrintf("%s took %d seconds to process %s %ldKB\n",ASSETCHAINS_SYMBOL,(int32_t)(time(NULL)-starttime),fname,datalen/1024);
                    lastpos[baseid] = lastfpos;
                    free(filedata), filedata = 0;
                    datalen = 0;
                }
                else if ( (fp= fopen(fname,"rb")) != 0 && sp != 0 )
                {
                    fseek(fp,0,SEEK_END);
                    //LogPrintf("couldnt OS_fileptr(%s), freading %ldKB\n",fname,ftell(fp)/1024);
                    if ( ftell(fp) > lastpos[baseid] )
                    {
                        if ( ASSETCHAINS_SYMBOL[0] != 0 )
                            LogPrintf("%s passport refid.%d %s fname.(%s) base.%s %ld %ld\n",ASSETCHAINS_SYMBOL,refid,symbol,fname,base,ftell(fp),lastpos[baseid]);
                        fseek(fp,lastpos[baseid],SEEK_SET);
                        while ( resistance_parsestatefile(sp,fp,symbol,dest) >= 0 && n < limit )
                        {
                            if ( n == limit-1 )
                            {
                                if ( time(NULL) < starttime+maxseconds )
                                    n = 0;
                                else
                                {
                                    //LogPrintf("expire passport loop %s -> %s at %ld\n",ASSETCHAINS_SYMBOL,base,lastpos[baseid]);
                                    expired++;
                                }
                            }
                            n++;
                        }
                        lastpos[baseid] = ftell(fp);
                        if ( 0 && lastpos[baseid] == 0 && strcmp(symbol,"RES") == 0 )
                            LogPrintf("from.(%s) lastpos[%s] %ld isrt.%d\n",ASSETCHAINS_SYMBOL,CURRENCIES[baseid],lastpos[baseid],resistance_isrealtime(&ht));
                    } //else LogPrintf("%s.%ld ",CURRENCIES[baseid],ftell(fp));
                    fclose(fp);
                } else error("load error.(%s) %p\n",fname,sp);
                resistance_statefname(fname,baseid<32?base:(char *)"",(char *)"realtime");
                if ( (fp= fopen(fname,"rb")) != 0 )
                {
                    if ( fread(buf,1,sizeof(buf),fp) == sizeof(buf) )
                    {
                        sp->CURRENT_HEIGHT = buf[0];
                        if ( buf[0] != 0 && buf[0] >= buf[1] && buf[2] > time(NULL)-60 )
                        {
                            isrealtime = 1;
                            RTmask |= (1LL << baseid);
                            memcpy(refsp->RTbufs[baseid+1],buf,sizeof(refsp->RTbufs[baseid+1]));
                        }
                        else if ( RESISTANCE_PAX != 0 && (time(NULL)-buf[2]) > 60 && ASSETCHAINS_SYMBOL[0] != 0 )
                            LogPrintf("[%s]: %s not RT %u %u %d\n",ASSETCHAINS_SYMBOL,base,buf[0],buf[1],(int32_t)(time(NULL)-buf[2]));
                    } //else error("%s size error RT\n",base);
                    fclose(fp);
                } //else LogPrintf("%s open error RT\n",base);
            }
        }
        else
        {
            refsp->RTmask &= ~(1LL << baseid);
            resistance_statefname(fname,baseid<32?base:(char *)"",(char *)"realtime");
            if ( (fp= fopen(fname,"wb")) != 0 )
            {
                buf[0] = (uint32_t)chainActive.Tip()->nHeight;
                buf[1] = (uint32_t)resistance_longestchain();
                if ( buf[0] != 0 && buf[0] == buf[1] )
                {
                    buf[2] = (uint32_t)time(NULL);
                    RTmask |= (1LL << baseid);
                    memcpy(refsp->RTbufs[baseid+1],buf,sizeof(refsp->RTbufs[baseid+1]));
                    if ( refid != 0 )
                        memcpy(refsp->RTbufs[0],buf,sizeof(refsp->RTbufs[0]));
                }
                if ( fwrite(buf,1,sizeof(buf),fp) != sizeof(buf) )
                    error("[%s] %s error writing realtime\n",ASSETCHAINS_SYMBOL,base);
                fclose(fp);
            } else LogPrintf("%s create error RT\n",base);
        }
        if ( sp != 0 && isrealtime == 0 )
            refsp->RTbufs[0][2] = 0;
    }
    resistance_paxtotal();
    refsp->RTmask |= RTmask;
    if ( expired == 0 && RESISTANCE_PASSPORT_INITDONE == 0 )
    {
        RESISTANCE_PASSPORT_INITDONE = 1;
        LogPrintf("READY for RPC calls at %u! done PASSPORT %s refid.%d\n",(uint32_t)time(NULL),ASSETCHAINS_SYMBOL,refid);
    }

    return true;
}

int32_t resistance_paxcmp(char *symbol,int32_t resheight,uint64_t value,uint64_t checkvalue,uint64_t seed)
{
    int32_t ratio;
    if ( seed == 0 && checkvalue != 0 )
    {
        ratio = ((value << 6) / checkvalue);
        if ( ratio >= 60 && ratio <= 67 )
            return(0);
        else
        {
            if ( ASSETCHAINS_SYMBOL[0] != 0 )
                LogPrintf("ht.%d ignore mismatched %s value %lld vs checkvalue %lld -> ratio.%d\n",resheight,symbol,(long long)value,(long long)checkvalue,ratio);
            return(-1);
        }
    }
    else if ( checkvalue != 0 )
    {
        ratio = ((value << 10) / checkvalue);
        if ( ratio >= 1023 && ratio <= 1025 )
            return(0);
    }
    return(value != checkvalue);
}

void pax_keyset(uint8_t *buf,uint256 txid,uint16_t vout,uint8_t type)
{
    memcpy(buf,&txid,32);
    memcpy(&buf[32],&vout,2);
    buf[34] = type;
}

struct pax_transaction *resistance_paxfind(uint256 txid,uint16_t vout,uint8_t type)
{
    struct pax_transaction *pax; uint8_t buf[35];
    pthread_mutex_lock(&resistance_mutex);
    pax_keyset(buf,txid,vout,type);
    HASH_FIND(hh,PAX,buf,sizeof(buf),pax);
    pthread_mutex_unlock(&resistance_mutex);
    return(pax);
}

struct pax_transaction *resistance_paxfinds(uint256 txid,uint16_t vout)
{
    struct pax_transaction *pax; int32_t i; uint8_t types[] = { 'I', 'D', 'X', 'A', 'W' };
    for (i=0; i<sizeof(types)/sizeof(*types); i++)
        if ( (pax= resistance_paxfind(txid,vout,types[i])) != 0 )
            return(pax);
    return(0);
}

void resistance_gateway_deposit(char *coinaddr,uint64_t value,const char *symbol,uint64_t fiatoshis,uint8_t *rmd160,uint256 txid,uint16_t vout,uint8_t type,int32_t height,int32_t otherheight,const char *source,int32_t approved) // assetchain context
{
    struct pax_transaction *pax; uint8_t buf[35]; int32_t addflag = 0; struct resistance_state *sp; char str[RESISTANCE_ASSETCHAIN_MAXLEN],dest[RESISTANCE_ASSETCHAIN_MAXLEN],*s;
    //if ( RESISTANCE_PAX == 0 )
    //    return;
    //if ( strcmp(symbol,ASSETCHAINS_SYMBOL) != 0 )
    //    return;
    sp = resistance_stateptr(str,dest);
    pthread_mutex_lock(&resistance_mutex);
    pax_keyset(buf,txid,vout,type);
    HASH_FIND(hh,PAX,buf,sizeof(buf),pax);
    if ( pax == 0 )
    {
        pax = (struct pax_transaction *)calloc(1,sizeof(*pax));
        pax->txid = txid;
        pax->vout = vout;
        pax->type = type;
        memcpy(pax->buf,buf,sizeof(pax->buf));
        HASH_ADD_KEYPTR(hh,PAX,pax->buf,sizeof(pax->buf),pax);
        addflag = 1;
        if ( 0 && ASSETCHAINS_SYMBOL[0] == 0 )
        {
            int32_t i; for (i=0; i<32; i++)
                LogPrintf("%02x",((uint8_t *)&txid)[i]);
            LogPrintf(" v.%d [%s] kht.%d ht.%d create pax.%p symbol.%s source.%s\n",vout,ASSETCHAINS_SYMBOL,height,otherheight,pax,symbol,source);
        }
    }
    pthread_mutex_unlock(&resistance_mutex);
    if ( coinaddr != 0 )
    {
        strcpy(pax->coinaddr,coinaddr);
        if ( value != 0 )
            pax->satoshis = value;
        if ( symbol != 0 )
            strcpy(pax->symbol,symbol);
        if ( source != 0 )
            strcpy(pax->source,source);
        if ( fiatoshis != 0 )
            pax->fiatoshis = fiatoshis;
        if ( rmd160 != 0 )
            memcpy(pax->rmd160,rmd160,20);
        if ( height != 0 )
            pax->height = height;
        if ( otherheight != 0 )
            pax->otherheight = otherheight;
    }
    else
    {
        pax->marked = height;
        //LogPrintf("pax.%p MARK DEPOSIT ht.%d other.%d\n",pax,height,otherheight);
    }
}

int32_t resistance_rwapproval(int32_t rwflag,uint8_t *opretbuf,struct pax_transaction *pax)
{
    int32_t i,len = 0;
    if ( rwflag == 1 )
    {
        for (i=0; i<32; i++)
            opretbuf[len++] = ((uint8_t *)&pax->txid)[i];
        opretbuf[len++] = pax->vout & 0xff;
        opretbuf[len++] = (pax->vout >> 8) & 0xff;
    }
    else
    {
        for (i=0; i<32; i++)
            ((uint8_t *)&pax->txid)[i] = opretbuf[len++];
        //for (i=0; i<32; i++)
        //    LogPrintf("%02x",((uint8_t *)&pax->txid)[31-i]);
        pax->vout = opretbuf[len++];
        pax->vout += ((uint32_t)opretbuf[len++] << 8);
        //LogPrintf(" txid v.%d\n",pax->vout);
    }
    len += iguana_rwnum(rwflag,&opretbuf[len],sizeof(pax->satoshis),&pax->satoshis);
    len += iguana_rwnum(rwflag,&opretbuf[len],sizeof(pax->fiatoshis),&pax->fiatoshis);
    len += iguana_rwnum(rwflag,&opretbuf[len],sizeof(pax->height),&pax->height);
    len += iguana_rwnum(rwflag,&opretbuf[len],sizeof(pax->otherheight),&pax->otherheight);
    if ( rwflag != 0 )
    {
        memcpy(&opretbuf[len],pax->rmd160,20), len += 20;
        for (i=0; i<4; i++)
            opretbuf[len++] = pax->source[i];
    }
    else
    {
        memcpy(pax->rmd160,&opretbuf[len],20), len += 20;
        for (i=0; i<4; i++)
            pax->source[i] = opretbuf[len++];
    }
    return(len);
}

int32_t resistance_issued_opreturn(char *base,uint256 *txids,uint16_t *vouts,int64_t *values,int64_t *srcvalues,int32_t *resheights,int32_t *otherheights,int8_t *baseids,uint8_t *rmd160s,uint8_t *opretbuf,int32_t opretlen,int32_t isresistance)
{
    struct pax_transaction p,*pax; int32_t i,n=0,j,len=0,incr,height,otherheight; uint8_t type,rmd160[20]; uint64_t fiatoshis; char symbol[RESISTANCE_ASSETCHAIN_MAXLEN];
    //if ( RESISTANCE_PAX == 0 )
    //    return(0);
    incr = 34 + (isresistance * (2*sizeof(fiatoshis) + 2*sizeof(height) + 20 + 4));
    //41e77b91cb68dc2aa02fa88550eae6b6d44db676a7e935337b6d1392d9718f03cb0200305c90660400000000fbcbeb1f000000bde801006201000058e7945ad08ddba1eac9c9b6c8e1e97e8016a2d152
    
    // 41e94d736ec69d88c08b5d238abeeca609c02357a8317e0d56c328bcb1c259be5d0200485bc80200000000404b4c000000000059470200b80b000061f22ba7d19fe29ac3baebd839af8b7127d1f9075553440046bb4cc7a3b5cd39dffe7206507a3482a00780e617f68b273cce9817ed69298d02001069ca1b0000000080f0fa02000000005b470200b90b000061f22ba7d19fe29ac3baebd839af8b7127d1f90755
    
    //for (i=0; i<opretlen; i++)
    //    LogPrintf("%02x",opretbuf[i]);
    //LogPrintf(" opretlen.%d (%s)\n",opretlen,base);
    //LogPrintf(" opretlen.%d vs %d incr.%d (%d)\n",opretlen,(int32_t)(2*sizeof(fiatoshis) + 2*sizeof(height) + 20 + 2),incr,opretlen/incr);
    //if ( ASSETCHAINS_SYMBOL[0] == 0 || strncmp(ASSETCHAINS_SYMBOL,base,strlen(base)) == 0 )
    {
        type = opretbuf[0];
        opretbuf++, opretlen--;
        for (n=0; n<opretlen/incr; n++)
        {
            if ( isresistance != 0 )
            {
                memset(&p,0,sizeof(p));
                len += resistance_rwapproval(0,&opretbuf[len],&p);
                if ( values != 0 && srcvalues != 0 && resheights != 0 && otherheights != 0 && baseids != 0 && rmd160s != 0 )
                {
                    txids[n] = p.txid;
                    vouts[n] = p.vout;
                    values[n] = (strcmp("RES",base) == 0) ? p.satoshis : p.fiatoshis;
                    srcvalues[n] = (strcmp("RES",base) == 0) ? p.fiatoshis : p.satoshis;
                    resheights[n] = p.height;
                    otherheights[n] = p.otherheight;
                    memcpy(&rmd160s[n * 20],p.rmd160,20);
                    baseids[n] = resistance_baseid(p.source);
                    if ( 0 )
                    {
                        char coinaddr[64];
                        bitcoin_address(coinaddr,60,&rmd160s[n * 20],20);
                        LogPrintf(">>>>>>> %s: (%s) fiat %.8f resheight.%d other.%d -> %s %.8f\n",type=='A'?"approvedA":"issuedX",baseids[n]>=0?CURRENCIES[baseids[n]]:"???",dstr(p.fiatoshis),resheights[n],otherheights[n],coinaddr,dstr(values[n]));
                    }
                }
            }
            else
            {
                for (i=0; i<4; i++)
                    base[i] = opretbuf[opretlen-4+i];
                for (j=0; j<32; j++)
                {
                    ((uint8_t *)&txids[n])[j] = opretbuf[len++];
                    //LogPrintf("%02x",((uint8_t *)&txids[n])[j]);
                }
                vouts[n] = opretbuf[len++];
                vouts[n] = (opretbuf[len++] << 8) | vouts[n];
                baseids[n] = resistance_baseid(base);
                if ( (pax= resistance_paxfinds(txids[n],vouts[n])) != 0 )
                {
                    values[n] = (strcmp("RES",base) == 0) ? pax->satoshis : pax->fiatoshis;
                    srcvalues[n] = (strcmp("RES",base) == 0) ? pax->fiatoshis : pax->satoshis;
                    resheights[n] = pax->height;
                    otherheights[n] = pax->otherheight;
                    memcpy(&rmd160s[n * 20],pax->rmd160,20);
                }
            }
            //LogPrintf(" resistance_issued_opreturn issuedtxid v%d i.%d opretlen.%d\n",vouts[n],n,opretlen);
        }
    }
    return(n);
}

struct pax_transaction *resistance_paxmark(int32_t height,uint256 txid,uint16_t vout,uint8_t type,int32_t mark)
{
    struct pax_transaction *pax; uint8_t buf[35];
    pthread_mutex_lock(&resistance_mutex);
    pax_keyset(buf,txid,vout,type);
    HASH_FIND(hh,PAX,buf,sizeof(buf),pax);
    if ( pax == 0 )
    {
        pax = (struct pax_transaction *)calloc(1,sizeof(*pax));
        pax->txid = txid;
        pax->vout = vout;
        pax->type = type;
        memcpy(pax->buf,buf,sizeof(pax->buf));
        HASH_ADD_KEYPTR(hh,PAX,pax->buf,sizeof(pax->buf),pax);
        //LogPrintf("ht.%d create pax.%p mark.%d\n",height,pax,mark);
    }
    if ( pax != 0 )
    {
        pax->marked = mark;
        //if ( height > 214700 || pax->height > 214700 )
        //    LogPrintf("mark ht.%d %.8f %.8f\n",pax->height,dstr(pax->satoshis),dstr(pax->fiatoshis));
        
    }
    pthread_mutex_unlock(&resistance_mutex);
    return(pax);
}

void resistance_paxdelete(struct pax_transaction *pax)
{
    return; // breaks when out of order
    pthread_mutex_lock(&resistance_mutex);
    HASH_DELETE(hh,PAX,pax);
    pthread_mutex_unlock(&resistance_mutex);
}

const char *resistance_opreturn(int32_t height,uint64_t value,uint8_t *opretbuf,int32_t opretlen,uint256 txid,uint16_t vout,char *source)
{
    uint8_t rmd160[20],rmd160s[64*20],addrtype,shortflag,pubkey33[33]; int32_t didstats,i,j,n,kvheight,len,toresistance,resheight,otherheights[64],resheights[64]; int8_t baseids[64]; char base[4],coinaddr[64],destaddr[64]; uint256 txids[64]; uint16_t vouts[64]; uint64_t convtoshis,seed; int64_t fee,fiatoshis,satoshis,checktoshis,values[64],srcvalues[64]; struct pax_transaction *pax,*pax2; struct resistance_state *basesp; double diff;
    const char *typestr = "unknown";
    if ( ASSETCHAINS_SYMBOL[0] != 0 && resistance_baseid(ASSETCHAINS_SYMBOL) < 0 && opretbuf[0] != 'K' )
    {
        //LogPrintf("resistance_opreturn skip %s\n",ASSETCHAINS_SYMBOL);
        return("assetchain");
    }
    memset(baseids,0xff,sizeof(baseids));
    memset(values,0,sizeof(values));
    memset(srcvalues,0,sizeof(srcvalues));
    memset(rmd160s,0,sizeof(rmd160s));
    memset(resheights,0,sizeof(resheights));
    memset(otherheights,0,sizeof(otherheights));
    toresistance = (resistance_is_issuer() == 0);
    if ( opretbuf[0] == 'K' && opretlen != 40 )
    {
        resistance_kvupdate(opretbuf,opretlen,value);
        return("kv");
    }
    else if ( ASSETCHAINS_SYMBOL[0] == 0 && RESISTANCE_PAX == 0 )
        return("nopax");
    if ( opretbuf[0] == 'D' )
    {
        toresistance = 0;
        if ( opretlen == 38 ) // any RES tx
        {
            iguana_rwnum(0,&opretbuf[34],sizeof(resheight),&resheight);
            memset(base,0,sizeof(base));
            PAX_pubkey(0,&opretbuf[1],&addrtype,rmd160,base,&shortflag,&fiatoshis);
            bitcoin_address(coinaddr,addrtype,rmd160,20);
            checktoshis = PAX_fiatdest(&seed,toresistance,destaddr,pubkey33,coinaddr,resheight,base,fiatoshis);
            if ( resistance_paxcmp(base,resheight,value,checktoshis,resheight < 225000 ? seed : 0) != 0 )
                checktoshis = PAX_fiatdest(&seed,toresistance,destaddr,pubkey33,coinaddr,height,base,fiatoshis);
            typestr = "deposit";
            if ( 0 && strcmp("NOK",base) == 0 )
            {
                LogPrintf("[%s] %s paxdeposit height.%d vs resheight.%d\n",ASSETCHAINS_SYMBOL,base,height,resheight);
                LogPrintf("(%s) (%s) resheight.%d vs height.%d check %.8f vs %.8f toresistance.%d %d seed.%llx\n",ASSETCHAINS_SYMBOL,base,resheight,height,dstr(checktoshis),dstr(value),resistance_is_issuer(),strncmp(ASSETCHAINS_SYMBOL,base,strlen(base)) == 0,(long long)seed);
                for (i=0; i<32; i++)
                    LogPrintf("%02x",((uint8_t *)&txid)[i]);
                LogPrintf(" <- txid.v%u ",vout);
                for (i=0; i<33; i++)
                    LogPrintf("%02x",pubkey33[i]);
                LogPrintf(" checkpubkey check %.8f v %.8f dest.(%s) resheight.%d height.%d\n",dstr(checktoshis),dstr(value),destaddr,resheight,height);
            }
            if ( strcmp(base,ASSETCHAINS_SYMBOL) == 0 && (resheight > 195000 || resheight <= height) )
            {
                didstats = 0;
                if ( resistance_paxcmp(base,resheight,value,checktoshis,resheight < 225000 ? seed : 0) == 0 )
                {
                    if ( (pax= resistance_paxfind(txid,vout,'D')) == 0 )
                    {
                        if ( (basesp= resistance_stateptrget(base)) != 0 )
                        {
                            basesp->deposited += fiatoshis;
                            didstats = 1;
                            if ( 0 && strcmp(base,ASSETCHAINS_SYMBOL) == 0 )
                                LogPrintf("########### %p deposited %s += %.8f resheight.%d %.8f\n",basesp,base,dstr(fiatoshis),resheight,dstr(value));
                        } else LogPrintf("cant get stateptr.(%s)\n",base);
                        resistance_gateway_deposit(coinaddr,value,base,fiatoshis,rmd160,txid,vout,'D',resheight,height,(char *)"RES",0);
                    }
                    if ( (pax= resistance_paxfind(txid,vout,'D')) != 0 )
                    {
                        pax->height = resheight;
                        pax->validated = value;
                        pax->satoshis = value;
                        pax->fiatoshis = fiatoshis;
                        if ( didstats == 0 && pax->didstats == 0 )
                        {
                            if ( (basesp= resistance_stateptrget(base)) != 0 )
                            {
                                basesp->deposited += fiatoshis;
                                didstats = 1;
                                if ( 0 && strcmp(base,ASSETCHAINS_SYMBOL) == 0 )
                                    LogPrintf("########### %p depositedB %s += %.8f/%.8f resheight.%d/%d %.8f/%.8f\n",basesp,base,dstr(fiatoshis),dstr(pax->fiatoshis),resheight,pax->height,dstr(value),dstr(pax->satoshis));
                            }
                        } //
                        if ( didstats != 0 )
                            pax->didstats = 1;
                        if ( (pax2= resistance_paxfind(txid,vout,'I')) != 0 )
                        {
                            pax2->fiatoshis = pax->fiatoshis;
                            pax2->satoshis = pax->satoshis;
                            pax->marked = pax2->marked = pax->height;
                            pax2->height = pax->height = height;
                            if ( pax2->didstats == 0 )
                            {
                                if ( (basesp= resistance_stateptrget(base)) != 0 )
                                {
                                    basesp->issued += pax2->fiatoshis;
                                    pax2->didstats = 1;
                                    if ( 0 && strcmp(base,"USD") == 0 )
                                        LogPrintf("########### %p issueda %s += %.8f resheight.%d %.8f other.%d [%d]\n",basesp,base,dstr(pax2->fiatoshis),pax2->height,dstr(pax2->satoshis),pax2->otherheight,height);
                                }
                            }
                        }
                    }
                }
                else
                {
                    if ( (pax= resistance_paxfind(txid,vout,'D')) != 0 )
                        pax->marked = checktoshis;
                    if ( resheight > 238000 && (resheight > 214700 || strcmp(base,ASSETCHAINS_SYMBOL) == 0) ) //seed != 0 &&
                        LogPrintf("pax %s deposit %.8f rejected resheight.%d %.8f RES check %.8f seed.%llu\n",base,dstr(fiatoshis),resheight,dstr(value),dstr(checktoshis),(long long)seed);
                }
            } //else LogPrintf("[%s] %s paxdeposit height.%d vs resheight.%d\n",ASSETCHAINS_SYMBOL,base,height,resheight);
        } //else error("unsupported size.%d for opreturn D\n",opretlen);
    }
    else if ( opretbuf[0] == 'I' )
    {
        toresistance = 0;
        if ( strncmp((char *)"RES",(char *)&opretbuf[opretlen-4],3) != 0 && strncmp(ASSETCHAINS_SYMBOL,(char *)&opretbuf[opretlen-4],3) == 0 )
        {
            if ( (n= resistance_issued_opreturn(base,txids,vouts,values,srcvalues,resheights,otherheights,baseids,rmd160s,opretbuf,opretlen,0)) > 0 )
            {
                for (i=0; i<n; i++)
                {
                    if ( baseids[i] < 0 )
                    {
                        static uint32_t counter;
                        if ( counter++ < 0 )
                            LogPrintf("%d of %d illegal baseid.%d, this can be ignored\n",i,n,baseids[i]);
                        continue;
                    }
                    bitcoin_address(coinaddr,60,&rmd160s[i*20],20);
                    resistance_gateway_deposit(coinaddr,0,ASSETCHAINS_SYMBOL,0,0,txids[i],vouts[i],'I',height,0,CURRENCIES[baseids[i]],0);
                    resistance_paxmark(height,txids[i],vouts[i],'I',height);
                    if ( (pax= resistance_paxfind(txids[i],vouts[i],'I')) != 0 )
                    {
                        pax->type = opretbuf[0];
                        strcpy(pax->source,(char *)&opretbuf[opretlen-4]);
                        if ( (pax2= resistance_paxfind(txids[i],vouts[i],'D')) != 0 && pax2->fiatoshis != 0 && pax2->satoshis != 0 )
                        {
                            // realtime path?
                            pax->fiatoshis = pax2->fiatoshis;
                            pax->satoshis = pax2->satoshis;
                            pax->marked = pax2->marked = pax2->height;
                            if ( pax->didstats == 0 )
                            {
                                if ( (basesp= resistance_stateptrget(CURRENCIES[baseids[i]])) != 0 )
                                {
                                    basesp->issued += pax->fiatoshis;
                                    pax->didstats = 1;
                                    pax->height = pax2->height;
                                    pax->otherheight = height;
                                    if ( 1 && strcmp(CURRENCIES[baseids[i]],"USD") == 0 )
                                        LogPrintf("########### %p issuedb %s += %.8f resheight.%d %.8f other.%d [%d]\n",basesp,CURRENCIES[baseids[i]],dstr(pax->fiatoshis),pax->height,dstr(pax->satoshis),pax->otherheight,height);
                                }
                            }
                        }
                    }
                    if ( (pax= resistance_paxmark(height,txids[i],vouts[i],'I',height)) != 0 )
                        resistance_paxdelete(pax);
                    if ( (pax= resistance_paxmark(height,txids[i],vouts[i],'D',height)) != 0 )
                        resistance_paxdelete(pax);
                }
            } //else LogPrintf("opreturn none issued?\n");
        }
    }
    else if ( height < 236000 && opretbuf[0] == 'W' && strncmp(ASSETCHAINS_SYMBOL,(char *)&opretbuf[opretlen-4],3) == 0 )//&& opretlen >= 38 )
    {
        if ( resistance_baseid((char *)&opretbuf[opretlen-4]) >= 0 && strcmp(ASSETCHAINS_SYMBOL,(char *)&opretbuf[opretlen-4]) == 0 )
        {
            for (i=0; i<opretlen; i++)
                LogPrintf("%02x",opretbuf[i]);
            LogPrintf(" [%s] reject obsolete withdraw request.%s\n",ASSETCHAINS_SYMBOL,(char *)&opretbuf[opretlen-4]);
            return(typestr);
        }
        toresistance = 1;
        iguana_rwnum(0,&opretbuf[34],sizeof(resheight),&resheight);
        memset(base,0,sizeof(base));
        PAX_pubkey(0,&opretbuf[1],&addrtype,rmd160,base,&shortflag,&satoshis);
        bitcoin_address(coinaddr,addrtype,rmd160,20);
        checktoshis = PAX_fiatdest(&seed,toresistance,destaddr,pubkey33,coinaddr,resheight,base,value);
        typestr = "withdraw";
        //LogPrintf(" [%s] WITHDRAW %s.height.%d vs height.%d check %.8f/%.8f vs %.8f toresistance.%d %d seed.%llx -> (%s) len.%d\n",ASSETCHAINS_SYMBOL,base,resheight,height,dstr(checktoshis),dstr(satoshis),dstr(value),resistance_is_issuer(),strncmp(ASSETCHAINS_SYMBOL,base,strlen(base)) == 0,(long long)seed,coinaddr,opretlen);
        didstats = 0;
        //if ( resistance_paxcmp(base,resheight,satoshis,checktoshis,seed) == 0 )
        {
            if ( value != 0 && ((pax= resistance_paxfind(txid,vout,'W')) == 0 || pax->didstats == 0) )
            {
                if ( (basesp= resistance_stateptrget(base)) != 0 )
                {
                    basesp->withdrawn += value;
                    didstats = 1;
                    if ( 0 && strcmp(base,ASSETCHAINS_SYMBOL) == 0 )
                        LogPrintf("########### %p withdrawn %s += %.8f check %.8f\n",basesp,base,dstr(value),dstr(checktoshis));
                }
                if ( 0 && strcmp(base,"RUB") == 0 && (pax == 0 || pax->approved == 0) )
                    LogPrintf("notarize %s %.8f -> %.8f res.%d other.%d\n",ASSETCHAINS_SYMBOL,dstr(value),dstr(satoshis),resheight,height);
            }
            resistance_gateway_deposit(coinaddr,0,(char *)"RES",value,rmd160,txid,vout,'W',resheight,height,source,0);
            if ( (pax= resistance_paxfind(txid,vout,'W')) != 0 )
            {
                pax->type = opretbuf[0];
                strcpy(pax->source,base);
                strcpy(pax->symbol,"RES");
                pax->height = resheight;
                pax->otherheight = height;
                pax->satoshis = satoshis;
            }
        } // else LogPrintf("withdraw %s paxcmp ht.%d %d error value %.8f -> %.8f vs %.8f\n",base,resheight,height,dstr(value),dstr(satoshis),dstr(checktoshis));
        // need to allocate pax
    }
    else if ( height < 236000 && toresistance != 0 && opretbuf[0] == 'A' && ASSETCHAINS_SYMBOL[0] == 0 )
    {
        toresistance = 1;
        if ( 0 && ASSETCHAINS_SYMBOL[0] != 0 )
        {
            for (i=0; i<opretlen; i++)
                LogPrintf("%02x",opretbuf[i]);
            LogPrintf(" opret[%c] else path toresistance.%d ht.%d before %.8f opretlen.%d\n",opretbuf[0],toresistance,height,dstr(resistance_paxtotal()),opretlen);
        }
        if ( (n= resistance_issued_opreturn(base,txids,vouts,values,srcvalues,resheights,otherheights,baseids,rmd160s,opretbuf,opretlen,1)) > 0 )
        {
            for (i=0; i<n; i++)
            {
                //for (j=0; j<32; j++)
                //    LogPrintf("%02x",((uint8_t *)&txids[i])[j]);
                //LogPrintf(" v%d %.8f %.8f k.%d ht.%d base.%d\n",vouts[i],dstr(values[i]),dstr(srcvalues[i]),resheights[i],otherheights[i],baseids[i]);
                if ( baseids[i] < 0 )
                {
                    for (i=0; i<opretlen; i++)
                        LogPrintf("%02x",opretbuf[i]);
                    LogPrintf(" opret[%c] else path toresistance.%d ht.%d before %.8f opretlen.%d\n",opretbuf[0],toresistance,height,dstr(resistance_paxtotal()),opretlen);
                    //LogPrintf("baseids[%d] %d\n",i,baseids[i]);
                    if ( (pax= resistance_paxfind(txids[i],vouts[i],'W')) != 0 || (pax= resistance_paxfind(txids[i],vouts[i],'X')) != 0 )
                    {
                        baseids[i] = resistance_baseid(pax->symbol);
                        LogPrintf("override neg1 with (%s)\n",pax->symbol);
                    }
                    if ( baseids[i] < 0 )
                        continue;
                }
                didstats = 0;
                seed = 0;
                checktoshis = resistance_paxprice(&seed,resheights[i],CURRENCIES[baseids[i]],(char *)"RES",(uint64_t)values[i]);
                //LogPrintf("PAX_fiatdest ht.%d price %s %.8f -> RES %.8f vs %.8f\n",resheights[i],CURRENCIES[baseids[i]],(double)values[i]/COIN,(double)srcvalues[i]/COIN,(double)checktoshis/COIN);
                if ( srcvalues[i] == checktoshis )
                {
                    if ( (pax= resistance_paxfind(txids[i],vouts[i],'A')) == 0 )
                    {
                        bitcoin_address(coinaddr,60,&rmd160s[i*20],20);
                        resistance_gateway_deposit(coinaddr,srcvalues[i],CURRENCIES[baseids[i]],values[i],&rmd160s[i*20],txids[i],vouts[i],'A',resheights[i],otherheights[i],CURRENCIES[baseids[i]],resheights[i]);
                        if ( (pax= resistance_paxfind(txids[i],vouts[i],'A')) == 0 )
                            LogPrintf("unexpected null pax for approve\n");
                        else pax->validated = checktoshis;
                        if ( (pax2= resistance_paxfind(txids[i],vouts[i],'W')) != 0 )
                            pax2->approved = resheights[i];
                        resistance_paxmark(height,txids[i],vouts[i],'W',height);
                        //resistance_paxmark(height,txids[i],vouts[i],'A',height);
                        if ( values[i] != 0 && (basesp= resistance_stateptrget(CURRENCIES[baseids[i]])) != 0 )
                        {
                            basesp->approved += values[i];
                            didstats = 1;
                            //LogPrintf("pax.%p ########### %p approved %s += %.8f -> %.8f/%.8f kht.%d %d\n",pax,basesp,CURRENCIES[baseids[i]],dstr(values[i]),dstr(srcvalues[i]),dstr(checktoshis),resheights[i],otherheights[i]);
                        }
                        //LogPrintf(" i.%d (%s) <- %.8f ADDFLAG APPROVED\n",i,coinaddr,dstr(values[i]));
                    }
                    else if ( pax->didstats == 0 && srcvalues[i] != 0 )
                    {
                        if ( (basesp= resistance_stateptrget(CURRENCIES[baseids[i]])) != 0 )
                        {
                            basesp->approved += values[i];
                            didstats = 1;
                            //LogPrintf("pax.%p ########### %p approved %s += %.8f -> %.8f/%.8f kht.%d %d\n",pax,basesp,CURRENCIES[baseids[i]],dstr(values[i]),dstr(srcvalues[i]),dstr(checktoshis),resheights[i],otherheights[i]);
                        }
                    } //else LogPrintf(" i.%d of n.%d pax.%p baseids[] %d\n",i,n,pax,baseids[i]);
                    if ( (pax= resistance_paxfind(txids[i],vouts[i],'A')) != 0 )
                    {
                        pax->type = opretbuf[0];
                        pax->approved = resheights[i];
                        pax->validated = checktoshis;
                        if ( didstats != 0 )
                            pax->didstats = 1;
                        //if ( strcmp(CURRENCIES[baseids[i]],ASSETCHAINS_SYMBOL) == 0 )
                        //LogPrintf(" i.%d approved.%d <<<<<<<<<<<<< APPROVED %p\n",i,resheights[i],pax);
                    }
                }
            }
        } //else LogPrintf("n.%d from opreturns\n",n);
        //LogPrintf("extra.[%d] after %.8f\n",n,dstr(resistance_paxtotal()));
    }
    else if ( height < 236000 && opretbuf[0] == 'X' && ASSETCHAINS_SYMBOL[0] == 0 )
    {
        toresistance = 1;
        if ( (n= resistance_issued_opreturn(base,txids,vouts,values,srcvalues,resheights,otherheights,baseids,rmd160s,opretbuf,opretlen,1)) > 0 )
        {
            for (i=0; i<n; i++)
            {
                if ( baseids[i] < 0 )
                    continue;
                bitcoin_address(coinaddr,60,&rmd160s[i*20],20);
                resistance_gateway_deposit(coinaddr,0,0,0,0,txids[i],vouts[i],'X',height,0,(char *)"RES",0);
                resistance_paxmark(height,txids[i],vouts[i],'W',height);
                resistance_paxmark(height,txids[i],vouts[i],'A',height);
                resistance_paxmark(height,txids[i],vouts[i],'X',height);
                if ( (pax= resistance_paxfind(txids[i],vouts[i],'X')) != 0 )
                {
                    pax->type = opretbuf[0];
                    if ( height < 121842 ) // fields got switched around due to legacy issues and approves
                        value = srcvalues[i];
                    else value = values[i];
                    if ( baseids[i] >= 0 && value != 0 && (basesp= resistance_stateptrget(CURRENCIES[baseids[i]])) != 0 )
                    {
                        basesp->redeemed += value;
                        pax->didstats = 1;
                        if ( strcmp(CURRENCIES[baseids[i]],ASSETCHAINS_SYMBOL) == 0 )
                            LogPrintf("ht.%d %.8f ########### %p redeemed %s += %.8f %.8f kht.%d ht.%d\n",height,dstr(value),basesp,CURRENCIES[baseids[i]],dstr(value),dstr(srcvalues[i]),resheights[i],otherheights[i]);
                    }
                }
                if ( (pax= resistance_paxmark(height,txids[i],vouts[i],'W',height)) != 0 )
                    resistance_paxdelete(pax);
                if ( (pax= resistance_paxmark(height,txids[i],vouts[i],'A',height)) != 0 )
                    resistance_paxdelete(pax);
                if ( (pax= resistance_paxmark(height,txids[i],vouts[i],'X',height)) != 0 )
                    resistance_paxdelete(pax);
            }
        } //else LogPrintf("resistance_issued_opreturn returned %d\n",n);
    }
    return(typestr);
}

long resistance_indfile_update(FILE *indfp,uint32_t *prevpos100p,long lastfpos,long newfpos,uint8_t func,uint32_t *indcounterp)
{
    uint32_t tmp;
    if ( indfp != 0 )
    {
        tmp = ((uint32_t)(newfpos - *prevpos100p) << 8) | (func & 0xff);
        if ( ftell(indfp)/sizeof(uint32_t) != *indcounterp )
            LogPrintf("indfp fpos %ld -> ind.%ld vs counter.%u\n",ftell(indfp),ftell(indfp)/sizeof(uint32_t),*indcounterp);
        //LogPrintf("ftell.%ld indcounter.%u lastfpos.%ld newfpos.%ld func.%02x\n",ftell(indfp),*indcounterp,lastfpos,newfpos,func);
        fwrite(&tmp,1,sizeof(tmp),indfp), (*indcounterp)++;
        if ( (*indcounterp % 100) == 0 )
        {
            *prevpos100p = (uint32_t)newfpos;
            fwrite(prevpos100p,1,sizeof(*prevpos100p),indfp), (*indcounterp)++;
        }
    }
    return(newfpos);
}

void resistance_stateind_set(struct resistance_state *sp,uint32_t *inds,int32_t n,uint8_t *filedata,long datalen,char *symbol,char *dest)
{
    uint8_t func; long lastK,lastT,lastN,lastV,fpos,lastfpos; int32_t i,count,doissue,iter,numn,numv,numN,numV,numR; uint32_t tmp,prevpos100,offset;
    count = numR = numN = numV = numn = numv = 0;
    lastK = lastT = lastN = lastV = -1;
    for (iter=0; iter<2; iter++)
    {
        for (lastfpos=fpos=prevpos100=i=0; i<n; i++)
        {
            tmp = inds[i];
            if ( (i % 100) == 0 )
                prevpos100 = tmp;
            else
            {
                func = (tmp & 0xff);
                offset = (tmp >> 8);
                fpos = prevpos100 + offset;
                if ( lastfpos >= datalen || (filedata[lastfpos] != func && func != 0) )
                    LogPrintf("i.%d/n.%d lastfpos.%ld >= datalen.%ld or [%d] != func.%d\n",i,n,lastfpos,datalen,filedata[lastfpos],func);
                else if ( iter == 0 )
                {
                    switch ( func )
                    {
                        default: case 'P': case 'U': case 'D':
                            inds[i] &= 0xffffff00;
                            break;
                        case 'K':
                            lastK = lastfpos;
                            inds[i] &= 0xffffff00;
                            break;
                        case 'T':
                            lastT = lastfpos;
                            inds[i] &= 0xffffff00;
                            break;
                        case 'N':
                            lastN = lastfpos;
                            numN++;
                            break;
                        case 'V':
                            lastV = lastfpos;
                            numV++;
                            break;
                        case 'R':
                            numR++;
                            break;
                    }
                }
                else
                {
                    doissue = 0;
                    if ( func == 'K' )
                    {
                        if ( lastK == lastfpos )
                            doissue = 1;
                    }
                    else if ( func == 'T' )
                    {
                        if ( lastT == lastfpos )
                            doissue = 1;
                    }
                    else if ( func == 'N' )
                    {
                        if ( numn > numN-128 )
                            doissue = 1;
                        numn++;
                    }
                    else if ( func == 'V' )
                    {
                        if ( RESISTANCE_PAX != 0 && numv > numV-1440 )
                            doissue = 1;
                        numv++;
                    }
                    else if ( func == 'R' )
                        doissue = 1;
                    if ( doissue != 0 )
                    {
                        //LogPrintf("issue %c total.%d lastfpos.%ld\n",func,count,lastfpos);
                        resistance_parsestatefiledata(sp,filedata,&lastfpos,datalen,symbol,dest);
                        count++;
                    }
                }
            }
            lastfpos = fpos;
        }
    }
    LogPrintf("numR.%d numV.%d numN.%d count.%d\n",numR,numV,numN,count);
    /*else if ( func == 'K' ) // RES height: stop after 1st
     else if ( func == 'T' ) // RES height+timestamp: stop after 1st
     
     else if ( func == 'N' ) // notarization, scan backwards 1440+ blocks;
     else if ( func == 'V' ) // price feed: can stop after 1440+
     else if ( func == 'R' ) // opreturn:*/
}

long resistance_stateind_validate(struct resistance_state *sp,char *indfname,uint8_t *filedata,long datalen,uint32_t *prevpos100p,uint32_t *indcounterp,char *symbol,char *dest)
{
    FILE *fp; long fsize,lastfpos=0,fpos=0; uint8_t *inds,func; int32_t i,n; uint32_t offset,tmp,prevpos100 = 0;
    *indcounterp = *prevpos100p = 0;
    if ( (inds= OS_fileptr(&fsize,indfname)) != 0 )
    {
        lastfpos = 0;
        LogPrintf("inds.%p validate %s fsize.%ld datalen.%ld n.%ld lastfpos.%ld\n",inds,indfname,fsize,datalen,fsize / sizeof(uint32_t),lastfpos);
        if ( (fsize % sizeof(uint32_t)) == 0 )
        {
            n = (int32_t)(fsize / sizeof(uint32_t));
            for (i=0; i<n; i++)
            {
                memcpy(&tmp,&inds[i * sizeof(uint32_t)],sizeof(uint32_t));
                if ( 0 && i > n-10 )
                    LogPrintf("%d: tmp.%08x [%c] prevpos100.%u\n",i,tmp,tmp&0xff,prevpos100);
                if ( (i % 100) == 0 )
                    prevpos100 = tmp;
                else
                {
                    func = (tmp & 0xff);
                    offset = (tmp >> 8);
                    fpos = prevpos100 + offset;
                    if ( lastfpos >= datalen || filedata[lastfpos] != func )
                    {
                        LogPrintf("validate.%d error (%u %d) prev100 %u -> fpos.%ld datalen.%ld [%d] (%c) vs (%c) lastfpos.%ld\n",i,offset,func,prevpos100,fpos,datalen,lastfpos < datalen ? filedata[lastfpos] : -1,func,filedata[lastfpos],lastfpos);
                        return(-1);
                    }
                }
                lastfpos = fpos;
            }
            *indcounterp = n;
            *prevpos100p = prevpos100;
            if ( sp != 0 )
                resistance_stateind_set(sp,(uint32_t *)inds,n,filedata,fpos,symbol,dest);
            //LogPrintf("free inds.%p %s validated[%d] fpos.%ld datalen.%ld, offset %ld vs fsize.%ld\n",inds,indfname,i,fpos,datalen,i * sizeof(uint32_t),fsize);
            free(inds);
            return(fpos);
        } else error("wrong filesize %s %ld\n",indfname,fsize);
    }
    free(inds);
    error("indvalidate return -1\n");
    return(-1);
}

int32_t resistance_faststateinit(struct resistance_state *sp,char *fname,char *symbol,char *dest)
{
    FILE *indfp; char indfname[1024]; uint8_t *filedata; long validated=-1,datalen,fpos,lastfpos; uint32_t tmp,prevpos100,indcounter,starttime; int32_t func,finished = 0;
    starttime = (uint32_t)time(NULL);
    safecopy(indfname,fname,sizeof(indfname)-4);
    strcat(indfname,".ind");
    if ( (filedata= OS_fileptr(&datalen,fname)) != 0 )
    {
        if ( 1 )//datalen >= (1LL << 32) || GetArg("-genind",0) != 0 || (validated= resistance_stateind_validate(0,indfname,filedata,datalen,&prevpos100,&indcounter,symbol,dest)) < 0 )
        {
            lastfpos = fpos = 0;
            indcounter = prevpos100 = 0;
            if ( (indfp= fopen(indfname,"wb")) != 0 )
                fwrite(&prevpos100,1,sizeof(prevpos100),indfp), indcounter++;
            LogPrintf("processing %s %ldKB, validated.%ld\n",fname,datalen/1024,validated);
            while ( (func= resistance_parsestatefiledata(sp,filedata,&fpos,datalen,symbol,dest)) >= 0 )
            {
                lastfpos = resistance_indfile_update(indfp,&prevpos100,lastfpos,fpos,func,&indcounter);
            }
            if ( indfp != 0 )
            {
                fclose(indfp);
                if ( (fpos= resistance_stateind_validate(0,indfname,filedata,datalen,&prevpos100,&indcounter,symbol,dest)) < 0 )
                    LogPrintf("unexpected resistancestate.ind validate failure %s datalen.%ld\n",indfname,datalen);
                else LogPrintf("%s validated fpos.%ld\n",indfname,fpos);
            }
            finished = 1;
            LogPrintf("took %d seconds to process %s %ldKB\n",(int32_t)(time(NULL)-starttime),fname,datalen/1024);
        }
        else if ( validated > 0 )
        {
            if ( (indfp= fopen(indfname,"rb+")) != 0 )
            {
                lastfpos = fpos = validated;
                LogPrintf("datalen.%ld validated %ld -> indcounter %u, prevpos100 %u offset.%ld\n",datalen,validated,indcounter,prevpos100,indcounter * sizeof(uint32_t));
                if ( fpos < datalen )
                {
                    fseek(indfp,indcounter * sizeof(uint32_t),SEEK_SET);
                    if ( ftell(indfp) == indcounter * sizeof(uint32_t) )
                    {
                        while ( (func= resistance_parsestatefiledata(sp,filedata,&fpos,datalen,symbol,dest)) >= 0 )
                        {
                            lastfpos = resistance_indfile_update(indfp,&prevpos100,lastfpos,fpos,func,&indcounter);
                            if ( lastfpos != fpos )
                                LogPrintf("unexpected lastfpos.%ld != %ld\n",lastfpos,fpos);
                        }
                    }
                    fclose(indfp);
                }
                if ( resistance_stateind_validate(sp,indfname,filedata,datalen,&prevpos100,&indcounter,symbol,dest) < 0 )
                    LogPrintf("unexpected resistancestate.ind validate failure %s datalen.%ld\n",indfname,datalen);
                else
                {
                    LogPrintf("%s validated updated from validated.%ld to %ld new.[%ld] -> indcounter %u, prevpos100 %u offset.%ld | elapsed %d seconds\n",indfname,validated,fpos,fpos-validated,indcounter,prevpos100,indcounter * sizeof(uint32_t),(int32_t)(time(NULL) - starttime));
                    finished = 1;
                }
            }
        } else error("resistance_faststateinit unexpected case\n");
        free(filedata);
        return(finished == 1);
    }
    return(-1);
}
