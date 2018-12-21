
#ifndef komodo__h
#define komodo__h

#include "komodo_defs.h"
#include "komodo_globals.h"
#include "komodo_utils.h"
#include "komodo_gateway.h"
#include "komodo_events.h"
#include "main.h"

void komodo_stateupdate(int32_t height,uint8_t notarypubs[][33],uint8_t numnotaries,uint8_t notaryid,uint256 txhash,uint64_t voutmask,uint8_t numvouts,uint32_t *pvals,uint8_t numpvals,int32_t KMDheight,uint32_t KMDtimestamp,uint64_t opretvalue,uint8_t *opretbuf,uint16_t opretlen,uint16_t vout)//,uint256 MoM,int32_t MoMdepth)
{
    static FILE *fp; static int32_t errs,didinit; static uint256 zero;
    struct komodo_state *sp; char fname[512],symbol[KOMODO_ASSETCHAIN_MAXLEN],dest[KOMODO_ASSETCHAIN_MAXLEN]; int32_t retval,ht,func; uint8_t num,pubkeys[64][33];
    if ( didinit == 0 )
    {
        portable_mutex_init(&KOMODO_KV_mutex);
        didinit = 1;
    }
    if ( (sp= komodo_stateptr(symbol,dest)) == 0 )
    {
        KOMODO_INITDONE = (uint32_t)time(NULL);
        LogPrintf("[%s] no komodo_stateptr\n",ASSETCHAINS_SYMBOL);
        return;
    }
    //LogPrintf("[%s] (%s) -> (%s)\n",ASSETCHAINS_SYMBOL,symbol,dest);
    if ( fp == 0 )
    {
        komodo_statefname(fname,ASSETCHAINS_SYMBOL,(char *)"komodostate");
        if ( (fp= fopen(fname,"rb+")) != 0 )
        {
            if ( (retval= komodo_faststateinit(sp,fname,symbol,dest)) > 0 )
                fseek(fp,0,SEEK_END);
            else
            {
                LogPrintf("komodo_faststateinit retval.%d\n",retval);
                while ( komodo_parsestatefile(sp,fp,symbol,dest) >= 0 )
                    ;
            }
        } else fp = fopen(fname,"wb+");
        KOMODO_INITDONE = (uint32_t)time(NULL);
    }
    if ( height <= 0 )
    {
        //LogPrintf("early return: stateupdate height.%d\n",height);
        return;
    }
    if ( fp != 0 ) // write out funcid, height, other fields, call side effect function
    {
        //LogPrintf("fpos.%ld ",ftell(fp));
        if ( KMDheight != 0 )
        {
            if ( KMDtimestamp != 0 )
            {
                fputc('T',fp);
                if ( fwrite(&height,1,sizeof(height),fp) != sizeof(height) )
                    errs++;
                if ( fwrite(&KMDheight,1,sizeof(KMDheight),fp) != sizeof(KMDheight) )
                    errs++;
                if ( fwrite(&KMDtimestamp,1,sizeof(KMDtimestamp),fp) != sizeof(KMDtimestamp) )
                    errs++;
            }
            else
            {
                fputc('K',fp);
                if ( fwrite(&height,1,sizeof(height),fp) != sizeof(height) )
                    errs++;
                if ( fwrite(&KMDheight,1,sizeof(KMDheight),fp) != sizeof(KMDheight) )
                    errs++;
            }
            komodo_eventadd_kmdheight(sp,symbol,height,KMDheight,KMDtimestamp);
        }
        else if ( opretbuf != 0 && opretlen > 0 )
        {
            uint16_t olen = opretlen;
            fputc('R',fp);
            if ( fwrite(&height,1,sizeof(height),fp) != sizeof(height) )
                errs++;
            if ( fwrite(&txhash,1,sizeof(txhash),fp) != sizeof(txhash) )
                errs++;
            if ( fwrite(&vout,1,sizeof(vout),fp) != sizeof(vout) )
                errs++;
            if ( fwrite(&opretvalue,1,sizeof(opretvalue),fp) != sizeof(opretvalue) )
                errs++;
            if ( fwrite(&olen,1,sizeof(olen),fp) != olen )
                errs++;
            if ( fwrite(opretbuf,1,olen,fp) != olen )
                errs++;
            //LogPrintf("ht.%d R opret[%d] sp.%p\n",height,olen,sp);
            //komodo_opreturn(height,opretvalue,opretbuf,olen,txhash,vout);
            komodo_eventadd_opreturn(sp,symbol,height,txhash,opretvalue,vout,opretbuf,olen);
        }
        else if ( notarypubs != 0 && numnotaries > 0 )
        {
            LogPrintf("ht.%d func P[%d] errs.%d\n",height,numnotaries,errs);
            fputc('P',fp);
            if ( fwrite(&height,1,sizeof(height),fp) != sizeof(height) )
                errs++;
            fputc(numnotaries,fp);
            if ( fwrite(notarypubs,33,numnotaries,fp) != numnotaries )
                errs++;
            komodo_eventadd_pubkeys(sp,symbol,height,numnotaries,notarypubs);
        }
        else if ( voutmask != 0 && numvouts > 0 )
        {
            //LogPrintf("ht.%d func U %d %d errs.%d hashsize.%ld\n",height,numvouts,notaryid,errs,sizeof(txhash));
            fputc('U',fp);
            if ( fwrite(&height,1,sizeof(height),fp) != sizeof(height) )
                errs++;
            fputc(numvouts,fp);
            fputc(notaryid,fp);
            if ( fwrite(&voutmask,1,sizeof(voutmask),fp) != sizeof(voutmask) )
                errs++;
            if ( fwrite(&txhash,1,sizeof(txhash),fp) != sizeof(txhash) )
                errs++;
            //komodo_eventadd_utxo(sp,symbol,height,notaryid,txhash,voutmask,numvouts);
        }
        else if ( pvals != 0 && numpvals > 0 )
        {
            int32_t i,nonz = 0;
            for (i=0; i<32; i++)
                if ( pvals[i] != 0 )
                    nonz++;
            if ( nonz >= 32 )
            {
                fputc('V',fp);
                if ( fwrite(&height,1,sizeof(height),fp) != sizeof(height) )
                    errs++;
                fputc(numpvals,fp);
                if ( fwrite(pvals,sizeof(uint32_t),numpvals,fp) != numpvals )
                    errs++;
                komodo_eventadd_pricefeed(sp,symbol,height,pvals,numpvals);
                //LogPrintf("ht.%d V numpvals[%d]\n",height,numpvals);
            }
            //LogPrintf("save pvals height.%d numpvals.%d\n",height,numpvals);
        }
        else if ( height != 0 )
        {
            //LogPrintf("ht.%d func N ht.%d errs.%d\n",height,NOTARIZED_HEIGHT,errs);
            if ( sp != 0 )
            {
                //if ( sp->MoMdepth > 0 && sp->MoM != zero )
                //    fputc('M',fp);
                //else
                fputc('N',fp);
                if ( fwrite(&height,1,sizeof(height),fp) != sizeof(height) )
                    errs++;
                if ( fwrite(&sp->NOTARIZED_HEIGHT,1,sizeof(sp->NOTARIZED_HEIGHT),fp) != sizeof(sp->NOTARIZED_HEIGHT) )
                    errs++;
                if ( fwrite(&sp->NOTARIZED_HASH,1,sizeof(sp->NOTARIZED_HASH),fp) != sizeof(sp->NOTARIZED_HASH) )
                    errs++;
                if ( fwrite(&sp->NOTARIZED_DESTTXID,1,sizeof(sp->NOTARIZED_DESTTXID),fp) != sizeof(sp->NOTARIZED_DESTTXID) )
                    errs++;
                /*if ( sp->MoMdepth > 0 && sp->MoM != zero )
                {
                    if ( fwrite(&sp->MoM,1,sizeof(sp->MoM),fp) != sizeof(sp->MoM) )
                        errs++;
                    if ( fwrite(&sp->MoMdepth,1,sizeof(sp->MoMdepth),fp) != sizeof(sp->MoMdepth) )
                        errs++;
                }*/
                komodo_eventadd_notarized(sp,symbol,height,dest,sp->NOTARIZED_HASH,sp->NOTARIZED_DESTTXID,sp->NOTARIZED_HEIGHT);//,sp->MoM,sp->MoMdepth);
            }
        }
        fflush(fp);
    }
}

