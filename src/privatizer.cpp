// Copyright (c) 2019 The Resistance Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "privatizer.h"

#include "amount.h"
#include "init.h"
#include "key_io.h"
#include "main.h"
#include "util.h"
#include "utilmoneystr.h"
#include "wallet/wallet.h"

#include <boost/thread.hpp>

using namespace std;

#define PRIVATIZER_MAX_COUNT 10
#define PRIVATIZER_MIN_WORK_AMOUNT COIN
#define PRIVATIZER_TX_FEE 10000
#define PRIVATIZER_MIN_DELAY_T_TO_Z 0
#define PRIVATIZER_MAX_DELAY_T_TO_Z 0
#define PRIVATIZER_MIN_DELAY_Z_TO_Z 2
#define PRIVATIZER_MAX_DELAY_Z_TO_Z 6
#define PRIVATIZER_MIN_DELAY_Z_TO_T 2
#define PRIVATIZER_MAX_DELAY_Z_TO_T 6

extern CAmount getBalanceTaddr(std::string transparentAddress, int minDepth, bool ignoreUnspendable);
extern CAmount getBalanceZaddr(std::string address, int minDepth, bool ignoreUnspendable);
extern UniValue z_sendmany(const UniValue& params, bool fHelp);
extern UniValue z_getnewaddress(const UniValue& params, bool fHelp);

std::string reservedPrivatizerPublicAddr;
std::string reservedPrivatizerPrivateAddr;

/**
 * Privatizer class
 */
class Privatizer
{
public:
    std::string publicAddress;
    std::string privateAddress;

public:
    bool Empty() const
    {
        return publicAddress.empty();
    }

    virtual UniValue GetUniValue() const
    {
        UniValue ret(UniValue::VOBJ);
        ret.pushKV("publicAddress", publicAddress);
        ret.pushKV("privateAddress", privateAddress);
        return ret;
    }
};

/**
 * A work item of Privatizer
 */
class PrivatizerWork
{
public:
    enum Workflow
    {
        T_TO_Z,
        Z_TO_Z,
        Z_TO_T
    }workflow;

    int startHeight; // This work will be proceeded after height
    std::string opid;
    std::string srcAddress;
    std::string destAddress;
    CAmount amount;

public:
    bool IsReady(int height) const
    {
        if (height < startHeight)
            return false;
        return opid.empty();
    }

    virtual UniValue GetUniValue() const
    {
        UniValue ret(UniValue::VOBJ);
        ret.pushKV("startHeight", startHeight);
        ret.pushKV("srcAddress", srcAddress);
        ret.pushKV("destAddress", destAddress);
        ret.pushKV("amount", FormatMoney(amount));
        ret.pushKV("opid", opid);
        return ret;
    }

    bool Start()
    {
        opid.clear();
        UniValue params(UniValue::VARR);
        params.push_back(srcAddress);
        UniValue dests(UniValue::VARR);
        UniValue dest(UniValue::VOBJ);
        CAmount realAmount = 0;
        LOCK2(cs_main, pwalletMain->cs_wallet);
        if (workflow == PrivatizerWork::T_TO_Z)
            realAmount = getBalanceTaddr(srcAddress, 1, false);
        else
            realAmount = getBalanceZaddr(srcAddress, 1, false);
        if (realAmount < amount) {
            LogPrintf("Warning PrivatizerWork %s of %s -> %s, but %s\n", FormatMoney(amount), srcAddress, destAddress, FormatMoney(realAmount));
            return false;
        }
        dest.pushKV("address", destAddress);
        dest.pushKV("amount", (amount - PRIVATIZER_TX_FEE) / (double)COIN);
        dests.push_back(dest);
        CAmount restAmout = realAmount - amount;
        if (restAmout > 0) {
            UniValue rest(UniValue::VOBJ);
            rest.pushKV("address", srcAddress);
            rest.pushKV("amount", restAmout / (double)COIN);
            dests.push_back(rest);
        }
        params.push_back(dests);
        UniValue ret = z_sendmany(params, false);
        opid = ret.get_str();
        LogPrintf("Start PrivatizerWork %s of %s -> %s opid %s\n", FormatMoney(amount), srcAddress, destAddress, opid);
        return !opid.empty() ? 1 : -1;
    }
};

/**
 * Extension information of Privatizer
 */
class PrivatizerInfo : public Privatizer
{
public:
    std::vector<PrivatizerWork> workflows;

public:
    static int GetStartHeight(int height, int min, int max)
    {
        return height + GetRandInt(max - min) + min;
    }

