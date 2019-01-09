/******************************************************************************
 * Copyright © 2014-2018 The SuperNET Developers.                             *
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

#ifndef resistance_events__h
#define resistance_events__h

#include "resistance_defs.h"
#include "resistance_bitcoind.h"
#include "resistance_notary.h"
#include "resistance_pax.h"
#include "resistance_gateway.h"

static struct resistance_event *resistance_eventadd(struct resistance_state *sp,int32_t height,char *symbol,uint8_t type,uint8_t *data,uint16_t datalen)
{
    struct resistance_event *ep=0; uint16_t len = (uint16_t)(sizeof(*ep) + datalen);
    if ( sp != 0 && ASSETCHAINS_SYMBOL[0] != 0 )
    {
        portable_mutex_lock(&resistance_mutex);
        ep = (struct resistance_event *)calloc(1,len);
        ep->len = len;
        ep->height = height;
        ep->type = type;
        strcpy(ep->symbol,symbol);
        if ( datalen != 0 )
            memcpy(ep->space,data,datalen);
        sp->Resistance_events = (struct resistance_event **)realloc(sp->Resistance_events,(1 + sp->Resistance_numevents) * sizeof(*sp->Resistance_events));
        sp->Resistance_events[sp->Resistance_numevents++] = ep;
        portable_mutex_unlock(&resistance_mutex);
    }
    return(ep);
}

static void resistance_eventadd_notarized(struct resistance_state *sp,char *symbol,int32_t height,char *dest,uint256 notarized_hash,uint256 notarized_desttxid,int32_t notarizedheight)//,uint256 MoM,int32_t MoMdepth)
{
    struct resistance_event_notarized N;
    if ( NOTARY_PUBKEY33[0] != 0 && resistance_verifynotarization(symbol,dest,height,notarizedheight,notarized_hash,notarized_desttxid) != 0 )
    {
        if ( height > 50000 || ASSETCHAINS_SYMBOL[0] != 0 )
            error("[%s] error validating notarization ht.%d notarized_height.%d, if on a pruned %s node this can be ignored\n",ASSETCHAINS_SYMBOL,height,notarizedheight,dest);
    }
    else
    {
        if ( 0 && ASSETCHAINS_SYMBOL[0] != 0 )
            LogPrintf("validated [%s] ht.%d notarized %d\n",ASSETCHAINS_SYMBOL,height,notarizedheight);
        memset(&N,0,sizeof(N));
        N.blockhash = notarized_hash;
        N.desttxid = notarized_desttxid;
        N.notarizedheight = notarizedheight;
        //N.MoM = MoM;
        //N.MoMdepth = MoMdepth;
        strncpy(N.dest,dest,sizeof(N.dest)-1);
        resistance_eventadd(sp,height,symbol,RESISTANCE_EVENT_NOTARIZED,(uint8_t *)&N,sizeof(N));
        if ( sp != 0 )
            resistance_notarized_update(sp,height,notarizedheight,notarized_hash,notarized_desttxid);//,MoM,MoMdepth);
    }
}

static void resistance_eventadd_pubkeys(struct resistance_state *sp,char *symbol,int32_t height,uint8_t num,uint8_t pubkeys[64][33])
{
    struct resistance_event_pubkeys P;
    //LogPrintf("eventadd pubkeys ht.%d\n",height);
    memset(&P,0,sizeof(P));
    P.num = num;
    memcpy(P.pubkeys,pubkeys,33 * num);
    resistance_eventadd(sp,height,symbol,RESISTANCE_EVENT_RATIFY,(uint8_t *)&P,(int32_t)(sizeof(P.num) + 33 * num));
    if ( sp != 0 )
        resistance_notarysinit(height,pubkeys,num);
}

static void resistance_eventadd_pricefeed(struct resistance_state *sp,char *symbol,int32_t height,uint32_t *prices,uint8_t num)
{
    struct resistance_event_pricefeed F;
    if ( num == sizeof(F.prices)/sizeof(*F.prices) )
    {
        memset(&F,0,sizeof(F));
        F.num = num;
        memcpy(F.prices,prices,sizeof(*F.prices) * num);
        resistance_eventadd(sp,height,symbol,RESISTANCE_EVENT_PRICEFEED,(uint8_t *)&F,(int32_t)(sizeof(F.num) + sizeof(*F.prices) * num));
        if ( sp != 0 )
            resistance_pvals(height,prices,num);
    } //else LogPrintf("skip pricefeed[%d]\n",num);
}

static void resistance_eventadd_opreturn(struct resistance_state *sp,char *symbol,int32_t height,uint256 txid,uint64_t value,uint16_t vout,uint8_t *buf,uint16_t opretlen)
{
    struct resistance_event_opreturn O; uint8_t *opret;
    if ( ASSETCHAINS_SYMBOL[0] != 0 )
    {
        opret = (uint8_t *)calloc(1,sizeof(O) + opretlen + 16);
        O.txid = txid;
        O.value = value;
        O.vout = vout;
        memcpy(opret,&O,sizeof(O));
        memcpy(&opret[sizeof(O)],buf,opretlen);
        O.oplen = (int32_t)(opretlen + sizeof(O));
        resistance_eventadd(sp,height,symbol,RESISTANCE_EVENT_OPRETURN,opret,O.oplen);
        free(opret);
        if ( sp != 0 )
            resistance_opreturn(height,value,buf,opretlen,txid,vout,symbol);
    }
}

static void resistance_event_undo(struct resistance_state *sp,struct resistance_event *ep)
{
    switch ( ep->type )
    {
        case RESISTANCE_EVENT_RATIFY: LogPrintf("rewind of ratify, needs to be coded.%d\n",ep->height); break;
        case RESISTANCE_EVENT_NOTARIZED: break;
        case RESISTANCE_EVENT_RESHEIGHT:
            if ( ep->height <= sp->SAVEDHEIGHT )
                sp->SAVEDHEIGHT = ep->height;
            break;
        case RESISTANCE_EVENT_PRICEFEED:
            // backtrack prices;
            break;
        case RESISTANCE_EVENT_OPRETURN:
            // backtrack opreturns
            break;
    }
}

static void resistance_event_rewind(struct resistance_state *sp,char *symbol,int32_t height)
{
    struct resistance_event *ep;
    if ( sp != 0 )
    {
        if ( ASSETCHAINS_SYMBOL[0] == 0 && height <= RESISTANCE_LASTMINED && prevRESISTANCE_LASTMINED != 0 )
        {
            LogPrintf("undo RESISTANCE_LASTMINED %d <- %d\n",RESISTANCE_LASTMINED,prevRESISTANCE_LASTMINED);
            RESISTANCE_LASTMINED = prevRESISTANCE_LASTMINED;
            prevRESISTANCE_LASTMINED = 0;
        }
        while ( sp->Resistance_events != 0 && sp->Resistance_numevents > 0 )
        {
            if ( (ep= sp->Resistance_events[sp->Resistance_numevents-1]) != 0 )
            {
                if ( ep->height < height )
                    break;
                //LogPrintf("[%s] undo %s event.%c ht.%d for rewind.%d\n",ASSETCHAINS_SYMBOL,symbol,ep->type,ep->height,height);
                resistance_event_undo(sp,ep);
                sp->Resistance_numevents--;
            }
        }
    }
}

static void resistance_setresheight(struct resistance_state *sp,int32_t resheight,uint32_t timestamp)
{
    if ( sp != 0 )
    {
        if ( resheight > sp->SAVEDHEIGHT )
        {
            sp->SAVEDHEIGHT = resheight;
            sp->SAVEDTIMESTAMP = timestamp;
        }
        if ( resheight > sp->CURRENT_HEIGHT )
            sp->CURRENT_HEIGHT = resheight;
    }
}

static void resistance_eventadd_resheight(struct resistance_state *sp,char *symbol,int32_t height,int32_t resheight,uint32_t timestamp)
{
    uint32_t buf[2];
    if ( resheight > 0 )
    {
        buf[0] = (uint32_t)resheight;
        buf[1] = timestamp;
        resistance_eventadd(sp,height,symbol,RESISTANCE_EVENT_RESHEIGHT,(uint8_t *)buf,sizeof(buf));
        if ( sp != 0 )
            resistance_setresheight(sp,resheight,timestamp);
    }
    else
    {
        //LogPrintf("REWIND resheight.%d\n",resheight);
        resheight = -resheight;
        resistance_eventadd(sp,height,symbol,RESISTANCE_EVENT_REWIND,(uint8_t *)&height,sizeof(height));
        if ( sp != 0 )
            resistance_event_rewind(sp,symbol,height);
    }
}

#endif //resistance_events__h