static int32_t gettxout_scriptPubKey(uint8_t *scriptPubKey,int32_t maxsize,uint256 txid,int32_t n)
{
    int32_t i,m; uint8_t *ptr;
    LOCK(cs_main);
    CTransaction tx;
    uint256 hashBlock;
    if ( GetTransaction(txid,tx,hashBlock,true) == 0 )
        return(-1);
    else if ( n <= tx.vout.size() ) // vout.size() seems off by 1
    {
        ptr = ParseHex(HexStr(tx.vout[n].scriptPubKey)).data();
        m = tx.vout[n].scriptPubKey.size();
        for (i=0; i<maxsize&&i<m; i++)
            scriptPubKey[i] = ptr[i];
        //LogPrintf("got scriptPubKey via rawtransaction\n");
        return(i);
    }
    return(-1);
}

static int32_t komodo_notarycmp(uint8_t *scriptPubKey,int32_t scriptlen,uint8_t pubkeys[64][33],int32_t numnotaries,uint8_t rmd160[20])
{
    int32_t i;
    if ( scriptlen == 25 && memcmp(&scriptPubKey[3],rmd160,20) == 0 )
        return(0);
    else if ( scriptlen == 35 )
    {
        for (i=0; i<numnotaries; i++)
            if ( memcmp(&scriptPubKey[1],pubkeys[i],33) == 0 )
                return(i);
    }
    return(-1);
}