    virtual UniValue GetUniValue() const
    {
        UniValue ret = Privatizer::GetUniValue();
        UniValue works(UniValue::VARR);
        for (auto & work : workflows)
            works.push_back(work.GetUniValue());
        ret.pushKV("workflows", works);
        return ret;
    }

    bool IsPendingDeposit()
    {
        for (int i = 0; i < workflows.size(); i++) {
            PrivatizerWork work = workflows[i];
            if (work.workflow == PrivatizerWork::T_TO_Z)
                return true;
        }
        return false;
    }

    void ReserveWork(const PrivatizerWork &work)
    {
        LogPrintf("Reserve PrivatizerWork %s of %s -> %s at %d\n", FormatMoney(work.amount), work.srcAddress, work.destAddress, work.startHeight);
        workflows.push_back(work);
    }

    bool CheckNewDeposit(int height)
    {
        try {
            if (IsPendingDeposit())
                return false;
            CAmount amount = 0;
            {
                LOCK2(cs_main, pwalletMain->cs_wallet);
                amount = getBalanceTaddr(publicAddress, 1, false);
            }
            if (amount < PRIVATIZER_MIN_WORK_AMOUNT)
                return false;
            PrivatizerWork work;
            work.startHeight = GetStartHeight(height, PRIVATIZER_MIN_DELAY_T_TO_Z, PRIVATIZER_MAX_DELAY_T_TO_Z);
            work.workflow = PrivatizerWork::T_TO_Z;
            work.srcAddress = publicAddress;
            work.destAddress = z_getnewaddress(UniValue(UniValue::VARR), false).get_str();
            work.amount = GetRand(amount - PRIVATIZER_MIN_WORK_AMOUNT + 1) + PRIVATIZER_MIN_WORK_AMOUNT;
            ReserveWork(work);
            return true;
        } catch(const std::exception& e) {
            throw runtime_error(strprintf("%s: Error: (%s -> %s) %s", __func__, publicAddress, privateAddress, e.what()));
        }
        return false;
    }

    bool CheckPendingWork(std::string &newOpid, int height)
    {
        try {
            for (int i = 0; i < workflows.size(); i++) {
                PrivatizerWork work = workflows[i];
                if (work.IsReady(height) && work.Start()) {
                    workflows[i] = work;
                    newOpid = work.opid;
                    return true;
                }
            }
        } catch (const UniValue& objError) {
            throw runtime_error(strprintf("%s: Error: (%s -> %s) %s", __func__, publicAddress, privateAddress, objError.write()));
        } catch (const std::exception& e) {
            throw runtime_error(strprintf("%s: Error: (%s -> %s) %s", __func__, publicAddress, privateAddress, e.what()));
        }
        return false;
    }

    bool OnCompletedWork(const std::string &opid, int height)
    {
        try {
            for (auto p = workflows.begin(); p != workflows.end(); p++) {
                auto work = *p;
                if (work.opid == opid) {
                    workflows.erase(p);
                    if (work.workflow != PrivatizerWork::Z_TO_T) { // if not last work
                        PrivatizerWork nextWork;
                        PrivatizerWork::Workflow workflow;
                        int minDelay = 0, maxDelay = 0;
                        if (work.workflow == PrivatizerWork::T_TO_Z) {
                            workflow = PrivatizerWork::Z_TO_Z;
                            minDelay = PRIVATIZER_MIN_DELAY_Z_TO_Z;
                            maxDelay = PRIVATIZER_MAX_DELAY_Z_TO_Z;
                        } else {
                            workflow = PrivatizerWork::Z_TO_T;
                            minDelay = PRIVATIZER_MIN_DELAY_Z_TO_T;
                            maxDelay = PRIVATIZER_MAX_DELAY_Z_TO_T;
                        }
                        nextWork.workflow = workflow;
                        nextWork.startHeight = GetStartHeight(height, minDelay, maxDelay);
                        nextWork.srcAddress = work.destAddress;
                        nextWork.destAddress = nextWork.workflow == PrivatizerWork::Z_TO_T ? 
                                            privateAddress :
                                            z_getnewaddress(UniValue(), false).get_str();
                        nextWork.amount = work.amount - PRIVATIZER_TX_FEE;
                        ReserveWork(nextWork);
                    }
                    return true;
                }
            }
            throw runtime_error("Could not find");
        } catch(const std::exception& e) {
            throw runtime_error(strprintf("%s: Error: (%s -> %s) opid %s %s", __func__, publicAddress, privateAddress, opid, e.what()));
        }
        return false;
    }
};

