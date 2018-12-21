
#ifndef komodo_kv__h
#define komodo_kv__h

#include "komodo_curve25519.h"

static int32_t komodo_kvnumdays(uint32_t flags)
{
    int32_t numdays;
    if ( (numdays= ((flags>>2)&0x3ff)+1) > 365 )
        numdays = 365;
    return(numdays);
}

static int32_t komodo_kvduration(uint32_t flags)
{
    return(komodo_kvnumdays(flags) * KOMODO_KVDURATION);
}

static uint64_t komodo_kvfee(uint32_t flags,int32_t opretlen,int32_t keylen)
{
    int32_t numdays,k; uint64_t fee;
    if ( (k= keylen) > 32 )
        k = 32;
    numdays = komodo_kvnumdays(flags);
    if ( (fee= (numdays*(opretlen * opretlen / k))) < 100000 )
        fee = 100000;
    return(fee);
}

static int32_t komodo_kvsearch(uint256 *pubkeyp,int32_t current_height,uint32_t *flagsp,int32_t *heightp,uint8_t value[IGUANA_MAXSCRIPTSIZE],uint8_t *key,int32_t keylen)
{
    struct komodo_kv *ptr; int32_t duration,retval = -1;
    *heightp = -1;
    *flagsp = 0;
    memset(pubkeyp,0,sizeof(*pubkeyp));
    portable_mutex_lock(&KOMODO_KV_mutex);
    HASH_FIND(hh,KOMODO_KV,key,keylen,ptr);
    if ( ptr != 0 )
    {
        duration = komodo_kvduration(ptr->flags);
        //LogPrintf("duration.%d flags.%d current.%d ht.%d keylen.%d valuesize.%d\n",duration,ptr->flags,current_height,ptr->height,ptr->keylen,ptr->valuesize);
        if ( current_height > (ptr->height + duration) )
        {
            HASH_DELETE(hh,KOMODO_KV,ptr);
            if ( ptr->value != 0 )
                free(ptr->value);
            if ( ptr->key != 0 )
                free(ptr->key);
            free(ptr);
        }
        else
        {
            *heightp = ptr->height;
            *flagsp = ptr->flags;
            int32_t i; for (i=0; i<32; i++)
            {
                //LogPrintf("%02x",((uint8_t *)&ptr->pubkey)[31-i]);
                ((uint8_t *)pubkeyp)[i] = ((uint8_t *)&ptr->pubkey)[31-i];
            }
            //LogPrintf(" ptr->pubkey\n");
            memcpy(pubkeyp,&ptr->pubkey,sizeof(*pubkeyp));
            if ( (retval= ptr->valuesize) > 0 )
                memcpy(value,ptr->value,retval);
        }
    }
    portable_mutex_unlock(&KOMODO_KV_mutex);
    if ( retval < 0 )
    {
        // search rawmempool
    }
    return(retval);
}

