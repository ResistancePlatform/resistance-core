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

/*
 z_exportkey "zaddr"
 z_exportwallet "filename"
 z_getoperationstatus (["operationid", ... ])
 z_gettotalbalance ( minconf )
 z_importkey "zkey" ( rescan )
 z_importwallet "filename"
 z_listaddresses
 z_sendmany "fromaddress" [{"address":... ,"amount":..., "memo":"<hex>"},...] ( minconf ) ( fee )
 */

#ifndef resistance_privatizer__h
#define resistance_privatizer__h

#ifdef _WIN32
#include <wincrypt.h>
#endif

#include "resistance_structs.h"
#include "resistance_globals.h"
#include "main.h"

static char *privatizer_issuemethod(char *userpass,char *method,char *params,uint16_t port)
{
    cJSON *retjson, *resultjson,*resjson = 0; char *retstr;
    if ( (retstr= resistance_issuemethod(userpass,method,params,port)) != 0 )
    {
        if ( (retjson= cJSON_Parse(retstr)) != 0 )
        {
            if ( (resultjson=jobj(retjson,(char *)"result")) != 0 )
                resjson = jduplicate(resultjson);
            else if ( (resultjson=jobj(retjson,(char *)"error")) != 0 )
                resjson = jduplicate(resultjson);
            else
            {
                resjson = cJSON_CreateObject();
                jaddstr(resjson,(char *)"error",(char *)"cant parse return");
            }
            free_json(retjson);
        }
        free(retstr);
    }
    if ( resjson != 0 )
        return(jprint(resjson,1));
    else return(clonestr((char *)"{\"error\":\"unknown error\"}"));
}

static char *privatizer_importaddress(char *address)
{
    char params[1024];
    sprintf(params,"[\"%s\", \"%s\", false]",address,address);
    return(privatizer_issuemethod(RESUSERPASS,(char *)"importaddress",params,BITCOIND_PORT));
}

static char *privatizer_validateaddress(char *addr)
{
    char params[1024];
    sprintf(params,"[\"%s\"]",addr);
    LogPrintf("validateaddress.%s\n",params);
    return(privatizer_issuemethod(RESUSERPASS,(char *)"validateaddress",params,BITCOIND_PORT));
}

static int32_t Privatizer_secretaddrfind(char *searchaddr)
{
    int32_t i;
    for (i=0; i<Privatizer_numsecretaddrs; i++)
    {
        if ( strcmp(searchaddr,Privatizer_secretaddrs[i]) == 0 )
            return(i);
    }
    return(-1);
}

static int32_t Privatizer_secretaddradd(char *secretaddr) // external
{
    int32_t ind;
    if ( secretaddr != 0 && secretaddr[0] != 0 )
    {
        if ( Privatizer_numsecretaddrs < PRIVATIZER_MAXSECRETADDRS )
        {
            if ( strcmp(Privatizer_deposit,secretaddr) != 0 )
            {
                if ( (ind= Privatizer_secretaddrfind(secretaddr)) < 0 )
                {
                    ind = Privatizer_numsecretaddrs++;
                    safecopy(Privatizer_secretaddrs[ind],secretaddr,64);
                }
                return(ind);
            } else return(PRIVATIZER_ERROR_SECRETCANTBEDEPOSIT);
        } else return(PRIVATIZER_ERROR_TOOMANYSECRETS);
    }
    else
    {
        memset(Privatizer_secretaddrs,0,sizeof(Privatizer_secretaddrs));
        Privatizer_numsecretaddrs = 0;
    }
    return(Privatizer_numsecretaddrs);
}

static int32_t Privatizer_depositaddradd(char *depositaddr) // external
{
    int32_t ind,retval = PRIVATIZER_ERROR_DUPLICATEDEPOSIT; char *retstr; cJSON *retjson,*ismine;
    if ( depositaddr == 0 )
        depositaddr = (char *)"";
    if ( (ind= Privatizer_secretaddrfind(depositaddr)) < 0 )
    {
        if ( (retstr= privatizer_validateaddress(depositaddr)) != 0 )
        {
            if ( (retjson= cJSON_Parse(retstr)) != 0 )
            {
                if ( (ismine= jobj(retjson,(char *)"ismine")) != 0 && cJSON_IsTrue(ismine) != 0 )
                {
                    retval = 0;
                    safecopy(Privatizer_deposit,depositaddr,sizeof(Privatizer_deposit));
                }
                else
                {
                    retval = PRIVATIZER_ERROR_NOTINWALLET;
                    LogPrintf("%s not in wallet: ismine.%p %d %s\n",depositaddr,ismine,cJSON_IsTrue(ismine),jprint(retjson,0));
                }
                free_json(retjson);
            }
            free(retstr);
        }
    }
    return(retval);
}

