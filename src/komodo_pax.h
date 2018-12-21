
#ifndef komodo_pax__h
#define komodo_pax__h

#include "komodo_defs.h"
#include "komodo_globals.h"
#include "komodo_utils.h"

static double PAX_BTCUSD(int32_t height,uint32_t btcusd)
{
    double btcfactor,BTCUSD;
    if ( height >= BTCFACTOR_HEIGHT )
        btcfactor = 100000.;
    else btcfactor = 1000.;
    BTCUSD = ((double)btcusd / (1000000000. / btcfactor));
    if ( height >= BTCFACTOR_HEIGHT && height < 500000 && BTCUSD > 20000 && btcfactor == 100000. )
        BTCUSD /= 100;
    return(BTCUSD);
}

static void komodo_pvals(int32_t height,uint32_t *pvals,uint8_t numpvals)
{
    int32_t i,nonz; uint32_t kmdbtc,btcusd,cnyusd; double KMDBTC,BTCUSD,CNYUSD;
    if ( numpvals >= 35 )
    {
        for (nonz=i=0; i<32; i++)
        {
            if ( pvals[i] != 0 )
                nonz++;
            //LogPrintf("%u ",pvals[i]);
        }
        if ( nonz == 32 )
        {
            kmdbtc = pvals[i++];
            btcusd = pvals[i++];
            cnyusd = pvals[i++];
            KMDBTC = ((double)kmdbtc / (1000000000. * 1000.));
            BTCUSD = PAX_BTCUSD(height,btcusd);
            CNYUSD = ((double)cnyusd / 1000000000.);
            portable_mutex_lock(&komodo_mutex);
            PVALS = (uint32_t *)realloc(PVALS,(NUM_PRICES+1) * sizeof(*PVALS) * 36);
            PVALS[36 * NUM_PRICES] = height;
            memcpy(&PVALS[36 * NUM_PRICES + 1],pvals,sizeof(*pvals) * 35);
            NUM_PRICES++;
            portable_mutex_unlock(&komodo_mutex);
            if ( 0 )
                LogPrintf("OP_RETURN.%d RES %.8f BTC %.6f CNY %.6f NUM_PRICES.%d (%llu %llu %llu)\n",height,KMDBTC,BTCUSD,CNYUSD,NUM_PRICES,(long long)kmdbtc,(long long)btcusd,(long long)cnyusd);
        }
    }
}

static int32_t PAX_pubkey(int32_t rwflag,uint8_t *pubkey33,uint8_t *addrtypep,uint8_t rmd160[20],char fiat[4],uint8_t *shortflagp,int64_t *fiatoshisp)
{
    if ( rwflag != 0 )
    {
        memset(pubkey33,0,33);
        pubkey33[0] = 0x02 | (*shortflagp != 0);
        memcpy(&pubkey33[1],fiat,3);
        iguana_rwnum(rwflag,&pubkey33[4],sizeof(*fiatoshisp),(void *)fiatoshisp);
        pubkey33[12] = *addrtypep;
        memcpy(&pubkey33[13],rmd160,20);
    }
    else
    {
        *shortflagp = (pubkey33[0] == 0x03);
        memcpy(fiat,&pubkey33[1],3);
        fiat[3] = 0;
        iguana_rwnum(rwflag,&pubkey33[4],sizeof(*fiatoshisp),(void *)fiatoshisp);
        if ( *shortflagp != 0 )
            *fiatoshisp = -(*fiatoshisp);
        *addrtypep = pubkey33[12];
        memcpy(rmd160,&pubkey33[13],20);
    }
    return(33);
}

static uint64_t komodo_paxvol(uint64_t volume,uint64_t price)
{
    if ( volume < 10000000000 )
        return((volume * price) / 1000000000);
    else if ( volume < (uint64_t)10 * 10000000000 )
        return((volume * (price / 10)) / 100000000);
    else if ( volume < (uint64_t)100 * 10000000000 )
        return(((volume / 10) * (price / 10)) / 10000000);
    else if ( volume < (uint64_t)1000 * 10000000000 )
        return(((volume / 10) * (price / 100)) / 1000000);
    else if ( volume < (uint64_t)10000 * 10000000000 )
        return(((volume / 100) * (price / 100)) / 100000);
    else if ( volume < (uint64_t)100000 * 10000000000 )
        return(((volume / 100) * (price / 1000)) / 10000);
    else if ( volume < (uint64_t)1000000 * 10000000000 )
        return(((volume / 1000) * (price / 1000)) / 1000);
    else if ( volume < (uint64_t)10000000 * 10000000000 )
        return(((volume / 1000) * (price / 10000)) / 100);
    else return(((volume / 10000) * (price / 10000)) / 10);
}