/** 
 * Privatizer manager
 */
class PrivatizerMan
{
private:
    //! critical section to protect the inner data structures
    mutable CCriticalSection cs;

    //! Privatizer item table, PublicAddress to Privatizer
    std::map<std::string, PrivatizerInfo> mapPublicaddrPrivatizer;

    //! Map PrivateAddress to PrivatizerId
    std::map<std::string, std::string> mapPrivateaddrPrivatizerId;

    //! Map Opid to PrivatizerId/PublicAddress
    std::map<std::string, std::string> mapOpidPrivatizerId;

public:
    void Clear()
    {
        LOCK(cs);
        mapPublicaddrPrivatizer.clear();
        mapPrivateaddrPrivatizerId.clear();
        mapOpidPrivatizerId.clear();
    }

    PrivatizerMan()
    {
        Clear();
    }

    ~PrivatizerMan()
    {
        Clear();
    }

    template<class T>
    T GetPrivatizer(const std::string &publicAddress, const std::string &privateAddress) const
    {
        LOCK(cs);
        auto item = mapPublicaddrPrivatizer.find(publicAddress);
        if (item != mapPublicaddrPrivatizer.end() &&
            (privateAddress.empty() || privateAddress == item->second.privateAddress))
            return item->second;
        return T();
    }

    template<class T>
    void GetPrivatizers(std::vector<T> &v) const
    {
        LOCK(cs);
        for (auto & item: mapPublicaddrPrivatizer)
            v.push_back(item.second);
    }

    void AddPrivatizer(const std::string &publicAddress, const std::string &privateAddress, bool overwrite)
    {
        if (mapPublicaddrPrivatizer.size() >= PRIVATIZER_MAX_COUNT)
            throw runtime_error("Could not add privatizer since it has already maximum");
        CTxDestination tPubAddr = DecodeDestination(publicAddress);
        if (!IsValidDestination(tPubAddr))
            throw runtime_error("publicAddress should be taddr");
        isminetype mine = pwalletMain ? IsMine(*pwalletMain, tPubAddr) : ISMINE_NO;
        if (!(mine & ISMINE_SPENDABLE))
            throw runtime_error("publicAddress should be mine");
        CTxDestination tPrivAddr = DecodeDestination(privateAddress);
        if (!IsValidDestination(tPrivAddr))
            throw runtime_error("privateAddress should be taddr");
        if (tPubAddr == tPrivAddr)
            throw runtime_error("publicAddress and privateAddress could not be same");
        if (mapPrivateaddrPrivatizerId.find(publicAddress) != mapPrivateaddrPrivatizerId.end())
            throw runtime_error("publicAddress does already exist as privateAddress");
        if (!GetPrivatizer<PrivatizerInfo>(privateAddress, std::string()).Empty())
            throw runtime_error("privateAddress does already exist as publicAddress");
        auto item = GetPrivatizer<PrivatizerInfo>(publicAddress, std::string());
        if (!item.Empty()) {
            if (!overwrite)
                throw runtime_error("Exists already");
            if (item.privateAddress == privateAddress)
                return;
        }
        item.publicAddress = publicAddress;
        item.privateAddress = privateAddress;
        LOCK(cs);
        mapPublicaddrPrivatizer[publicAddress] = item;
        mapPrivateaddrPrivatizerId[privateAddress] = item.publicAddress;
    }

    void RemovePrivatizer(const std::string &publicAddress, const std::string &privateAddress, bool matchAll)
    {
        auto item = GetPrivatizer<Privatizer>(publicAddress, matchAll ? privateAddress : std::string());
        if (item.Empty())
            throw runtime_error("Not exist");
        LOCK(cs);
        mapPublicaddrPrivatizer.erase(item.publicAddress);
        mapPrivateaddrPrivatizerId.erase(item.privateAddress);
    }

    void OnFinishedZsendmany(const std::string &opid, bool success)
    {
        try {
            LOCK(cs);
            auto id = mapOpidPrivatizerId.find(opid);
            if (id == mapOpidPrivatizerId.end())
                return;
            PrivatizerInfo item = GetPrivatizer<PrivatizerInfo>(id->second, std::string());
            if (item.Empty())
                return;
            
            if (!success)
                throw runtime_error(strprintf("%s: Error: (%s -> %s) failed opid %s", __func__, item.publicAddress, item.privateAddress, opid));
            
            item.OnCompletedWork(opid, GetBlockHeight());
            mapPublicaddrPrivatizer[item.publicAddress] = item;
        } catch(const std::exception& e) {
            LogPrintf("%s\n", e.what());
        }
    }