#ifdef _WIN32
static void OS_randombytes(unsigned char *x,long xlen)
{
    HCRYPTPROV prov = 0;
    CryptAcquireContextW(&prov, NULL, NULL,PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT);
    CryptGenRandom(prov, xlen, x);
    CryptReleaseContext(prov, 0);
}
#endif

static int32_t Privatizer_secretaddr(char *secretaddr)
{
    uint32_t r;
    if ( Privatizer_numsecretaddrs > 0 )
    {
        OS_randombytes((uint8_t *)&r,sizeof(r));
        r %= Privatizer_numsecretaddrs;
        safecopy(secretaddr,Privatizer_secretaddrs[r],64);
    }
    return(r);
}

static int32_t privatizer_addresstype(char *addr)
{
    if ( addr[0] == '"' && addr[strlen(addr)-1] == '"' )
    {
        addr[strlen(addr)-1] = 0;
        addr++;
    }
    if ( addr[0] == 'z' && addr[1] == 'r' && strlen(addr) >= 40 ) // mainnet
        return('z');
    else if ( addr[0] == 'z' && addr[1] == 't' && strlen(addr) >= 40 ) // testnet and regtest
        return('z');
    else if ( strlen(addr) < 40 )
        return('t');
    LogPrintf("strange.(%s)\n",addr);
    return(-1);
}

static struct privatizer_item *privatizer_opidfind(char *opid)
{
    struct privatizer_item *ptr;
    HASH_FIND(hh,Privatizers,opid,(int32_t)strlen(opid),ptr);
    return(ptr);
}

static struct privatizer_item *privatizer_opidadd(char *opid)
{
    struct privatizer_item *ptr = 0;
    if ( opid != 0 && (ptr= privatizer_opidfind(opid)) == 0 )
    {
        ptr = (struct privatizer_item *)calloc(1,sizeof(*ptr));
        safecopy(ptr->opid,opid,sizeof(ptr->opid));
        HASH_ADD_KEYPTR(hh,Privatizers,ptr->opid,(int32_t)strlen(ptr->opid),ptr);
        if ( ptr != privatizer_opidfind(opid) )
            LogPrintf("privatizer_opidadd.(%s) ERROR, couldnt find after add\n",opid);
    }
    return(ptr);
}

static char *privatizer_zgetnewaddress()
{
    char params[1024];
    sprintf(params,"[]");
    return(privatizer_issuemethod(RESUSERPASS,(char *)"z_getnewaddress",params,BITCOIND_PORT));
}

static char *privatizer_zlistoperationids()
{
    char params[1024];
    sprintf(params,"[]");
    return(privatizer_issuemethod(RESUSERPASS,(char *)"z_listoperationids",params,BITCOIND_PORT));
}

static char *privatizer_zgetoperationresult(char *opid)
{
    char params[1024];
    sprintf(params,"[[\"%s\"]]",opid);
    return(privatizer_issuemethod(RESUSERPASS,(char *)"z_getoperationresult",params,BITCOIND_PORT));
}

static char *privatizer_zgetoperationstatus(char *opid)
{
    char params[1024];
    sprintf(params,"[[\"%s\"]]",opid);
    return(privatizer_issuemethod(RESUSERPASS,(char *)"z_getoperationstatus",params,BITCOIND_PORT));
}