static void komodo_kvupdate(uint8_t *opretbuf,int32_t opretlen,uint64_t value)
{
    static uint256 zeroes;
    uint32_t flags; uint256 pubkey,refpubkey,sig; int32_t i,refvaluesize,hassig,coresize,haspubkey,height,kvheight; uint16_t keylen,valuesize,newflag = 0; uint8_t *key,*valueptr,keyvalue[IGUANA_MAXSCRIPTSIZE]; struct komodo_kv *ptr; char *transferpubstr,*tstr; uint64_t fee;
    if ( ASSETCHAINS_SYMBOL[0] == 0 ) // disable KV for RES
        return;
    iguana_rwnum(0,&opretbuf[1],sizeof(keylen),&keylen);
    iguana_rwnum(0,&opretbuf[3],sizeof(valuesize),&valuesize);
    iguana_rwnum(0,&opretbuf[5],sizeof(height),&height);
    iguana_rwnum(0,&opretbuf[9],sizeof(flags),&flags);
    key = &opretbuf[13];
    if ( keylen+13 > opretlen )
    {
        static uint32_t counter;
        if ( ++counter < 1 )
            LogPrintf("komodo_kvupdate: keylen.%d + 13 > opretlen.%d, this can be ignored\n",keylen,opretlen);
        return;
    }
    valueptr = &key[keylen];
    fee = komodo_kvfee(flags,opretlen,keylen);
    //LogPrintf("fee %.8f vs %.8f flags.%d keylen.%d valuesize.%d height.%d (%02x %02x %02x) (%02x %02x %02x)\n",(double)fee/COIN,(double)value/COIN,flags,keylen,valuesize,height,key[0],key[1],key[2],valueptr[0],valueptr[1],valueptr[2]);
    if ( value >= fee )
    {
        coresize = (int32_t)(sizeof(flags)+sizeof(height)+sizeof(keylen)+sizeof(valuesize)+keylen+valuesize+1);
        if ( opretlen == coresize || opretlen == coresize+sizeof(uint256) || opretlen == coresize+2*sizeof(uint256) )
        {
            memset(&pubkey,0,sizeof(pubkey));
            memset(&sig,0,sizeof(sig));
            if ( (haspubkey= (opretlen >= coresize+sizeof(uint256))) != 0 )
            {
                for (i=0; i<32; i++)
                    ((uint8_t *)&pubkey)[i] = opretbuf[coresize+i];
            }
            if ( (hassig= (opretlen == coresize+sizeof(uint256)*2)) != 0 )
            {
                for (i=0; i<32; i++)
                    ((uint8_t *)&sig)[i] = opretbuf[coresize+sizeof(uint256)+i];
            }
            memcpy(keyvalue,key,keylen);
            if ( (refvaluesize= komodo_kvsearch((uint256 *)&refpubkey,height,&flags,&kvheight,&keyvalue[keylen],key,keylen)) >= 0 )
            {
                if ( memcmp(&zeroes,&refpubkey,sizeof(refpubkey)) != 0 )
                {
                    if ( komodo_kvsigverify(keyvalue,keylen+refvaluesize,refpubkey,sig) < 0 )
                    {
                        LogPrintf("komodo_kvsigverify error [%d]\n",coresize-13);
                        return;
                    }
                }
            }
            portable_mutex_lock(&KOMODO_KV_mutex);
            HASH_FIND(hh,KOMODO_KV,key,keylen,ptr);
            if ( ptr != 0 )
            {
                //if ( (ptr->flags & KOMODO_KVPROTECTED) != 0 )
                {
                    tstr = (char *)"transfer:";
                    transferpubstr = (char *)&valueptr[strlen(tstr)];
                    if ( strncmp(tstr,(char *)valueptr,strlen(tstr)) == 0 && is_hexstr(transferpubstr,0) == 64 )
                    {
                        LogPrintf("transfer.(%s) to [%s]? ishex.%d\n",key,transferpubstr,is_hexstr(transferpubstr,0));
                        for (i=0; i<32; i++)
                            ((uint8_t *)&pubkey)[31-i] = _decode_hex(&transferpubstr[i*2]);
                    }
                }
            }
            else if ( ptr == 0 )
            {
                ptr = (struct komodo_kv *)calloc(1,sizeof(*ptr));
                ptr->key = (uint8_t *)calloc(1,keylen);
                ptr->keylen = keylen;
                memcpy(ptr->key,key,keylen);
                newflag = 1;
                HASH_ADD_KEYPTR(hh,KOMODO_KV,ptr->key,ptr->keylen,ptr);
                //LogPrintf("KV add.(%s) (%s)\n",ptr->key,valueptr);
            }
            if ( newflag != 0 || (ptr->flags & KOMODO_KVPROTECTED) == 0 )
            {
                if ( ptr->value != 0 )
                    free(ptr->value), ptr->value = 0;
                if ( (ptr->valuesize= valuesize) != 0 )
                {
                    ptr->value = (uint8_t *)calloc(1,valuesize);
                    memcpy(ptr->value,valueptr,valuesize);
                }
            } else LogPrintf("newflag.%d zero or protected %d\n",newflag,(ptr->flags & KOMODO_KVPROTECTED));
            /*for (i=0; i<32; i++)
                LogPrintf("%02x",((uint8_t *)&ptr->pubkey)[i]);
            LogPrintf(" <- ");
            for (i=0; i<32; i++)
                LogPrintf("%02x",((uint8_t *)&pubkey)[i]);
            LogPrintf(" new pubkey\n");*/
            memcpy(&ptr->pubkey,&pubkey,sizeof(ptr->pubkey));
            ptr->height = height;
            ptr->flags = flags; // jl777 used to or in KVPROTECTED
            portable_mutex_unlock(&KOMODO_KV_mutex);
        } else error("size mismatch %d vs %d\n",opretlen,coresize);
    } else error("not enough fee\n");
}

#endif //komodo_kv__h
