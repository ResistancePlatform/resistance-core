
#ifndef komodo_bitcoind__h
#define komodo_bitcoind__h

#include "komodo_defs.h"
#include "komodo_globals.h"
#include "komodo_utils.h"

static int32_t komodo_verifynotarizedscript(int32_t height,uint8_t *script,int32_t len,uint256 NOTARIZED_HASH)
{
    int32_t i; uint256 hash; char params[256];
    for (i=0; i<32; i++)
        ((uint8_t *)&hash)[i] = script[2+i];
    if ( hash == NOTARIZED_HASH )
        return(0);
    for (i=0; i<32; i++)
        LogPrintf("%02x",((uint8_t *)&NOTARIZED_HASH)[i]);
    LogPrintf(" notarized, ");
    for (i=0; i<32; i++)
        LogPrintf("%02x",((uint8_t *)&hash)[i]);
    LogPrintf(" opreturn from [%s] ht.%d MISMATCHED\n",ASSETCHAINS_SYMBOL,height);
    return(-1);
}

static int32_t komodo_verifynotarization(char *symbol,char *dest,int32_t height,int32_t NOTARIZED_HEIGHT,uint256 NOTARIZED_HASH,uint256 NOTARIZED_DESTTXID)
{
    char params[256],*jsonstr,*hexstr; uint8_t *script,_script[8192]; int32_t n,len,retval = -1; cJSON *json,*txjson,*vouts,*vout,*skey;
    script = _script;
    sprintf(params,"[\"%s\", 1]",NOTARIZED_DESTTXID.ToString().c_str());
    if ( strcmp(symbol,ASSETCHAINS_SYMBOL[0]==0?(char *)"RES":ASSETCHAINS_SYMBOL) != 0 )
        return(0);
    if ( 0 && ASSETCHAINS_SYMBOL[0] != 0 )
        LogPrintf("[%s] src.%s dest.%s params.[%s] ht.%d notarized.%d\n",ASSETCHAINS_SYMBOL,symbol,dest,params,height,NOTARIZED_HEIGHT);
    if ( strcmp(dest,"RES") == 0 )
    {
        if ( RESUSERPASS[0] != 0 )
        {
            if ( ASSETCHAINS_SYMBOL[0] != 0 )
            {
                jsonstr = komodo_issuemethod(RESUSERPASS,(char *)"getrawtransaction",params,RES_PORT);
                //LogPrintf("userpass.(%s) got (%s)\n",RESUSERPASS,jsonstr);
            }
        }//else jsonstr = _dex_getrawtransaction();
        else return(0); // need universal way to issue DEX* API, since notaries mine most blocks, this ok
    }
    else if ( strcmp(dest,"BTC") == 0 )
    {
        if ( BTCUSERPASS[0] != 0 )
        {
            //LogPrintf("BTCUSERPASS.(%s)\n",BTCUSERPASS);
            jsonstr = komodo_issuemethod(BTCUSERPASS,(char *)"getrawtransaction",params,8332);
        }
        //else jsonstr = _dex_getrawtransaction();
        else return(0);
    }
    else
    {
        LogPrintf("[%s] verifynotarization error unexpected dest.(%s)\n",ASSETCHAINS_SYMBOL,dest);
        return(-1);
    }
    if ( jsonstr != 0 )
    {
        if ( (json= cJSON_Parse(jsonstr)) != 0 )
        {
            if ( (txjson= jobj(json,(char *)"result")) != 0 && (vouts= jarray(&n,txjson,(char *)"vout")) != 0 )
            {
                vout = jitem(vouts,n-1);
                if ( 0 && ASSETCHAINS_SYMBOL[0] != 0 )
                    LogPrintf("vout.(%s)\n",jprint(vout,0));
                if ( (skey= jobj(vout,(char *)"scriptPubKey")) != 0 )
                {
                    if ( (hexstr= jstr(skey,(char *)"hex")) != 0 )
                    {
                        //LogPrintf("HEX.(%s) vs hash.%s\n",hexstr,NOTARIZED_HASH.ToString().c_str());
                        len = strlen(hexstr) >> 1;
                        decode_hex(script,len,hexstr);
                        if ( script[1] == 0x4c )
                        {
                            script++;
                            len--;
                        }
                        else if ( script[1] == 0x4d )
                        {
                            script += 2;
                            len -= 2;
                        }
                        retval = komodo_verifynotarizedscript(height,script,len,NOTARIZED_HASH);
                    }
                }
            }
            free_json(txjson);
        }
        free(jsonstr);
    }
    return(retval);
}

static uint64_t komodo_seed(int32_t height)
{
    uint64_t seed = 0;

    seed = (height << 13) ^ (height << 2);
    seed <<= 21;
    seed |= (height & 0xffffffff);
    seed ^= (seed << 17) ^ (seed << 1);

    return(seed);
}

#endif //komodo_bitcoind__h