static char *privatizer_sendt_to_z(char *taddr,double total,char *zaddr,double amount)
{
    char params[1024]; double fee = ((amount-3*PRIVATIZER_TXFEE) * PRIVATIZER_FEE) * 1.5;
    double restAmount = total - amount;
    if ( privatizer_addresstype(zaddr) != 'z' || privatizer_addresstype(taddr) != 't' )
        return(clonestr((char *)"{\"error\":\"illegal address in t to z\"}"));
    if ( restAmount < 0 )
        return(clonestr((char *)"{\"error\":\"amount is incorrect in t to z\"}"));
    else if ( restAmount == 0 )
        sprintf(params,"[\"%s\", [{\"address\":\"%s\",\"amount\":%.8f}, {\"address\":\"%s\",\"amount\":%.8f}], 1, %.8f]",taddr,zaddr,amount-fee-PRIVATIZER_TXFEE,PRIVATIZER_ADDR,fee,PRIVATIZER_TXFEE);
    else
        sprintf(params,"[\"%s\", [{\"address\":\"%s\",\"amount\":%.8f}, {\"address\":\"%s\",\"amount\":%.8f}, {\"address\":\"%s\",\"amount\":%.8f}], 1, %.8f]",taddr,zaddr,amount-fee-PRIVATIZER_TXFEE,PRIVATIZER_ADDR,fee,taddr,restAmount,PRIVATIZER_TXFEE);
    LogPrintf("t -> z: %s\n",params);
    return(privatizer_issuemethod(RESUSERPASS,(char *)"z_sendmany",params,BITCOIND_PORT));
}

static char *privatizer_sendz_to_z(char *zaddrS,char *zaddrD,double amount)
{
    char params[1024]; double fee = (amount-2*PRIVATIZER_TXFEE) * PRIVATIZER_FEE;
    if ( privatizer_addresstype(zaddrS) != 'z' || privatizer_addresstype(zaddrD) != 'z' )
        return(clonestr((char *)"{\"error\":\"illegal address in z to z\"}"));
    //sprintf(params,"[\"%s\", [{\"address\":\"%s\",\"amount\":%.8f}, {\"address\":\"%s\",\"amount\":%.8f}], 1, %.8f]",zaddrS,zaddrD,amount-fee-PRIVATIZER_TXFEE,PRIVATIZER_ADDR,fee,PRIVATIZER_TXFEE);
    sprintf(params,"[\"%s\", [{\"address\":\"%s\",\"amount\":%.8f}], 1, %.8f]",zaddrS,zaddrD,amount-fee-PRIVATIZER_TXFEE,PRIVATIZER_TXFEE);
    LogPrintf("z -> z: %s\n",params);
    return(privatizer_issuemethod(RESUSERPASS,(char *)"z_sendmany",params,BITCOIND_PORT));
}

static char *privatizer_sendz_to_t(char *zaddr,char *taddr,double amount)
{
    char params[1024]; double fee = ((amount-PRIVATIZER_TXFEE) * PRIVATIZER_FEE) * 1.5;
    if ( privatizer_addresstype(zaddr) != 'z' || privatizer_addresstype(taddr) != 't' )
        return(clonestr((char *)"{\"error\":\"illegal address in z to t\"}"));
    sprintf(params,"[\"%s\", [{\"address\":\"%s\",\"amount\":%.8f}, {\"address\":\"%s\",\"amount\":%.8f}], 1, %.8f]",zaddr,taddr,amount-fee-PRIVATIZER_TXFEE,PRIVATIZER_ADDR,fee,PRIVATIZER_TXFEE);
    LogPrintf("z -> t: %s\n",params);
    return(privatizer_issuemethod(RESUSERPASS,(char *)"z_sendmany",params,BITCOIND_PORT));
}

static char *privatizer_zlistaddresses()
{
    char params[1024];
    sprintf(params,"[]");
    return(privatizer_issuemethod(RESUSERPASS,(char *)"z_listaddresses",params,BITCOIND_PORT));
}

static char *privatizer_zlistreceivedbyaddress(char *addr)
{
    char params[1024];
    sprintf(params,"[\"%s\", 1]",addr);
    return(privatizer_issuemethod(RESUSERPASS,(char *)"z_listreceivedbyaddress",params,BITCOIND_PORT));
}

static char *privatizer_getreceivedbyaddress(char *addr)
{
    char params[1024];
    sprintf(params,"[\"%s\", 1]",addr);
    return(privatizer_issuemethod(RESUSERPASS,(char *)"getreceivedbyaddress",params,BITCOIND_PORT));
}

static char *privatizer_importprivkey(char *wifstr)
{
    char params[1024];
    sprintf(params,"[\"%s\", \"\", false]",wifstr);
    return(privatizer_issuemethod(RESUSERPASS,(char *)"importprivkey",params,BITCOIND_PORT));
}