static void pax_rank(uint64_t *ranked,uint32_t *pvals)
{
    int32_t i; uint64_t vals[32],sum = 0;
    for (i=0; i<32; i++)
    {
        vals[i] = komodo_paxvol(M1SUPPLY[i] / MINDENOMS[i],pvals[i]);
        sum += vals[i];
    }
    for (i=0; i<32; i++)
    {
        ranked[i] = (vals[i] * 1000000000) / sum;
        //LogPrintf("%.6f ",(double)ranked[i]/1000000000.);
    }
    //LogPrintf("sum %llu\n",(long long)sum);
}

static uint64_t komodo_paxcalc(int32_t height,uint32_t *pvals,int32_t baseid,int32_t relid,uint64_t basevolume,uint64_t refkmdbtc,uint64_t refbtcusd)
{
    uint32_t pvalb,pvalr; double BTCUSD; uint64_t price,kmdbtc,btcusd,usdvol,baseusd,usdkmd,baserel,ranked[32];
    if ( basevolume > KOMODO_PAXMAX )
    {
        LogPrintf("paxcalc overflow %.8f\n",dstr(basevolume));
        return(0);
    }
    if ( (pvalb= pvals[baseid]) != 0 )
    {
        if ( relid == MAX_CURRENCIES )
        {
            if ( height < 236000 )
            {
                if ( kmdbtc == 0 )
                    kmdbtc = pvals[MAX_CURRENCIES];
                if ( btcusd == 0 )
                    btcusd = pvals[MAX_CURRENCIES + 1];
            }
            else
            {
                if ( (kmdbtc= pvals[MAX_CURRENCIES]) == 0 )
                    kmdbtc = refkmdbtc;
                if ( (btcusd= pvals[MAX_CURRENCIES + 1]) == 0 )
                    btcusd = refbtcusd;
            }
            if ( kmdbtc < 25000000 )
                kmdbtc = 25000000;
            if ( pvals[USD] != 0 && kmdbtc != 0 && btcusd != 0 )
            {
                baseusd = (((uint64_t)pvalb * 1000000000) / pvals[USD]);
                usdvol = komodo_paxvol(basevolume,baseusd);
                usdkmd = ((uint64_t)kmdbtc * 1000000000) / btcusd;
                if ( height >= 236000-10 )
                {
                    BTCUSD = PAX_BTCUSD(height,btcusd);
                    if ( height < BTCFACTOR_HEIGHT || (height < 500000 && BTCUSD > 20000) )
                        usdkmd = ((uint64_t)kmdbtc * btcusd) / 1000000000;
                    else usdkmd = ((uint64_t)kmdbtc * btcusd) / 10000000;
                    ///if ( height >= BTCFACTOR_HEIGHT && BTCUSD >= 43 )
                    //    usdkmd = ((uint64_t)kmdbtc * btcusd) / 10000000;
                    //else usdkmd = ((uint64_t)kmdbtc * btcusd) / 1000000000;
                    price = ((uint64_t)10000000000 * MINDENOMS[USD] / MINDENOMS[baseid]) / komodo_paxvol(usdvol,usdkmd);
                    //LogPrintf("ht.%d %.3f kmdbtc.%llu btcusd.%llu base -> USD %llu, usdkmd %llu usdvol %llu -> %llu\n",height,BTCUSD,(long long)kmdbtc,(long long)btcusd,(long long)baseusd,(long long)usdkmd,(long long)usdvol,(long long)(MINDENOMS[USD] * komodo_paxvol(usdvol,usdkmd) / (MINDENOMS[baseid]/100)));
                    //LogPrintf("usdkmd.%llu basevolume.%llu baseusd.%llu paxvol.%llu usdvol.%llu -> %llu %llu\n",(long long)usdkmd,(long long)basevolume,(long long)baseusd,(long long)komodo_paxvol(basevolume,baseusd),(long long)usdvol,(long long)(MINDENOMS[USD] * komodo_paxvol(usdvol,usdkmd) / (MINDENOMS[baseid]/100)),(long long)price);
                    //LogPrintf("usdkmd.%llu basevolume.%llu baseusd.%llu paxvol.%llu usdvol.%llu -> %llu\n",(long long)usdkmd,(long long)basevolume,(long long)baseusd,(long long)komodo_paxvol(basevolume,baseusd),(long long)usdvol,(long long)(MINDENOMS[USD] * komodo_paxvol(usdvol,usdkmd) / (MINDENOMS[baseid]/100)));
                } else price = (MINDENOMS[USD] * komodo_paxvol(usdvol,usdkmd) / (MINDENOMS[baseid]/100));
                return(price);
            } //else LogPrintf("zero val in RES conv %llu %llu %llu\n",(long long)pvals[USD],(long long)kmdbtc,(long long)btcusd);
        }
        else if ( baseid == relid )
        {
            if ( baseid != MAX_CURRENCIES )
            {
                pax_rank(ranked,pvals);
                //LogPrintf("%s M1 percentage %.8f\n",CURRENCIES[baseid],dstr(10 * ranked[baseid]));
                return(10 * ranked[baseid]); // scaled percentage of M1 total
            } else return(basevolume);
        }
        else if ( (pvalr= pvals[relid]) != 0 )
        {
            baserel = ((uint64_t)pvalb * 1000000000) / pvalr;
            //LogPrintf("baserel.%lld %lld %lld %.8f %.8f\n",(long long)baserel,(long long)MINDENOMS[baseid],(long long)MINDENOMS[relid],dstr(MINDENOMS[baseid]/MINDENOMS[relid]),dstr(MINDENOMS[relid]/MINDENOMS[baseid]));
            if ( MINDENOMS[baseid] > MINDENOMS[relid] )
                basevolume /= (MINDENOMS[baseid] / MINDENOMS[relid]);
            else if ( MINDENOMS[baseid] < MINDENOMS[relid] )
                basevolume *= (MINDENOMS[relid] / MINDENOMS[baseid]);
            return(komodo_paxvol(basevolume,baserel));
        }
        else LogPrintf("null pval for %s\n",CURRENCIES[relid]);
    } else LogPrintf("null pval for %s\n",CURRENCIES[baseid]);
    return(0);
}