    void Run()
    {
        const int eachPrivatizerInterval = 10;    // milliseconds
        const int nextLoopInterval = 10000;

        while (true) {
            std::vector<Privatizer> vPrivertizers;
            GetPrivatizers<Privatizer>(vPrivertizers);
            for (auto & item: vPrivertizers) {
                try {
                    LOCK(cs);
                    auto info = GetPrivatizer<PrivatizerInfo>(item.publicAddress, item.privateAddress);
                    if (!info.Empty())
                    {
                        int height = GetBlockHeight();
                        std::string newOpid;
                        if (info.CheckPendingWork(newOpid, height)) {
                            mapPublicaddrPrivatizer[info.publicAddress] = info;
                            mapOpidPrivatizerId[newOpid] = info.publicAddress;
                        }
                        if (info.CheckNewDeposit(height))
                            mapPublicaddrPrivatizer[info.publicAddress] = info;
                    }
                } catch (const std::exception& e) {
                    LogPrintf("%s\n", e.what());
                } catch (...) {
                    LogPrintf("%s Unknown error\n", __func__);
                }
                boost::this_thread::interruption_point();
                MilliSleep(eachPrivatizerInterval);
            }
            boost::this_thread::interruption_point();
            MilliSleep(nextLoopInterval);
        }
    }

private:
    static int GetBlockHeight()
    {
        LOCK2(cs_main, pwalletMain ? &pwalletMain->cs_wallet : NULL);
        return chainActive.Height();
    }
};

static PrivatizerMan privatizerMan;

void ThreadPrivatizer()
{
    privatizerMan.Run();
}

void OnFinishedZsendmany(const std::string &opid, bool success)
{
    privatizerMan.OnFinishedZsendmany(opid, success);
}

UniValue GetPrivatizerAsUniValue(const std::string &publicAddress, const std::string &privateAddress)
{
    auto item = privatizerMan.GetPrivatizer<Privatizer>(publicAddress, privateAddress);
    return item.GetUniValue();
}

UniValue GetPrivatizerInfoAsUniValue(const std::string &publicAddress, const std::string &privateAddress)
{
    auto item = privatizerMan.GetPrivatizer<PrivatizerInfo>(publicAddress, privateAddress);
    return item.GetUniValue();
}

UniValue GetPrivatizersAsUniValue()
{
    UniValue ret(UniValue::VARR);
    std::vector<Privatizer> v;
    privatizerMan.GetPrivatizers<Privatizer>(v);
    for (auto & item: v)
        ret.push_back(item.GetUniValue());
    return ret;
}

UniValue GetPrivatizerInfosAsUniValue()
{
    UniValue ret(UniValue::VARR);
    std::vector<PrivatizerInfo> v;
    privatizerMan.GetPrivatizers<PrivatizerInfo>(v);
    for (auto & item: v)
        ret.push_back(item.GetUniValue());
    return ret;
}

void AddPrivatizer(const std::string &publicAddress, const std::string &privateAddress, bool overwrite)
{
    privatizerMan.AddPrivatizer(publicAddress, privateAddress, overwrite);
}

void RemovePrivatizer(const std::string &publicAddress, const std::string &privateAddress, bool matchAll)
{
    privatizerMan.RemovePrivatizer(publicAddress, privateAddress, matchAll);
}

static void PairReservedPrivatizer(bool overwrite)
{
    privatizerMan.AddPrivatizer(reservedPrivatizerPublicAddr, reservedPrivatizerPrivateAddr, overwrite);
}

void ReservePrivatizerPublicAddress(const std::string &publicAddress, bool overwrite)
{
    reservedPrivatizerPublicAddr = publicAddress;
    if (!reservedPrivatizerPublicAddr.empty() && !reservedPrivatizerPrivateAddr.empty())
    {
        PairReservedPrivatizer(overwrite);
        reservedPrivatizerPublicAddr.clear();
        reservedPrivatizerPrivateAddr.clear();
    }
}

void ReservePrivatizerPrivateAddress(const std::string &privateAddress, bool overwrite)
{
    reservedPrivatizerPrivateAddr = privateAddress;
    if (!reservedPrivatizerPublicAddr.empty() && !reservedPrivatizerPrivateAddr.empty())
    {
        PairReservedPrivatizer(overwrite);
        reservedPrivatizerPublicAddr.clear();
        reservedPrivatizerPrivateAddr.clear();
    }
}