static char *privatizer_zgetbalance(char *addr)
{
    char params[1024];
    sprintf(params,"[\"%s\", 1]",addr);
    return(privatizer_issuemethod(RESUSERPASS,(char *)"z_getbalance",params,BITCOIND_PORT));
}

static char *privatizer_listunspent(char *coinaddr)
{
    char params[1024];
    sprintf(params,"[1, 99999999, [\"%s\"]]",coinaddr);
    return(privatizer_issuemethod(RESUSERPASS,(char *)"listunspent",params,BITCOIND_PORT));
}

static char *privatizer_gettransaction(char *txidstr)
{
    char params[1024];
    sprintf(params,"[\"%s\", 1]",txidstr);
    return(privatizer_issuemethod(RESUSERPASS,(char *)"getrawtransaction",params,BITCOIND_PORT));
}

static int32_t privatizer_numvins(bits256 txid)
{
    char txidstr[65],params[1024],*retstr; cJSON *retjson,*vins; int32_t n,numvins = -1;
    bits256_str(txidstr,txid);
    if ( (retstr= privatizer_gettransaction(txidstr)) != 0 )
    {
        if ( (retjson= cJSON_Parse(retstr)) != 0 )
        {
            if ( jobj(retjson,(char *)"vin") != 0 && ((vins= jarray(&n,retjson,(char *)"vin")) == 0 || n == 0) )
            {
                numvins = n;
                //LogPrintf("numvins.%d\n",n);
            } //else LogPrintf("no vin.(%s)\n",retstr);
            free_json(retjson);
        }
        free(retstr);
    }
    return(numvins);
}

static int64_t privatizer_receivedby(char *addr)
{
    char *retstr; int64_t total = 0;
    if ( (retstr= privatizer_getreceivedbyaddress(addr)) != 0 )
    {
        total = atof(retstr) * SATOSHIDEN;
        free(retstr);
    }
    return(total);
}

static int64_t privatizer_balance(char *addr)
{
    char *retstr; double val; int64_t balance = 0; //cJSON *retjson; int32_t i,n;
    /*if ( privatizer_addresstype(addr) == 't' )
    {
        if ( (retstr= privatizer_listunspent(addr)) != 0 )
        {
            //LogPrintf("privatizer.[%s].(%s)\n","RES",retstr);
            if ( (retjson= cJSON_Parse(retstr)) != 0 )
            {
                if ( (n= cJSON_GetArraySize(retjson)) > 0 && cJSON_IsArray(retjson) != 0 )
                    for (i=0; i<n; i++)
                        balance += SATOSHIDEN * jdouble(jitem(retjson,i),(char *)"amount");
                free_json(retjson);
            }
            free(retstr);
        }
    }
    else*/ if ( (retstr= privatizer_zgetbalance(addr)) != 0 )
    {
        if ( (val= atof(retstr)) > SMALLVAL )
            balance = val * SATOSHIDEN;
        free(retstr);
    }
    return(balance);
}

static int32_t privatizer_itemset(struct privatizer_item *ptr,cJSON *item,char *status)
{
    cJSON *params,*amounts,*dest; char *from,*addr; int32_t i,n; int64_t amount;
    /*"params" : {
     "fromaddress" : "RDhEGYScNQYetCyG75Kf8Fg61UWPdwc1C5",
     "amounts" : [
     {
     "address" : "zc9s3UdkDFTnnwHrMCr1vYy2WmkjhmTxXNiqC42s7BjeKBVUwk766TTSsrRPKfnX31Bbu8wbrTqnjDqskYGwx48FZMPHvft",
     "amount" : 3.00000000
     }
     ],
     "minconf" : 1,
     "fee" : 0.00010000
     }*/
    if ( (params= jobj(item,(char *)"params")) != 0 )
    {
        //LogPrintf("params.(%s)\n",jprint(params,0));
        if ( (from= jstr(params,(char *)"fromaddress")) != 0 )
        {
            safecopy(ptr->src,from,sizeof(ptr->src));
        }
        if ( (amounts= jarray(&n,params,(char *)"amounts")) != 0 )
        {
            for (i=0; i<n; i++)
            {
                dest = jitem(amounts,i);
                //LogPrintf("%s ",jprint(dest,0));
                if ( (addr= jstr(dest,(char *)"address")) != 0 && (amount= jdouble(dest,(char *)"amount")*SATOSHIDEN) > 0 )
                {
                    if ( strcmp(addr,PRIVATIZER_ADDR) == 0 )
                        ptr->fee = amount;
                    else if ( strcmp(addr,ptr->src) != 0 )
                    {
                        ptr->amount = amount;
                        safecopy(ptr->dest,addr,sizeof(ptr->dest));
                    }
                }
            }
        }
        ptr->txfee = jdouble(params,(char *)"fee") * SATOSHIDEN;
    }
    return(1);
}