static uint64_t _komodo_paxprice(uint64_t *kmdbtcp,uint64_t *btcusdp,int32_t height,const char *base,char *rel,uint64_t basevolume,uint64_t kmdbtc,uint64_t btcusd)
{
    int32_t baseid=-1,relid=-1,i; uint32_t *ptr,*pvals;
    if ( height > 10 )
        height -= 10;
    if ( (baseid= komodo_baseid(base)) >= 0 && (relid= komodo_baseid(rel)) >= 0 )
    {
        //portable_mutex_lock(&komodo_mutex);
        for (i=NUM_PRICES-1; i>=0; i--)
        {
            ptr = &PVALS[36 * i];
            if ( *ptr < height )
            {
                pvals = &ptr[1];
                if ( kmdbtcp != 0 && btcusdp != 0 )
                {
                    *kmdbtcp = pvals[MAX_CURRENCIES] / 539;
                    *btcusdp = pvals[MAX_CURRENCIES + 1] / 539;
                }
                //portable_mutex_unlock(&komodo_mutex);
                if ( kmdbtc != 0 && btcusd != 0 )
                    return(komodo_paxcalc(height,pvals,baseid,relid,basevolume,kmdbtc,btcusd));
                else return(0);
            }
        }
        //portable_mutex_unlock(&komodo_mutex);
    } //else LogPrintf("paxprice invalid base.%s %d, rel.%s %d\n",base,baseid,rel,relid);
    return(0);
}