static void komodo_currentheight_set(int32_t height)
{
    char symbol[KOMODO_ASSETCHAIN_MAXLEN],dest[KOMODO_ASSETCHAIN_MAXLEN]; struct komodo_state *sp;
    if ( (sp= komodo_stateptr(symbol,dest)) != 0 )
        sp->CURRENT_HEIGHT = height;
}

static 

int32_t komodo_voutupdate(int32_t *isratificationp,int32_t notaryid,uint8_t *scriptbuf,int32_t scriptlen,int32_t height,uint256 txhash,int32_t i,int32_t j,uint64_t *voutmaskp,int32_t *specialtxp,int32_t *notarizedheightp,uint64_t value,int32_t notarized,uint64_t signedmask,uint32_t timestamp)
{
    static uint256 zero; static FILE *signedfp;
    int32_t opretlen,nid,k,len = 0; uint256 srchash,desttxid; uint8_t crypto777[33]; struct komodo_state *sp; char symbol[KOMODO_ASSETCHAIN_MAXLEN],dest[KOMODO_ASSETCHAIN_MAXLEN];
    if ( (sp= komodo_stateptr(symbol,dest)) == 0 )
        return(-1);
    if ( scriptlen == 35 && scriptbuf[0] == 33 && scriptbuf[34] == 0xac )
    {
        if ( i == 0 && j == 0 && memcmp(NOTARY_PUBKEY33,scriptbuf+1,33) == 0 && NOTARY_PUBKEY33[0] != 0 )
        {
            LogPrintf("%s KOMODO_LASTMINED.%d -> %d\n",ASSETCHAINS_SYMBOL,KOMODO_LASTMINED,height);
            prevKOMODO_LASTMINED = KOMODO_LASTMINED;
            KOMODO_LASTMINED = height;
        }
        decode_hex(crypto777,33,(char *)CRYPTO777_PUBSECPSTR);
        /*for (k=0; k<33; k++)
            LogPrintf("%02x",crypto777[k]);
        LogPrintf(" crypto777 ");
        for (k=0; k<scriptlen; k++)
            LogPrintf("%02x",scriptbuf[k]);
        LogPrintf(" <- script ht.%d i.%d j.%d cmp.%d\n",height,i,j,memcmp(crypto777,scriptbuf+1,33));*/
        if ( memcmp(crypto777,scriptbuf+1,33) == 0 )
        {
            *specialtxp = 1;
            //LogPrintf(">>>>>>>> ");
        }
        else if ( komodo_chosennotary(&nid,height,scriptbuf + 1,timestamp) >= 0 )
        {
            //LogPrintf("found notary.k%d\n",k);
            if ( notaryid < 64 )
            {
                if ( notaryid < 0 )
                {
                    notaryid = nid;
                    *voutmaskp |= (1LL << j);
                }
                else if ( notaryid != nid )
                {
                    //for (i=0; i<33; i++)
                    //    LogPrintf("%02x",scriptbuf[i+1]);
                    //LogPrintf(" %s mismatch notaryid.%d k.%d\n",ASSETCHAINS_SYMBOL,notaryid,nid);
                    notaryid = 64;
                    *voutmaskp = 0;
                }
                else *voutmaskp |= (1LL << j);
            }
        }
    }
    if ( scriptbuf[len++] == 0x6a )
    {
        if ( (opretlen= scriptbuf[len++]) == 0x4c )
            opretlen = scriptbuf[len++];
        else if ( opretlen == 0x4d )
        {
            opretlen = scriptbuf[len++];
            opretlen += (scriptbuf[len++] << 8);
        }
        if ( 0 && ASSETCHAINS_SYMBOL[0] != 0 )
            LogPrintf("[%s] notarized.%d notarizedht.%d sp.Nht %d sp.ht %d opretlen.%d (%c %c %c)\n",ASSETCHAINS_SYMBOL,notarized,*notarizedheightp,sp->NOTARIZED_HEIGHT,sp->CURRENT_HEIGHT,opretlen,scriptbuf[len+32*2+4],scriptbuf[len+32*2+4+1],scriptbuf[len+32*2+4+2]);
        if ( j == 1 && opretlen >= 32*2+4 && strcmp(ASSETCHAINS_SYMBOL[0]==0?"RES":ASSETCHAINS_SYMBOL,(char *)&scriptbuf[len+32*2+4]) == 0 )
        {
            len += iguana_rwbignum(0,&scriptbuf[len],32,(uint8_t *)&srchash);
            len += iguana_rwnum(0,&scriptbuf[len],sizeof(*notarizedheightp),(uint8_t *)notarizedheightp);
            len += iguana_rwbignum(0,&scriptbuf[len],32,(uint8_t *)&desttxid);
            if ( strcmp("PIZZA",ASSETCHAINS_SYMBOL) == 0 && opretlen == 110 )
            {
                notarized = 1;
            }
            static int32_t last_rewind;
            int32_t rewindtarget,validated = 0;
            CBlockIndex *pindex;//
            if ( IsInitialBlockDownload() == 0 && ((pindex= mapBlockIndex[srchash]) == 0 || pindex->nHeight != *notarizedheightp) )
            {
                if ( sp->NOTARIZED_HEIGHT > 0 && sp->NOTARIZED_HEIGHT < *notarizedheightp )
                    rewindtarget = sp->NOTARIZED_HEIGHT - 1;
                else if ( *notarizedheightp > 101 )
                    rewindtarget = *notarizedheightp - 101;
                else rewindtarget = 0;
                if ( rewindtarget != 0 && rewindtarget > KOMODO_REWIND && rewindtarget > last_rewind )
                {
                    if ( last_rewind != 0 )
                    {
                        //KOMODO_REWIND = rewindtarget;
                        error("%s FORK detected. notarized.%d %s not in this chain! last notarization %d -> rewindtarget.%d\n",ASSETCHAINS_SYMBOL,*notarizedheightp,srchash.ToString().c_str(),sp->NOTARIZED_HEIGHT,rewindtarget);
                    }
                    last_rewind = rewindtarget;
                }
            } else validated = 1;
            if ( notarized != 0 && *notarizedheightp > sp->NOTARIZED_HEIGHT && *notarizedheightp < height && validated != 0 )
            {
                int32_t nameoffset = (int32_t)strlen(ASSETCHAINS_SYMBOL) + 1;
                sp->NOTARIZED_HEIGHT = *notarizedheightp;
                sp->NOTARIZED_HASH = srchash;
                sp->NOTARIZED_DESTTXID = desttxid;
                /*memset(&sp->MoM,0,sizeof(sp->MoM));
                sp->MoMdepth = 0;
                if ( len+36 <= opretlen )
                {
                    len += iguana_rwbignum(0,&scriptbuf[len+nameoffset],32,(uint8_t *)&sp->MoM);
                    len += iguana_rwnum(0,&scriptbuf[len+nameoffset],sizeof(sp->MoMdepth),(uint8_t *)&sp->MoMdepth);
                    if ( sp->MoM == zero || sp->MoMdepth > 1440 || sp->MoMdepth < 0 )
                    {
                        memset(&sp->MoM,0,sizeof(sp->MoM));
                        sp->MoMdepth = 0;
                    }
                    else
                    {
                        //LogPrintf("VALID %s MoM.%s [%d]\n",ASSETCHAINS_SYMBOL,sp->MoM.ToString().c_str(),sp->MoMdepth);
                    }
                }*/
                komodo_stateupdate(height,0,0,0,zero,0,0,0,0,0,0,0,0,0,0);//,sp->MoM,sp->MoMdepth);
                len += nameoffset;
                //if ( ASSETCHAINS_SYMBOL[0] != 0 )
                //    LogPrintf("[%s] ht.%d NOTARIZED.%d %s.%s %sTXID.%s lens.(%d %d) MoM.%s %d\n",ASSETCHAINS_SYMBOL,height,*notarizedheightp,ASSETCHAINS_SYMBOL[0]==0?"RES":ASSETCHAINS_SYMBOL,srchash.ToString().c_str(),ASSETCHAINS_SYMBOL[0]==0?"BTC":"RES",desttxid.ToString().c_str(),opretlen,len,sp->MoM.ToString().c_str(),sp->MoMdepth);
                if ( ASSETCHAINS_SYMBOL[0] == 0 )
                {
                    if ( signedfp == 0 )
                    {
                        char fname[512];
                        komodo_statefname(fname,ASSETCHAINS_SYMBOL,(char *)"signedmasks");
                        if ( (signedfp= fopen(fname,"rb+")) == 0 )
                            signedfp = fopen(fname,"wb");
                        else fseek(signedfp,0,SEEK_END);
                    }
                    if ( signedfp != 0 )
                    {
                        fwrite(&height,1,sizeof(height),signedfp);
                        fwrite(&signedmask,1,sizeof(signedmask),signedfp);
                        fflush(signedfp);
                    }
                    if ( opretlen > len && scriptbuf[len] == 'A' )
                    {
                        //for (i=0; i<opretlen-len; i++)
                        //    LogPrintf("%02x",scriptbuf[len+i]);
                        //LogPrintf(" Found extradata.[%d] %d - %d\n",opretlen-len,opretlen,len);
                        komodo_stateupdate(height,0,0,0,txhash,0,0,0,0,0,0,value,&scriptbuf[len],opretlen-len+4+3+(scriptbuf[1] == 0x4d),j);//,zero,0);
                    }
                }
            } else if ( *notarizedheightp != sp->NOTARIZED_HEIGHT )
                LogPrintf("validated.%d notarized.%d %llx reject ht.%d NOTARIZED.%d prev.%d %s.%s DESTTXID.%s (%s) len.%d opretlen.%d\n",validated,notarized,(long long)signedmask,height,*notarizedheightp,sp->NOTARIZED_HEIGHT,ASSETCHAINS_SYMBOL[0]==0?"RES":ASSETCHAINS_SYMBOL,srchash.ToString().c_str(),desttxid.ToString().c_str(),(char *)&scriptbuf[len],len,opretlen);
        }
        else if ( i == 0 && j == 1 && opretlen == 149 )
        {
            if ( notaryid >= 0 && notaryid < 64 )
                komodo_paxpricefeed(height,&scriptbuf[len],opretlen);
        }
        else
        {
            //int32_t k; for (k=0; k<scriptlen; k++)
            //    LogPrintf("%02x",scriptbuf[k]);
            //LogPrintf(" <- script ht.%d i.%d j.%d value %.8f %s\n",height,i,j,dstr(value),ASSETCHAINS_SYMBOL);
            if ( opretlen >= 32*2+4 && strcmp(ASSETCHAINS_SYMBOL[0]==0?"RES":ASSETCHAINS_SYMBOL,(char *)&scriptbuf[len+32*2+4]) == 0 )
            {
                for (k=0; k<32; k++)
                    if ( scriptbuf[len+k] != 0 )
                        break;
                if ( k == 32 )
                {
                    *isratificationp = 1;
                    LogPrintf("ISRATIFICATION (%s)\n",(char *)&scriptbuf[len+32*2+4]);
                }
            }
            
            if ( *isratificationp == 0 && (signedmask != 0 || (scriptbuf[len] != 'X' && scriptbuf[len] != 'A')) ) // && scriptbuf[len] != 'I')
                komodo_stateupdate(height,0,0,0,txhash,0,0,0,0,0,0,value,&scriptbuf[len],opretlen,j);//,zero,0);
        }
    }
    return(notaryid);
}