static void privatizer_opidupdate(struct privatizer_item *ptr)
{
    char *retstr,*status; cJSON *retjson,*item;
    if ( ptr->status == 0 )
    {
        if ( (retstr= privatizer_zgetoperationstatus(ptr->opid)) != 0 )
        {
            if ( (retjson= cJSON_Parse(retstr)) != 0 )
            {
                if ( cJSON_GetArraySize(retjson) == 1 && cJSON_IsArray(retjson) != 0 )
                {
                    item = jitem(retjson,0);
                    //LogPrintf("%s\n",jprint(item,0));
                    if ( (status= jstr(item,(char *)"status")) != 0 )
                    {
                        if ( strcmp(status,(char *)"success") == 0 )
                        {
                            ptr->status = privatizer_itemset(ptr,item,status);
                            if ( (privatizer_addresstype(ptr->src) == 't' && privatizer_addresstype(ptr->dest) == 'z' && strcmp(ptr->src,Privatizer_deposit) != 0) || 
                                 (privatizer_addresstype(ptr->src) == 'z' && privatizer_addresstype(ptr->dest) == 't' && Privatizer_secretaddrfind(ptr->dest) < 0) )
                            {
                                LogPrintf("a non-privatizer t->z pruned\n");
                                free(privatizer_zgetoperationresult(ptr->opid));
                                ptr->status = -1;
                            }

                        }
                        else if ( strcmp(status,(char *)"failed") == 0 )
                        {
                            LogPrintf("privatizer_opidupdate %s failed\n",ptr->opid);
                            free(privatizer_zgetoperationresult(ptr->opid));
                            ptr->status = -1;
                        }
                    }
                }
                free_json(retjson);
            }
            free(retstr);
        }
    }
}

static void privatizer_prune(struct privatizer_item *ptr)
{
    struct privatizer_item *tmp; char oldsrc[128]; int32_t flag = 1;
    if ( is_hexstr(ptr->opid,0) == 64 )
        return;
    LogPrintf("privatizer_prune %s\n",ptr->opid);
    strcpy(oldsrc,ptr->src);
    free(privatizer_zgetoperationresult(ptr->opid));
    while ( flag != 0 )
    {
        flag = 0;
        HASH_ITER(hh,Privatizers,ptr,tmp)
        {
            if ( strcmp(oldsrc,ptr->dest) == 0 )
            {
                if ( is_hexstr(ptr->opid,0) != 64 )
                {
                    LogPrintf("privatizer_prune %s (%s -> %s) matched oldsrc\n",ptr->opid,ptr->src,ptr->dest);
                    free(privatizer_zgetoperationresult(ptr->opid));
                    strcpy(oldsrc,ptr->src);
                    flag = 1;
                    break;
                }
            }
        }
    }
}


bits256 jbits256(cJSON *json,char *field);