static uint64_t komodo_paxcorrelation(uint64_t *votes,int32_t numvotes,uint64_t seed)
{
    int32_t i,j,k,ind,zeroes,wt,nonz; int64_t delta; uint64_t lastprice,tolerance,den,densum,sum=0;
    for (sum=i=zeroes=nonz=0; i<numvotes; i++)
    {
        if ( votes[i] == 0 )
            zeroes++;
        else sum += votes[i], nonz++;
    }
    if ( nonz < (numvotes >> 2) )
        return(0);
    sum /= nonz;
    lastprice = sum;
    for (i=0; i<numvotes; i++)
    {
        if ( votes[i] == 0 )
            votes[i] = lastprice;
        else lastprice = votes[i];
    }
    tolerance = sum / 50;
    for (k=0; k<numvotes; k++)
    {
        ind = Peggy_inds[(k + seed) % numvotes];
        i = (int32_t)(ind % numvotes);
        wt = 0;
        if ( votes[i] != 0 )
        {
            for (j=0; j<numvotes; j++)
            {
                if ( votes[j] != 0 )
                {
                    if ( (delta= (votes[i] - votes[j])) < 0 )
                        delta = -delta;
                        if ( delta <= tolerance )
                        {
                            wt++;
                            if ( wt > (numvotes >> 1) )
                                break;
                        }
                }
            }
        }
        if ( wt > (numvotes >> 1) )
        {
            ind = i;
            for (densum=sum=j=0; j<numvotes; j++)
            {
                den = peggy_smooth_coeffs[j];
                densum += den;
                sum += (den * votes[(ind + j) % numvotes]);
                //LogPrintf("(%llu/%llu %.8f) ",(long long)sum,(long long)densum,(double)sum/densum);
            }
            if ( densum != 0 )
                sum /= densum;
            //sum = (sum * basevolume);
            //LogPrintf("paxprice seed.%llx sum %.8f densum %.8f\n",(long long)seed,dstr(sum),dstr(densum));
            break;
        }
    }
    return(sum);
}

static uint64_t _komodo_paxpriceB(uint64_t seed,int32_t height,const char *base,char *rel,uint64_t basevolume)
{
    int32_t i,j,k,ind,zeroes,numvotes,wt,nonz; int64_t delta; uint64_t lastprice,tolerance,den,densum,sum=0,votes[sizeof(Peggy_inds)/sizeof(*Peggy_inds)],btcusds[sizeof(Peggy_inds)/sizeof(*Peggy_inds)],kmdbtcs[sizeof(Peggy_inds)/sizeof(*Peggy_inds)],kmdbtc,btcusd;
    if ( basevolume > KOMODO_PAXMAX )
    {
        LogPrintf("komodo_paxprice overflow %.8f\n",dstr(basevolume));
        return(0);
    }
    if ( strcmp(base,"RES") == 0 || strcmp(base,"res") == 0 )
    {
        LogPrintf("RES cannot be base currency\n");
        return(0);
    }
    numvotes = (int32_t)(sizeof(Peggy_inds)/sizeof(*Peggy_inds));
    memset(votes,0,sizeof(votes));
    //if ( komodo_kmdbtcusd(0,&kmdbtc,&btcusd,height) < 0 ) crashes when via passthru GUI use
    {
        memset(btcusds,0,sizeof(btcusds));
        memset(kmdbtcs,0,sizeof(kmdbtcs));
        for (i=0; i<numvotes; i++)
        {
            _komodo_paxprice(&kmdbtcs[numvotes-1-i],&btcusds[numvotes-1-i],height-i,base,rel,100000,0,0);
            //LogPrintf("(%llu %llu) ",(long long)kmdbtcs[numvotes-1-i],(long long)btcusds[numvotes-1-i]);
        }
        kmdbtc = komodo_paxcorrelation(kmdbtcs,numvotes,seed) * 539;
        btcusd = komodo_paxcorrelation(btcusds,numvotes,seed) * 539;
        //komodo_kmdbtcusd(1,&kmdbtc,&btcusd,height);
    }
    for (i=nonz=0; i<numvotes; i++)
    {
        if ( (votes[numvotes-1-i]= _komodo_paxprice(0,0,height-i,base,rel,100000,kmdbtc,btcusd)) == 0 )
            zeroes++;
        else
        {
            nonz++;
            sum += votes[numvotes-1-i];
            //if ( (i % 10) == 0 )
            //    LogPrintf("[%llu] ",(long long)votes[numvotes-1-i]);
        }
    }
    //LogPrintf("kmdbtc %llu btcusd %llu ",(long long)kmdbtc,(long long)btcusd);
    //LogPrintf("komodo_paxprice nonz.%d of numvotes.%d seed.%llu %.8f\n",nonz,numvotes,(long long)seed,nonz!=0?dstr(1000. * (double)sum/nonz):0);
    if ( nonz <= (numvotes >> 1) )
    {
        return(0);
    }
    return(komodo_paxcorrelation(votes,numvotes,seed) * basevolume / 100000);
}