static void komodo_connectblock(CBlockIndex *pindex,CBlock& block)
{
    static int32_t hwmheight;
    uint64_t signedmask,voutmask; char symbol[KOMODO_ASSETCHAIN_MAXLEN],dest[KOMODO_ASSETCHAIN_MAXLEN]; struct komodo_state *sp;
    uint8_t scriptbuf[10001],pubkeys[64][33],rmd160[20],scriptPubKey[35]; uint256 zero,btctxid,txhash;
    int32_t i,j,k,numnotaries,notarized,scriptlen,isratification,nid,numvalid,specialtx,notarizedheight,notaryid,len,numvouts,numvins,height,txn_count;
    memset(&zero,0,sizeof(zero));
    komodo_init(pindex->nHeight);
    KOMODO_INITDONE = (uint32_t)time(NULL);
    if ( (sp= komodo_stateptr(symbol,dest)) == 0 )
    {
        error("unexpected null komodostateptr.[%s]\n",ASSETCHAINS_SYMBOL);
        return;
    }
    //LogPrintf("%s connect.%d\n",ASSETCHAINS_SYMBOL,pindex->nHeight);
    numnotaries = komodo_notaries(pubkeys,pindex->nHeight,pindex->GetBlockTime());
    calc_rmd160_sha256(rmd160,pubkeys[0],33);
    if ( pindex->nHeight > hwmheight )
        hwmheight = pindex->nHeight;
    else
    {
        if ( pindex->nHeight != hwmheight )
            LogPrintf("%s hwmheight.%d vs pindex->nHeight.%d t.%u reorg.%d\n",ASSETCHAINS_SYMBOL,hwmheight,pindex->nHeight,(uint32_t)pindex->nTime,hwmheight-pindex->nHeight);
        komodo_event_rewind(sp,symbol,pindex->nHeight);
        komodo_stateupdate(pindex->nHeight,0,0,0,zero,0,0,0,0,-pindex->nHeight,pindex->nTime,0,0,0,0);//,zero,0);
    }
    komodo_currentheight_set(chainActive.Tip()->nHeight);
    if ( pindex != 0 )
    {
        height = pindex->nHeight;
        txn_count = block.vtx.size();
        for (i=0; i<txn_count; i++)
        {
            txhash = block.vtx[i].GetHash();
            numvouts = block.vtx[i].vout.size();
            notaryid = -1;
            voutmask = specialtx = notarizedheight = isratification = notarized = 0;
            signedmask = (height < 91400) ? 1 : 0;
            numvins = block.vtx[i].vin.size();
            for (j=0; j<numvins; j++)
            {
                if ( i == 0 && j == 0 )
                    continue;
                if ( (scriptlen= gettxout_scriptPubKey(scriptPubKey,sizeof(scriptPubKey),block.vtx[i].vin[j].prevout.hash,block.vtx[i].vin[j].prevout.n)) > 0 )
                {
                    if ( (k= komodo_notarycmp(scriptPubKey,scriptlen,pubkeys,numnotaries,rmd160)) >= 0 )
                        signedmask |= (1LL << k);
                    else if ( 0 && numvins >= 17 )
                    {
                        int32_t k;
                        for (k=0; k<scriptlen; k++)
                            LogPrintf("%02x",scriptPubKey[k]);
                        LogPrintf(" scriptPubKey doesnt match any notary vini.%d of %d\n",j,numvins);
                    }
                } else LogPrintf("cant get scriptPubKey for ht.%d txi.%d vin.%d\n",height,i,j);
            }
            numvalid = bitweight(signedmask);
            if ( (((height < 90000 || (signedmask & 1) != 0) && numvalid >= KOMODO_MINRATIFY) ||
                  (numvalid >= KOMODO_MINRATIFY && ASSETCHAINS_SYMBOL[0] != 0) ||
                  numvalid > (numnotaries/5)) )
            {
                if ( ASSETCHAINS_SYMBOL[0] != 0 )
                {
                    static FILE *signedfp;
                    if ( signedfp == 0 )
                    {
                        char fname[512];
                        komodo_statefname(fname,ASSETCHAINS_SYMBOL,(char *)"signedmasks");
                        if ( (signedfp= fopen(fname,"rb+")) == 0 )
                            signedfp = fopen(fname,"wb");
                        else fseek(signedfp,0,SEEK_END);
                    }
                    if ( signedfp != 0 )
                    {
                        fwrite(&height,1,sizeof(height),signedfp);
                        fwrite(&signedmask,1,sizeof(signedmask),signedfp);
                        fflush(signedfp);
                    }
                    LogPrintf("[%s] ht.%d txi.%d signedmask.%llx numvins.%d numvouts.%d <<<<<<<<<<<  notarized\n",ASSETCHAINS_SYMBOL,height,i,(long long)signedmask,numvins,numvouts);
                }
                notarized = 1;
            }
            if ( NOTARY_PUBKEY33[0] != 0 && ASSETCHAINS_SYMBOL[0] == 0 )
                LogPrintf("(tx.%d: ",i);
            for (j=0; j<numvouts; j++)
            {
                /*if ( i == 0 && j == 0 )
                {
                    uint8_t *script = (uint8_t *)block.vtx[0].vout[numvouts-1].scriptPubKey.data();
                    if ( numvouts <= 2 || script[0] != 0x6a )
                    {
                        if ( numvouts == 2 && block.vtx[0].vout[1].nValue != 0 )
                        {
                            LogPrintf("ht.%d numvouts.%d value %.8f\n",height,numvouts,dstr(block.vtx[0].vout[1].nValue));
                            if ( height >= 235300 && block.vtx[0].vout[1].nValue >= 100000*COIN )
                                block.vtx[0].vout[1].nValue = 0;
                            break;
                        }
                    }
                }*/
                if ( NOTARY_PUBKEY33[0] != 0 && ASSETCHAINS_SYMBOL[0] == 0 )
                    LogPrintf("%.8f ",dstr(block.vtx[i].vout[j].nValue));
                len = block.vtx[i].vout[j].scriptPubKey.size();
                if ( len >= sizeof(uint32_t) && len <= sizeof(scriptbuf) )
                {
#ifdef KOMODO_ZCASH
                    memcpy(scriptbuf,block.vtx[i].vout[j].scriptPubKey.data(),len);
#else
                    memcpy(scriptbuf,(uint8_t *)&block.vtx[i].vout[j].scriptPubKey[0],len);
#endif
                    notaryid = komodo_voutupdate(&isratification,notaryid,scriptbuf,len,height,txhash,i,j,&voutmask,&specialtx,&notarizedheight,(uint64_t)block.vtx[i].vout[j].nValue,notarized,signedmask,(uint32_t)chainActive.Tip()->GetBlockTime());
                    if ( 0 && i > 0 )
                    {
                        for (k=0; k<len; k++)
                            LogPrintf("%02x",scriptbuf[k]);
                        LogPrintf(" <- notaryid.%d ht.%d i.%d j.%d numvouts.%d numvins.%d voutmask.%llx txid.(%s)\n",notaryid,height,i,j,numvouts,numvins,(long long)voutmask,txhash.ToString().c_str());
                    }
                }
            }
            if ( NOTARY_PUBKEY33[0] != 0 && ASSETCHAINS_SYMBOL[0] == 0 )
                LogPrintf(") ");
            if ( 0 && ASSETCHAINS_SYMBOL[0] == 0 )
                LogPrintf("[%s] ht.%d txi.%d signedmask.%llx numvins.%d numvouts.%d notarized.%d special.%d isratification.%d\n",ASSETCHAINS_SYMBOL,height,i,(long long)signedmask,numvins,numvouts,notarized,specialtx,isratification);
            if ( notarized != 0 && (notarizedheight != 0 || specialtx != 0) )
            {
                if ( isratification != 0 )
                {
                    LogPrintf("%s NOTARY SIGNED.%llx numvins.%d ht.%d txi.%d notaryht.%d specialtx.%d\n",ASSETCHAINS_SYMBOL,(long long)signedmask,numvins,height,i,notarizedheight,specialtx);
                    LogPrintf("ht.%d specialtx.%d isratification.%d numvouts.%d signed.%llx numnotaries.%d\n",height,specialtx,isratification,numvouts,(long long)signedmask,numnotaries);
                }
                if ( specialtx != 0 && isratification != 0 && numvouts > 2 )
                {
                    numvalid = 0;
                    memset(pubkeys,0,sizeof(pubkeys));
                    for (j=1; j<numvouts-1; j++)
                    {
                        len = block.vtx[i].vout[j].scriptPubKey.size();
                        if ( len >= sizeof(uint32_t) && len <= sizeof(scriptbuf) )
                        {
#ifdef KOMODO_ZCASH
                            memcpy(scriptbuf,block.vtx[i].vout[j].scriptPubKey.data(),len);
#else
                            memcpy(scriptbuf,(uint8_t *)&block.vtx[i].vout[j].scriptPubKey[0],len);
#endif
                            if ( len == 35 && scriptbuf[0] == 33 && scriptbuf[34] == 0xac )
                            {
                                memcpy(pubkeys[numvalid++],scriptbuf+1,33);
                                for (k=0; k<33; k++)
                                    LogPrintf("%02x",scriptbuf[k+1]);
                                LogPrintf(" <- new notary.[%d]\n",j-1);
                            }
                        }
                    }
                    if ( ((signedmask & 1) != 0 && numvalid >= KOMODO_MINRATIFY) || bitweight(signedmask) > (numnotaries/3) )
                    {
                        memset(&txhash,0,sizeof(txhash));
                        komodo_stateupdate(height,pubkeys,numvalid,0,txhash,0,0,0,0,0,0,0,0,0,0);//,zero,0);
                        LogPrintf("RATIFIED! >>>>>>>>>> new notaries.%d newheight.%d from height.%d\n",numvalid,(((height+KOMODO_ELECTION_GAP/2)/KOMODO_ELECTION_GAP)+1)*KOMODO_ELECTION_GAP,height);
                    } else LogPrintf("signedmask.%llx numvalid.%d wt.%d numnotaries.%d\n",(long long)signedmask,numvalid,bitweight(signedmask),numnotaries);
                }
            }
        }
        if ( NOTARY_PUBKEY33[0] != 0 && ASSETCHAINS_SYMBOL[0] == 0 )
            LogPrintf("%s ht.%d\n",ASSETCHAINS_SYMBOL[0] == 0 ? "RES" : ASSETCHAINS_SYMBOL,height);
        if ( pindex->nHeight == hwmheight )
            komodo_stateupdate(height,0,0,0,zero,0,0,0,0,height,(uint32_t)pindex->nTime,0,0,0,0);//,zero,0);
    } else error("komodo_connectblock: unexpected null pindex\n");
    //KOMODO_INITDONE = (uint32_t)time(NULL);
    //LogPrintf("%s end connect.%d\n",ASSETCHAINS_SYMBOL,pindex->nHeight);
}

static void komodo_disconnect(CBlockIndex *pindex,CBlock& block)
{
    char symbol[KOMODO_ASSETCHAIN_MAXLEN],dest[KOMODO_ASSETCHAIN_MAXLEN]; struct komodo_state *sp;
    //LogPrintf("disconnect ht.%d\n",pindex->nHeight);
    komodo_init(pindex->nHeight);
    if ( (sp= komodo_stateptr(symbol,dest)) != 0 )
    {
        //sp->rewinding = pindex->nHeight;
        //LogPrintf("-%d ",pindex->nHeight);
    } else error("komodo_disconnect: ht.%d cant get komodo_state.(%s)\n",pindex->nHeight,ASSETCHAINS_SYMBOL);
}

#endif //komodo__h