static void privatizer_zaddrinit(char *zaddr)
{
    struct privatizer_item *ptr; char *retstr,*totalstr; cJSON *item,*array; double total; bits256 txid; char txidstr[65],t_z,z_z;
    if ( (totalstr= privatizer_zgetbalance(zaddr)) != 0 )
    {
        if ( (total= atof(totalstr)) > SMALLVAL )
        {
            if ( (retstr= privatizer_zlistreceivedbyaddress(zaddr)) != 0 )
            {
                if ( (array= cJSON_Parse(retstr)) != 0 )
                {
                    t_z = z_z = 0;
                    if ( cJSON_GetArraySize(array) == 1 && cJSON_IsArray(array) != 0 )
                    {
                        item = jitem(array,0);
                        if ( (uint64_t)((total+0.0000000049) * SATOSHIDEN) == (uint64_t)((jdouble(item,(char *)"amount")+0.0000000049) * SATOSHIDEN) )
                        {
                            txid = jbits256(item,(char *)"txid");
                            bits256_str(txidstr,txid);
                            if ( (ptr= privatizer_opidadd(txidstr)) != 0 )
                            {
                                ptr->amount = (total * SATOSHIDEN);
                                ptr->status = 1;
                                strcpy(ptr->dest,zaddr);
                                if ( privatizer_addresstype(ptr->dest) != 'z' )
                                    LogPrintf("error setting dest type to Z: %s\n",jprint(item,0));
                                if ( privatizer_numvins(txid) == 0 )
                                {
                                    z_z = 1;
                                    strcpy(ptr->src,zaddr);
                                    ptr->src[3] = '0';
                                    ptr->src[4] = '0';
                                    ptr->src[5] = '0';
                                    if ( privatizer_addresstype(ptr->src) != 'z' )
                                        LogPrintf("error setting address type to Z: %s\n",jprint(item,0));
                                }
                                else
                                {
                                    t_z = 1;
                                    strcpy(ptr->src,"taddr");
                                    if ( privatizer_addresstype(ptr->src) != 't' )
                                        LogPrintf("error setting address type to T: %s\n",jprint(item,0));
                                }
                                LogPrintf("%s %s %.8f t_z.%d z_z.%d\n",zaddr,txidstr,total,t_z,z_z); // cant be z->t from spend
                            }
                        } else LogPrintf("mismatched %s %s total %.8f vs %.8f -> %lld\n",zaddr,totalstr,dstr(SATOSHIDEN * total),dstr(SATOSHIDEN * jdouble(item,(char *)"amount")),(long long)((uint64_t)(total * SATOSHIDEN) - (uint64_t)(jdouble(item,(char *)"amount") * SATOSHIDEN)));
                    }
                    free_json(array);
                }
                free(retstr);
            }
        }
        free(totalstr);
    }
}

static void privatizer_opidsupdate()
{
    char *retstr; cJSON *array; int32_t i,n; struct privatizer_item *ptr;
    if ( (retstr= privatizer_zlistoperationids()) != 0 )
    {
        if ( (array= cJSON_Parse(retstr)) != 0 )
        {
            if ( (n= cJSON_GetArraySize(array)) > 0 && cJSON_IsArray(array) != 0 )
            {
                //LogPrintf("%s -> n%d\n",retstr,n);
                for (i=0; i<n; i++)
                {
                    if ( (ptr= privatizer_opidadd(jstri(array,i))) != 0 )
                    {
                        if ( ptr->status == 0 )
                            privatizer_opidupdate(ptr);
                        //LogPrintf("privatizer_opidsupdate %d: %d %s -> %s %.8f\n",ptr->status,ptr->spent,ptr->src,ptr->dest,dstr(ptr->amount));
                        if ( privatizer_addresstype(ptr->src) == 'z' && privatizer_addresstype(ptr->dest) == 't' )
                            privatizer_prune(ptr);
                    }
                }
            }
            free_json(array);
        }
        free(retstr);
    }
}

static uint64_t privatizer_increment(uint8_t r,int32_t height,uint64_t total,uint64_t biggest,uint64_t medium, uint64_t smallest)
{
    int32_t i,n; uint64_t incrs[1000],remains = total;
    height /= PRIVATIZER_SYNCHRONIZED_BLOCKS;
    if ( (height % PRIVATIZER_SYNCHRONIZED_BLOCKS) == 0 || total >= 100*biggest )
    {
        if ( total >= biggest )
            return(biggest);
        else if ( total >= medium )
            return(medium);
        else if ( total >= smallest )
            return(smallest);
        else return(0);
    }
    else
    {
        n = 0;
        while ( remains > smallest && n < sizeof(incrs)/sizeof(*incrs) )
        {
            if ( remains >= biggest )
                incrs[n] = biggest;
            else if ( remains >= medium )
                incrs[n] = medium;
            else if ( remains >= smallest )
                incrs[n] = smallest;
            else break;
            remains -= incrs[n];
            n++;
        }
        if ( n > 0 )
        {
            r %= n;
            for (i=0; i<n; i++)
                LogPrintf("%.8f ",dstr(incrs[i]));
            LogPrintf("n.%d incrs r.%d -> %.8f\n",n,r,dstr(incrs[r]));
            return(incrs[r]);
        }
    }
    return(0);
}