static uint64_t komodo_paxpriceB(uint64_t seed,int32_t height,const char *base,char *rel,uint64_t basevolume)
{
    uint64_t baseusd,basekmd,usdkmd; int32_t baseid = komodo_baseid(base);
    if ( height >= 236000 && strcmp(rel,"res") == 0 )
    {
        usdkmd = _komodo_paxpriceB(seed,height,(char *)"USD",(char *)"RES",SATOSHIDEN);
        if ( strcmp("usd",base) == 0 )
            return(komodo_paxvol(basevolume,usdkmd) * 10);
        baseusd = _komodo_paxpriceB(seed,height,base,(char *)"USD",SATOSHIDEN);
        basekmd = (komodo_paxvol(basevolume,baseusd) * usdkmd) / 10000000;
        //if ( strcmp("RES",base) == 0 )
        //    LogPrintf("baseusd.%llu usdkmd.%llu %llu\n",(long long)baseusd,(long long)usdkmd,(long long)basekmd);
        return(basekmd);
    } else return(_komodo_paxpriceB(seed,height,base,rel,basevolume));
}

static uint64_t komodo_paxprice(uint64_t *seedp,int32_t height,const char *base,char *rel,uint64_t basevolume)
{
    int32_t i,nonz=0; int64_t diff; uint64_t price,seed,sum = 0;
    if ( ASSETCHAINS_SYMBOL[0] == 0 && chainActive.Tip() != 0 && height > chainActive.Tip()->nHeight )
    {
        if ( height < 100000000 )
        {
            static uint32_t counter;
            if ( counter++ < 3 )
                LogPrintf("komodo_paxprice height.%d vs tip.%d\n",height,chainActive.Tip()->nHeight);
        }
        return(0);
    }
    *seedp = komodo_seed(height);
    portable_mutex_lock(&komodo_mutex);
    for (i=0; i<17; i++)
    {
        if ( (price= komodo_paxpriceB(*seedp,height-i,base,rel,basevolume)) != 0 )
        {
            sum += price;
            nonz++;
            if ( 0 && i == 1 && nonz == 2 )
            {
                diff = (((int64_t)price - (sum >> 1)) * 10000);
                if ( diff < 0 )
                    diff = -diff;
                diff /= price;
                LogPrintf("(%llu %llu %lld).%lld ",(long long)price,(long long)(sum>>1),(long long)(((int64_t)price - (sum >> 1)) * 10000),(long long)diff);
                if ( diff < 33 )
                    break;
            }
            else if ( 0 && i == 3 && nonz == 4 )
            {
                diff = (((int64_t)price - (sum >> 2)) * 10000);
                if ( diff < 0 )
                    diff = -diff;
                diff /= price;
                LogPrintf("(%llu %llu %lld).%lld ",(long long)price,(long long)(sum>>2),(long long) (((int64_t)price - (sum >> 2)) * 10000),(long long)diff);
                if ( diff < 20 )
                    break;
            }
        }
        if ( height < 165000 || height > 236000 )
            break;
    }
    portable_mutex_unlock(&komodo_mutex);
    if ( nonz != 0 )
        sum /= nonz;
    //LogPrintf("-> %lld %s/%s i.%d ht.%d\n",(long long)sum,base,rel,i,height);
    return(sum);
}