static void privatizer_iteration()
{
    static int32_t lastheight; static uint32_t lasttime;
    char *zaddr,*addr,*retstr,secretaddr[64]; cJSON *array; int32_t i,iter,height,counter,chosen_one,n; uint64_t smallest,medium,biggest,amount=0,total=0; double fee; struct privatizer_item *ptr,*tmp; uint16_t r,s;
    if ( PRIVATIZER_PAUSE != 0 )
        return;
    if ( lasttime == 0 )
    {
        if ( (retstr= privatizer_zlistaddresses()) != 0 )
        {
            if ( (array= cJSON_Parse(retstr)) != 0 )
            {
                if ( (n= cJSON_GetArraySize(array)) > 0 && cJSON_IsArray(array) != 0 )
                {
                    for (i=0; i<n; i++)
                        privatizer_zaddrinit(jstri(array,i));
                }
                free_json(array);
            }
            free(retstr);
        }
    }
    height = (int32_t)chainActive.Tip()->nHeight;
    if ( time(NULL) < lasttime+20 )
        return;
    lasttime = (uint32_t)time(NULL);
    if ( lastheight == height )
        return;
    lastheight = height;
    //if ( (height % PRIVATIZER_SYNCHRONIZED_BLOCKS) != PRIVATIZER_SYNCHRONIZED_BLOCKS-3 )
    //    return;
    fee = PRIVATIZER_INCR * PRIVATIZER_FEE;
    smallest = SATOSHIDEN * ((PRIVATIZER_INCR + 3*fee) + 3*PRIVATIZER_TXFEE);
    medium = SATOSHIDEN * ((PRIVATIZER_INCR + 3*fee)*10 + 3*PRIVATIZER_TXFEE);
    biggest = SATOSHIDEN * ((PRIVATIZER_INCR + 3*fee)*777 + 3*PRIVATIZER_TXFEE);
    OS_randombytes((uint8_t *)&r,sizeof(r));
    s = (r % 3);
    //LogPrintf("privatizer_iteration r.%u s.%u\n",r,s);
    switch ( s )
    {
        case 0: // t -> z
        default:
            if ( Privatizer_deposit[0] != 0 && (total= privatizer_balance(Privatizer_deposit)) >= smallest )
            {
                if ( (zaddr= privatizer_zgetnewaddress()) != 0 )
                {
                    if ( zaddr[0] == '"' && zaddr[strlen(zaddr)-1] == '"' )
                    {
                        zaddr[strlen(zaddr)-1] = 0;
                        addr = zaddr+1;
                    } else addr = zaddr;
                    amount = privatizer_increment(r/3,height,total,biggest,medium,smallest);
                /*
                    amount = 0;
                    if ( (height % (PRIVATIZER_SYNCHRONIZED_BLOCKS*PRIVATIZER_SYNCHRONIZED_BLOCKS)) == 0 && total >= SATOSHIDEN * ((PRIVATIZER_INCR + 3*fee)*100 + 3*PRIVATIZER_TXFEE) )
                    amount = SATOSHIDEN * ((PRIVATIZER_INCR + 3*fee)*100 + 3*PRIVATIZER_TXFEE);
                    else if ( (r & 3) == 0 && total >= SATOSHIDEN * ((PRIVATIZER_INCR + 3*fee)*10 + 3*PRIVATIZER_TXFEE) )
                        amount = SATOSHIDEN * ((PRIVATIZER_INCR + 3*fee)*10 + 3*PRIVATIZER_TXFEE);
                    else amount = SATOSHIDEN * ((PRIVATIZER_INCR + 3*fee) + 3*PRIVATIZER_TXFEE);*/
                    if ( amount > 0 && (retstr= privatizer_sendt_to_z(Privatizer_deposit,dstr(total),addr,dstr(amount))) != 0 )
                    {
                        LogPrintf("sendt_to_z.(%s)\n",retstr);
                        free(retstr);
                    }
                    free(zaddr);
                } else LogPrintf("no zaddr from privatizer_zgetnewaddress\n");
            }
            else if ( Privatizer_deposit[0] != 0 )
                LogPrintf("%s total %.8f vs %.8f\n",Privatizer_deposit,dstr(total),(PRIVATIZER_INCR + 3*(fee+PRIVATIZER_TXFEE)));
            break;
        case 1: // z -> z
            privatizer_opidsupdate();
            chosen_one = -1;
            for (iter=counter=0; iter<2; iter++)
            {
                counter = n = 0;
                HASH_ITER(hh,Privatizers,ptr,tmp)
                {
                    //LogPrintf("privatizer_iteration %d: %d %s -> %s %.8f\n",ptr->status,ptr->spent,ptr->src,ptr->dest,dstr(ptr->amount));
                    if ( ptr->spent == 0 && ptr->status > 0 && privatizer_addresstype(ptr->src) == 't' && privatizer_addresstype(ptr->dest) == 'z' )
                    {
                        if ( (total= privatizer_balance(ptr->dest)) >= (fee + PRIVATIZER_FEE)*SATOSHIDEN )
                        {
                            if ( iter == 1 && counter == chosen_one )
                            {
                                if ( (zaddr= privatizer_zgetnewaddress()) != 0 )
                                {
                                    if ( zaddr[0] == '"' && zaddr[strlen(zaddr)-1] == '"' )
                                    {
                                        zaddr[strlen(zaddr)-1] = 0;
                                        addr = zaddr+1;
                                    } else addr = zaddr;
                                    if ( (retstr= privatizer_sendz_to_z(ptr->dest,addr,dstr(total))) != 0 )
                                    {
                                        LogPrintf("n.%d counter.%d chosen_one.%d send z_to_z.(%s)\n",n,counter,chosen_one,retstr);
                                        free(retstr);
                                    }
                                    ptr->spent = (uint32_t)time(NULL);
                                    free(zaddr);
                                    break;
                                }
                            }
                            counter++;
                        }
                    }
                    n++;
                }
                if ( counter == 0 )
                    break;
                if ( iter == 0 )
                {
                    OS_randombytes((uint8_t *)&chosen_one,sizeof(chosen_one));
                    if ( chosen_one < 0 )
                        chosen_one = -chosen_one;
                    chosen_one %= counter;
                    LogPrintf("privatizer z->z chosen_one.%d of %d, from %d\n",chosen_one,counter,n);
                }
            }
            break;
        case 2: // z -> t
            if ( Privatizer_numsecretaddrs > 0 )
            {
                privatizer_opidsupdate();
                chosen_one = -1;
                for (iter=0; iter<2; iter++)
                {
                    counter = n = 0;
                    HASH_ITER(hh,Privatizers,ptr,tmp)
                    {
                        //LogPrintf("status.%d %c %c %.8f\n",ptr->status,privatizer_addresstype(ptr->src),privatizer_addresstype(ptr->dest),dstr(ptr->amount));
                        if ( ptr->spent == 0 && ptr->status > 0 && privatizer_addresstype(ptr->src) == 'z' && privatizer_addresstype(ptr->dest) == 'z' )
                        {
                            if ( (total= privatizer_balance(ptr->dest)) >= (fee + PRIVATIZER_FEE)*SATOSHIDEN )
                            {
                                if ( iter == 1 && counter == chosen_one )
                                {
                                    Privatizer_secretaddr(secretaddr);
                                    if ( (retstr= privatizer_sendz_to_t(ptr->dest,secretaddr,dstr(total))) != 0 )
                                    {
                                        LogPrintf("%s send z_to_t.(%s)\n",secretaddr,retstr);
                                        free(retstr);
                                    } else LogPrintf("null return from privatizer_sendz_to_t\n");
                                    ptr->spent = (uint32_t)time(NULL);
                                    break;
                                }
                                counter++;
                            } //else LogPrintf("z->t spent.%u total %.8f error\n",ptr->spent,dstr(total));
                        }
                        n++;
                    }
                    if ( counter == 0 )
                        break;
                    if ( iter == 0 )
                    {
                        OS_randombytes((uint8_t *)&chosen_one,sizeof(chosen_one));
                        if ( chosen_one < 0 )
                            chosen_one = -chosen_one;
                        chosen_one %= counter;
                        LogPrintf("privatizer z->t chosen_one.%d of %d, from %d\n",chosen_one,counter,n);
                    } //else LogPrintf("n.%d counter.%d chosen.%d\n",n,counter,chosen_one);
                }
            }
            break;
    }
}

#endif //resistance_privatizer__h