static uint64_t PAX_fiatdest(uint64_t *seedp,int32_t tokomodo,char *destaddr,uint8_t pubkey33[33],char *coinaddr,int32_t height,char *origbase,int64_t fiatoshis)
{
    uint8_t shortflag = 0; char base[4]; int32_t i,baseid; uint8_t addrtype,rmd160[20]; int64_t komodoshis = 0;
    *seedp = komodo_seed(height);
    if ( (baseid= komodo_baseid(origbase)) < 0 || baseid == MAX_CURRENCIES )
    {
        if ( 0 && origbase[0] != 0 )
            LogPrintf("[%s] PAX_fiatdest illegal base.(%s)\n",ASSETCHAINS_SYMBOL,origbase);
        return(0);
    }
    for (i=0; i<3; i++)
        base[i] = toupper(origbase[i]);
    base[i] = 0;
    if ( fiatoshis < 0 )
        shortflag = 1, fiatoshis = -fiatoshis;
    komodoshis = komodo_paxprice(seedp,height,base,(char *)"RES",(uint64_t)fiatoshis);
    //LogPrintf("PAX_fiatdest ht.%d price %s %.8f -> RES %.8f seed.%llx\n",height,base,(double)fiatoshis/COIN,(double)komodoshis/COIN,(long long)*seedp);
    if ( bitcoin_addr2rmd160(&addrtype,rmd160,coinaddr) == 20 )
    {
        PAX_pubkey(1,pubkey33,&addrtype,rmd160,base,&shortflag,tokomodo != 0 ? &komodoshis : &fiatoshis);
        bitcoin_address(destaddr,KOMODO_PUBTYPE,pubkey33,33);
    }
    return(komodoshis);
}

static int32_t dpow_readprices(int32_t height,uint8_t *data,uint32_t *timestampp,double *KMDBTCp,double *BTCUSDp,double *CNYUSDp,uint32_t *pvals)
{
    uint32_t kmdbtc,btcusd,cnyusd; int32_t i,n,nonz,len = 0;
    if ( data[0] == 'P' && data[5] == 35 )
        data++;
    len += iguana_rwnum(0,&data[len],sizeof(uint32_t),(void *)timestampp);
    len += iguana_rwnum(0,&data[len],sizeof(uint32_t),(void *)&n);
    if ( n != 35 )
    {
        LogPrintf("dpow_readprices illegal n.%d\n",n);
        return(-1);
    }
    len += iguana_rwnum(0,&data[len],sizeof(uint32_t),(void *)&kmdbtc); // /= 1000
    len += iguana_rwnum(0,&data[len],sizeof(uint32_t),(void *)&btcusd); // *= 1000
    len += iguana_rwnum(0,&data[len],sizeof(uint32_t),(void *)&cnyusd);
    *KMDBTCp = ((double)kmdbtc / (1000000000. * 1000.));
    *BTCUSDp = PAX_BTCUSD(height,btcusd);
    *CNYUSDp = ((double)cnyusd / 1000000000.);
    for (i=nonz=0; i<n-3; i++)
    {
        if ( pvals[i] != 0 )
            nonz++;
        //else if ( nonz != 0 )
        //    LogPrintf("pvals[%d] is zero\n",i);
        len += iguana_rwnum(0,&data[len],sizeof(uint32_t),(void *)&pvals[i]);
        //LogPrintf("%u ",pvals[i]);
    }
    /*if ( nonz < n-3 )
    {
        //LogPrintf("nonz.%d n.%d retval -1\n",nonz,n);
        return(-1);
    }*/
    pvals[i++] = kmdbtc;
    pvals[i++] = btcusd;
    pvals[i++] = cnyusd;
    //LogPrintf("OP_RETURN prices\n");
    return(n);
}

static void komodo_paxpricefeed(int32_t height,uint8_t *pricefeed,int32_t opretlen)
{
    double KMDBTC,BTCUSD,CNYUSD; uint32_t numpvals,timestamp,pvals[128]; uint256 zero;
    numpvals = dpow_readprices(height,pricefeed,&timestamp,&KMDBTC,&BTCUSD,&CNYUSD,pvals);
    memset(&zero,0,sizeof(zero));
    komodo_stateupdate(height,0,0,0,zero,0,0,pvals,numpvals,0,0,0,0,0,0);
    if ( 0 )
    {
        int32_t i;
        for (i=0; i<numpvals; i++)
            LogPrintf("%u ",pvals[i]);
        LogPrintf("komodo_paxpricefeed vout OP_RETURN.%d prices numpvals.%d opretlen.%d kmdbtc %.8f BTCUSD %.8f CNYUSD %.8f\n",height,numpvals,opretlen,KMDBTC,BTCUSD,CNYUSD);
    }
}

#endif // komodo_pax__h
