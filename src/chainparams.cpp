// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Copyright (c) 2016-2018 The Zcash developers
// Copyright (c) 2018 The Resistance developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "key_io.h"
#include "main.h"

#include "util.h"
#include "utilstrencodings.h"

#include <assert.h>

#include <boost/assign/list_of.hpp>

#include "chainparamsseeds.h"
#include "assetchain.h"

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, const uint256& nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    // To create a genesis block for a new chain which is Overwintered:
    //   txNew.nVersion = OVERWINTER_TX_VERSION
    //   txNew.fOverwintered = true
    //   txNew.nVersionGroupId = OVERWINTER_VERSION_GROUP_ID
    //   txNew.nExpiryHeight = <default value>
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 520617983 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(txNew);
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = genesis.BuildMerkleTree();
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database (and is in any case of zero value).
 *
 * >>> from pyblake2 import blake2s
 * >>> 'Zcash' + blake2s(b'The Economist 2016-10-29 Known unknown: Another crypto-currency is born. BTC#436254 0000000000000000044f321997f336d2908cf8c8d6893e88dbf067e2d949487d ETH#2521903 483039a6b6bd8bd05f0584f9a078d075e454925eb71c1f13eaff59b405a721bb DJIA close on 27 Oct 2016: 18,169.68').hexdigest()
 *
 * CBlock(hash=00040fe8, ver=4, hashPrevBlock=00000000000000, hashMerkleRoot=c4eaa5, nTime=1477641360, nBits=1f07ffff, nNonce=4695, vtx=1)
 *   CTransaction(hash=c4eaa5, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff071f0104455a6361736830623963346565663862376363343137656535303031653335303039383462366665613335363833613763616331343161303433633432303634383335643334)
 *     CTxOut(nValue=0.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
 *   vMerkleTree: c4eaa5
 */
static CBlock CreateGenesisBlock(uint32_t nTime, const uint256& nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "NB2HI4B2F4XWE2LUFZWHSL3SGNZWS43UMFXGGMZQGE======";
    const CScript genesisOutputScript = CScript() << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */

const arith_uint256 maxUint = UintToArith256(uint256S("ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"));

class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";
        strCurrencyUnits = "RES";
        bip44CoinType = 356; // As registered in https://github.com/satoshilabs/slips/blob/master/slip-0044.md
        consensus.fCoinbaseMustBeProtected = false;
        consensus.nSubsidySlowStartHeight = 201;
        consensus.nSubsidySlowStartInterval = 43200;
        consensus.nSubsidyHalvingInterval = 2200000 - 14400;
        consensus.nPorRewardPercentage = 30;
        consensus.nMasternodeRewardPercentage = 30;
        consensus.nPlatformDevFundPercentage = 10;
        consensus.nPorRewardTxPercentage = 30;
        consensus.nMasternodeRewardTxPercentage = 30;
        consensus.nPlatformDevFundTxPercentage = 10;
        consensus.nCoinbaseMaturity = 100;
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 4000;
        consensus.powLimit = uint256S("0007ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowAveragingWindow = 17;
        assert(maxUint/UintToArith256(consensus.powLimit) >= consensus.nPowAveragingWindow);
        consensus.nPowMaxAdjustDown = 32; // 32% adjustment down
        consensus.nPowMaxAdjustUp = 16; // 16% adjustment up
        consensus.nPowTargetSpacing = 60;
        consensus.nPowAllowMinDifficultyBlocksAfterHeight = boost::none;
        consensus.vUpgrades[Consensus::BASE_SPROUT].nProtocolVersion = 170002;
        consensus.vUpgrades[Consensus::BASE_SPROUT].nActivationHeight =
            Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nProtocolVersion = 170002;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].nProtocolVersion = 170005;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].nActivationHeight =
            Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].nProtocolVersion = 170007;
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].nActivationHeight = 1;
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nProtocolVersion = 170009;
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        /**
         * The message start string should be awesome!
         */
        pchMessageStart[0] = 0x52;
        pchMessageStart[1] = 0x45;
        pchMessageStart[2] = 0x53;
        pchMessageStart[3] = 0x21;
        vAlertPubKey = ParseHex("0487bb118d004101d6ad12ad2a173417aac08fe99522f9e9f63fc76e79ee63ba964e6b3d5d9340e80d0eb0d4f346b07a1923450fcb3c2591f9e340d321e7c53c35");
        nDefaultPort = 8133;
        nPruneAfterHeight = 100000;

#if 0
        uint256 nonce = uint256(), hash;
        do {
            nonce = ArithToUint256(UintToArith256(nonce) + 1);
            genesis = CreateGenesisBlock(
                1564407265,
                nonce,
                0x1f07ffff, 4, 0);
            hash = genesis.GetPoWHash();
        } while (UintToArith256(hash) > UintToArith256(consensus.powLimit));
        printf("nonce = %s\n", nonce.ToString().c_str());
        printf("genesis.GetPoWHash = %s\n", hash.ToString().c_str());
        printf("genesis.GetHash = %s\n", genesis.GetHash().ToString().c_str());
        printf("genesis.hashMerkleRoot = %s\n", genesis.hashMerkleRoot.ToString().c_str());
#else
        genesis = CreateGenesisBlock(
            1564407265,
            uint256S("0x0000000000000000000000000000000000000000000000000000000000006332"),
            0x1f07ffff, 4, 0);
#endif
        assert(genesis.GetPoWHash() == uint256S("0x000242261b14bd5271d49f7549bc258ef4dc01b3b9d559efd641918c2fcbab76"));
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x328ab5de52cf3b7c1a5dbd02909b7e23a25d1715143c8b41aba903cf2b48754f"));
        assert(genesis.hashMerkleRoot == uint256S("0x7796cb16e0eda28f0e182764630ba87c88f147e5243178ff7ca68d266a949a9a"));

        vFixedSeeds.clear();
        vSeeds.clear();
        vSeeds.push_back(CDNSSeedData("mainnet.resistance.io", "mainnet.resistance.io"));
        vSeeds.push_back(CDNSSeedData("mainnet_seed1.resistance.io", "mainnet_seed1.resistance.io"));

        // guarantees the first 2 characters, when base58 encoded, are "r1"
        base58Prefixes[PUBKEY_ADDRESS]     = {0x1B,0x97};
        // guarantees the first 2 characters, when base58 encoded, are "r3"
        base58Prefixes[SCRIPT_ADDRESS]     = {0x1B,0x9C};
        // the first character, when base58 encoded, is "5" or "K" or "L" (as in Bitcoin)
        base58Prefixes[SECRET_KEY]         = {0x80};
        // do not rely on these BIP32 prefixes; they are not specified and may change
        base58Prefixes[EXT_PUBLIC_KEY]     = {0x04,0x88,0xB2,0x1E};
        base58Prefixes[EXT_SECRET_KEY]     = {0x04,0x88,0xAD,0xE4};
        // guarantees the first 2 characters, when base58 encoded, are "zr"
        base58Prefixes[ZCPAYMENT_ADDRRESS] = {0x16,0xB2};
        // guarantees the first 4 characters, when base58 encoded, are "ZiVK"
        base58Prefixes[ZCVIEWING_KEY]      = {0xA8,0xAB,0xD3};
        // guarantees the first 2 characters, when base58 encoded, are "SK"
        base58Prefixes[ZCSPENDING_KEY]     = {0xAB,0x36};

        bech32HRPs[SAPLING_PAYMENT_ADDRESS]      = "zs";
        bech32HRPs[SAPLING_FULL_VIEWING_KEY]     = "zviews";
        bech32HRPs[SAPLING_INCOMING_VIEWING_KEY] = "zivks";
        bech32HRPs[SAPLING_EXTENDED_SPEND_KEY]   = "secret-extended-key-main";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = false;

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            (0, consensus.hashGenesisBlock),
            genesis.nTime,  // * UNIX timestamp of last checkpoint block
            0,              // * total number of transactions between genesis and last checkpoint
                            //   (the tx=... number in the SetBestChain debug.log lines)
            0               // * estimated number of transactions per day after checkpoint
                            //   total number of tx / (checkpoint block height / (24 * 60))
        };

        // PoR reward script expects a vector of P2SH addresses
        vPorRewardAddress = { "r3BRC7xQ1JemFuRgCYPhpsQQhUmgbz772Dh" };
        assert(vPorRewardAddress.size() <= consensus.GetLastPorRewardBlockHeight());
        // Masternode reward script expects a vector of P2SH addresses
        vMasternodeRewardAddress = { "r394kKYEHd67UnsS7v1zuhRLPjANqD5T2G2" };
        assert(vMasternodeRewardAddress.size() <= consensus.GetLastMasternodeRewardBlockHeight());
        // PlatformDev fund script expects a vector of P2SH addresses
        vPlatformDevFundAddress = { "r39mwRFQzHz55ZrEL3dGuDVCXHtmCRJ2jzo" };
        assert(vPlatformDevFundAddress.size() <= consensus.GetLastPlatformDevFundBlockHeight());
    }
};
static CMainParams mainParams;

/**
 * Testnet (v3)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        strCurrencyUnits = "SER";
        bip44CoinType = 1;
        consensus.fCoinbaseMustBeProtected = false;
        consensus.nSubsidySlowStartHeight = 201;
        consensus.nSubsidySlowStartInterval = 43200;
        consensus.nSubsidyHalvingInterval = 2200000 - 14400;
        consensus.nPorRewardPercentage = 30;
        consensus.nMasternodeRewardPercentage = 30;
        consensus.nPlatformDevFundPercentage = 10;
        consensus.nPorRewardTxPercentage = 30;
        consensus.nMasternodeRewardTxPercentage = 30;
        consensus.nPlatformDevFundTxPercentage = 10;
        consensus.nCoinbaseMaturity = 2;
        consensus.nMajorityEnforceBlockUpgrade = 51;
        consensus.nMajorityRejectBlockOutdated = 75;
        consensus.nMajorityWindow = 400;
        consensus.powLimit = uint256S("0007ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowAveragingWindow = 17;
        assert(maxUint/UintToArith256(consensus.powLimit) >= consensus.nPowAveragingWindow);
        consensus.nPowMaxAdjustDown = 32; // 32% adjustment down
        consensus.nPowMaxAdjustUp = 16; // 16% adjustment up
        consensus.nPowTargetSpacing = 60;
        consensus.nPowAllowMinDifficultyBlocksAfterHeight = 0;
        consensus.vUpgrades[Consensus::BASE_SPROUT].nProtocolVersion = 170002;
        consensus.vUpgrades[Consensus::BASE_SPROUT].nActivationHeight =
            Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nProtocolVersion = 170002;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].nProtocolVersion = 170003;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].nActivationHeight =
            Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].nProtocolVersion = 170007;
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].nActivationHeight = 1;
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nProtocolVersion = 170008;
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        pchMessageStart[0] = 0x53;
        pchMessageStart[1] = 0x45;
        pchMessageStart[2] = 0x52;
        pchMessageStart[3] = 0x21;
        vAlertPubKey = ParseHex("04872bb1ad54b0ca5b5874b6e1888062bc9adb58f63a76cbba8243c12231f92b7adf7b1bb0d8c4541d9cb338607d6345caa837971f9cf32d313163f94ada0090f1");
        nDefaultPort = 18133;
        nPruneAfterHeight = 1000;

#if 0
        uint256 nonce = uint256(), hash;
        do {
            nonce = ArithToUint256(UintToArith256(nonce) + 1);
            genesis = CreateGenesisBlock(
                1564162983,
                nonce,
                0x1f07ffff, 4, 0);
            hash = genesis.GetPoWHash();
        } while (UintToArith256(hash) > UintToArith256(consensus.powLimit));
        printf("nonce = %s\n", nonce.ToString().c_str());
        printf("genesis.GetPoWHash = %s\n", hash.ToString().c_str());
        printf("genesis.GetHash = %s\n", genesis.GetHash().ToString().c_str());
        printf("genesis.hashMerkleRoot = %s\n", genesis.hashMerkleRoot.ToString().c_str());
#else
        genesis = CreateGenesisBlock(
            1564162983,
            uint256S("0x0000000000000000000000000000000000000000000000000000000000008e3e"),
            0x1f07ffff, 4, 0);
#endif
        assert(genesis.GetPoWHash() == uint256S("0x00040a0846c645729d46934c4307b91ebf0650230571952fc64a46d7fa3143d4"));
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x4b9b38252d0121cc14197f2764e85da2b5bd0892dd157f047b624212f97568b8"));
        assert(genesis.hashMerkleRoot == uint256S("0x7796cb16e0eda28f0e182764630ba87c88f147e5243178ff7ca68d266a949a9a"));

        vFixedSeeds.clear();
        vSeeds.clear();
        vSeeds.push_back(CDNSSeedData("testnet_v2.resistance.io", "testnet_v2.resistance.io"));

        // guarantees the first 2 characters, when base58 encoded, are "rp"
        base58Prefixes[PUBKEY_ADDRESS]     = {0x1C,0x0C};
        // guarantees the first 2 characters, when base58 encoded, are "rs"
        base58Prefixes[SCRIPT_ADDRESS]     = {0x1C,0x13};
        // the first character, when base58 encoded, is "9" or "c" (as in Bitcoin)
        base58Prefixes[SECRET_KEY]         = {0xEF};
        // do not rely on these BIP32 prefixes; they are not specified and may change
        base58Prefixes[EXT_PUBLIC_KEY]     = {0x04,0x35,0x87,0xCF};
        base58Prefixes[EXT_SECRET_KEY]     = {0x04,0x35,0x83,0x94};
        // guarantees the first 2 characters, when base58 encoded, are "zt"
        base58Prefixes[ZCPAYMENT_ADDRRESS] = {0x16,0xB6};
        // guarantees the first 4 characters, when base58 encoded, are "ZiVt"
        base58Prefixes[ZCVIEWING_KEY]      = {0xA8,0xAC,0x0C};
        // guarantees the first 2 characters, when base58 encoded, are "ST"
        base58Prefixes[ZCSPENDING_KEY]     = {0xAC,0x08};

        bech32HRPs[SAPLING_PAYMENT_ADDRESS]      = "ztestsapling";
        bech32HRPs[SAPLING_FULL_VIEWING_KEY]     = "zviewtestsapling";
        bech32HRPs[SAPLING_INCOMING_VIEWING_KEY] = "zivktestsapling";
        bech32HRPs[SAPLING_EXTENDED_SPEND_KEY]   = "secret-extended-key-test";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = true;

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            (0, consensus.hashGenesisBlock),
            genesis.nTime,  // * UNIX timestamp of last checkpoint block
            0,              // * total number of transactions between genesis and last checkpoint
                            //   (the tx=... number in the SetBestChain debug.log lines)
            0               // * estimated number of transactions per day after checkpoint
                            //   total number of tx / (checkpoint block height / (24 * 60))
        };

        // PoR reward script expects a vector of P2SH addresses
        vPorRewardAddress = { "rs8MA1zvQTjx5VHKmoDwqMWNbnG35GvcmEs" };
        assert(vPorRewardAddress.size() <= consensus.GetLastPorRewardBlockHeight());
        // Masternode reward script expects a vector of P2SH addresses
        vMasternodeRewardAddress = { "rs7afWjGdWG3RAzWeGVWKwAugFujgHbMDEW" };
        assert(vMasternodeRewardAddress.size() <= consensus.GetLastMasternodeRewardBlockHeight());
        // PlatformDev fund script expects a vector of P2SH addresses
        vPlatformDevFundAddress = { "rs1ipbEkuysjmybyPAr6z85brggV2sBrdEZ" };
        assert(vPlatformDevFundAddress.size() <= consensus.GetLastPlatformDevFundBlockHeight());
    }
};
static CTestNetParams testNetParams;

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";
        strCurrencyUnits = "REG";
        bip44CoinType = 1;
        consensus.fCoinbaseMustBeProtected = false;
        consensus.nSubsidySlowStartHeight = 201;
        consensus.nSubsidySlowStartInterval = 43200;
        consensus.nSubsidyHalvingInterval = 2200000 - 14400;
        consensus.nPorRewardPercentage = 30;
        consensus.nMasternodeRewardPercentage = 30;
        consensus.nPlatformDevFundPercentage = 10;
        consensus.nPorRewardTxPercentage = 30;
        consensus.nMasternodeRewardTxPercentage = 30;
        consensus.nPlatformDevFundTxPercentage = 10;
        consensus.nCoinbaseMaturity = 2;
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 1000;
        consensus.powLimit = uint256S("0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f");
        consensus.nPowAveragingWindow = 17;
        assert(maxUint/UintToArith256(consensus.powLimit) >= consensus.nPowAveragingWindow);
        consensus.nPowMaxAdjustDown = 0; // Turn off adjustment down
        consensus.nPowMaxAdjustUp = 0; // Turn off adjustment up
        consensus.nPowTargetSpacing = 60;
        consensus.nPowAllowMinDifficultyBlocksAfterHeight = 0;
        consensus.vUpgrades[Consensus::BASE_SPROUT].nProtocolVersion = 170002;
        consensus.vUpgrades[Consensus::BASE_SPROUT].nActivationHeight =
            Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nProtocolVersion = 170002;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].nProtocolVersion = 170003;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].nProtocolVersion = 170006;
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nProtocolVersion = 170008;
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        pchMessageStart[0] = 0x52;
        pchMessageStart[1] = 0x45;
        pchMessageStart[2] = 0x47;
        pchMessageStart[3] = 0x21;
        nDefaultPort = 28133;
        nPruneAfterHeight = 1000;

#if 0
        uint256 nonce = uint256(), hash;
        do {
            nonce = ArithToUint256(UintToArith256(nonce) + 1);
            genesis = CreateGenesisBlock(
                1542317813,
                nonce,
                0x200f0f0f, 4, 0);
            hash = genesis.GetPoWHash();
        } while (UintToArith256(hash) > UintToArith256(consensus.powLimit));
        printf("nonce = %s\n", nonce.ToString().c_str());
        printf("genesis.GetPoWHash = %s\n", hash.ToString().c_str());
        printf("genesis.GetHash = %s\n", genesis.GetHash().ToString().c_str());
        printf("genesis.hashMerkleRoot = %s\n", genesis.hashMerkleRoot.ToString().c_str());
#else
        genesis = CreateGenesisBlock(
            1542317813,
            uint256S("0x000000000000000000000000000000000000000000000000000000000000002c"),
            0x200f0f0f, 4, 0);
#endif
        assert(genesis.GetPoWHash() == uint256S("0x0095e3fbe59a2a612ededd70b1a13edbe39a177f26258a5827a1982eb2d96e60"));
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x1894e002dce091337415711e153dfdcd7d75d68ffd5491ddff8986758d4ea0e2"));
        assert(genesis.hashMerkleRoot == uint256S("0x7796cb16e0eda28f0e182764630ba87c88f147e5243178ff7ca68d266a949a9a"));

        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;
        fTestnetToBeDeprecatedFieldRPC = false;

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            (0, consensus.hashGenesisBlock),
            genesis.nTime,  // * UNIX timestamp of last checkpoint block
            0,              // * total number of transactions between genesis and last checkpoint
                            //   (the tx=... number in the SetBestChain debug.log lines)
            0               // * estimated number of transactions per day after checkpoint
                            //   total number of tx / (checkpoint block height / (24 * 60))
        };

        // These prefixes are the same as the testnet prefixes
        base58Prefixes[PUBKEY_ADDRESS]     = {0x1C,0x0C};
        base58Prefixes[SCRIPT_ADDRESS]     = {0x1C,0x13};
        base58Prefixes[SECRET_KEY]         = {0xEF};
        // do not rely on these BIP32 prefixes; they are not specified and may change
        base58Prefixes[EXT_PUBLIC_KEY]     = {0x04,0x35,0x87,0xCF};
        base58Prefixes[EXT_SECRET_KEY]     = {0x04,0x35,0x83,0x94};
        base58Prefixes[ZCPAYMENT_ADDRRESS] = {0x16,0xB6};
        base58Prefixes[ZCVIEWING_KEY]      = {0xA8,0xAC,0x0C};
        base58Prefixes[ZCSPENDING_KEY]     = {0xAC,0x08};

        bech32HRPs[SAPLING_PAYMENT_ADDRESS]      = "zregtestsapling";
        bech32HRPs[SAPLING_FULL_VIEWING_KEY]     = "zviewregtestsapling";
        bech32HRPs[SAPLING_INCOMING_VIEWING_KEY] = "zivkregtestsapling";
        bech32HRPs[SAPLING_EXTENDED_SPEND_KEY]   = "secret-extended-key-regtest";

        // PoR reward script expects a vector of P2SH addresses
        vPorRewardAddress = { "rs8MA1zvQTjx5VHKmoDwqMWNbnG35GvcmEs" };
        assert(vPorRewardAddress.size() <= consensus.GetLastPorRewardBlockHeight());
        // Masternode reward script expects a vector of P2SH addresses
        vMasternodeRewardAddress = { "rs7afWjGdWG3RAzWeGVWKwAugFujgHbMDEW" };
        assert(vMasternodeRewardAddress.size() <= consensus.GetLastMasternodeRewardBlockHeight());
        // PlatformDev fund script expects a vector of P2SH addresses
        vPlatformDevFundAddress = { "rs1ipbEkuysjmybyPAr6z85brggV2sBrdEZ" };
        assert(vPlatformDevFundAddress.size() <= consensus.GetLastPlatformDevFundBlockHeight());
    }

    void UpdateNetworkUpgradeParameters(Consensus::UpgradeIndex idx, int nActivationHeight)
    {
        assert(idx > Consensus::BASE_SPROUT && idx < Consensus::MAX_NETWORK_UPGRADES);
        consensus.vUpgrades[idx].nActivationHeight = nActivationHeight;
    }

    void SetRegTestZIP209Enabled() {
        fZIP209Enabled = true;
    }
};
static CRegTestParams regTestParams;

static CChainParams *pCurrentParams = 0;

CChainParams &SelectedParams() {
    assert(pCurrentParams);
    return *pCurrentParams;
}

const CChainParams &Params() {
    return SelectedParams();
}

CChainParams &Params(CBaseChainParams::Network network) {
    switch (network) {
        case CBaseChainParams::MAIN:
            return mainParams;
        case CBaseChainParams::TESTNET:
            return testNetParams;
        case CBaseChainParams::REGTEST:
            return regTestParams;
        default:
            assert(false && "Unimplemented network");
            return mainParams;
    }
}

void SelectParams(CBaseChainParams::Network network) {
    SelectBaseParams(network);
    pCurrentParams = &Params(network);

    auto assetchain = GetArg("-assetchain", "");
    if (!assetchain.empty())
        SetAssetchainParams(assetchain, SelectedBaseParams(), *pCurrentParams);

    // Some python qa rpc tests need to enforce the coinbase consensus rule
    if (network == CBaseChainParams::REGTEST && mapArgs.count("-regtestprotectcoinbase")) {
        regTestParams.SetRegTestCoinbaseMustBeProtected();
    }

    // When a developer is debugging turnstile violations in regtest mode, enable ZIP209
    if (network == CBaseChainParams::REGTEST && mapArgs.count("-developersetpoolsizezero")) {
        regTestParams.SetRegTestZIP209Enabled();
    }
}

bool SelectParamsFromCommandLine()
{
    CBaseChainParams::Network network = NetworkIdFromCommandLine();
    if (network == CBaseChainParams::MAX_NETWORK_TYPES)
        return false;

    SelectParams(network);
    return true;
}


// Block height must be >0 and <=last PoR reward block height
// Index variable i ranges from 0 - (vPorRewardAddress.size()-1)
std::string CChainParams::GetPorRewardAddressAtHeight(int nHeight) const {
    size_t maxHeight = consensus.GetLastPorRewardBlockHeight();
    assert(nHeight > 0 && nHeight <= maxHeight);

    size_t addressChangeInterval = (maxHeight + vPorRewardAddress.size()) / vPorRewardAddress.size();
    size_t i = nHeight / addressChangeInterval;
    return vPorRewardAddress[i];
}

// Block height must be >0 and <=last PoR reward block height
// The PoR reward address is expected to be a multisig (P2SH) address
CScript CChainParams::GetPorRewardScriptAtHeight(int nHeight) const {
    assert(nHeight > 0 && nHeight <= consensus.GetLastPorRewardBlockHeight());

    CTxDestination address = DecodeDestination(GetPorRewardAddressAtHeight(nHeight).c_str());
    assert(IsValidDestination(address));
    assert(boost::get<CScriptID>(&address) != nullptr);
    CScriptID scriptID = boost::get<CScriptID>(address); // address is a boost variant
    CScript script = CScript() << OP_HASH160 << ToByteVector(scriptID) << OP_EQUAL;
    return script;
}

std::string CChainParams::GetPorRewardAddressAtIndex(int i) const {
    assert(i >= 0 && i < vPorRewardAddress.size());
    return vPorRewardAddress[i];
}

// Block height must be >0 and <=last Masternode reward block height
// Index variable i ranges from 0 - (vMasternodeRewardAddress.size()-1)
std::string CChainParams::GetMasternodeRewardAddressAtHeight(int nHeight) const {
    size_t maxHeight = consensus.GetLastMasternodeRewardBlockHeight();
    assert(nHeight > 0 && nHeight <= maxHeight);

    size_t addressChangeInterval = (maxHeight + vMasternodeRewardAddress.size()) / vMasternodeRewardAddress.size();
    size_t i = nHeight / addressChangeInterval;
    return vMasternodeRewardAddress[i];
}

// Block height must be >0 and <=last Masternode reward block height
// The Masternode reward address is expected to be a multisig (P2SH) address
CScript CChainParams::GetMasternodeRewardScriptAtHeight(int nHeight) const {
    assert(nHeight > 0 && nHeight <= consensus.GetLastMasternodeRewardBlockHeight());

    CTxDestination address = DecodeDestination(GetMasternodeRewardAddressAtHeight(nHeight).c_str());
    assert(IsValidDestination(address));
    assert(boost::get<CScriptID>(&address) != nullptr);
    CScriptID scriptID = boost::get<CScriptID>(address); // address is a boost variant
    CScript script = CScript() << OP_HASH160 << ToByteVector(scriptID) << OP_EQUAL;
    return script;
}

std::string CChainParams::GetMasternodeRewardAddressAtIndex(int i) const {
    assert(i >= 0 && i < vMasternodeRewardAddress.size());
    return vMasternodeRewardAddress[i];
}

// Block height must be >0 and <=last PlatformDev fund block height
// Index variable i ranges from 0 - (vPlatformDevFundAddress.size()-1)
std::string CChainParams::GetPlatformDevFundAddressAtHeight(int nHeight) const {
    size_t maxHeight = consensus.GetLastPlatformDevFundBlockHeight();
    assert(nHeight > 0 && nHeight <= maxHeight);

    size_t addressChangeInterval = (maxHeight + vPlatformDevFundAddress.size()) / vPlatformDevFundAddress.size();
    size_t i = nHeight / addressChangeInterval;
    return vPlatformDevFundAddress[i];
}

// Block height must be >0 and <=last PlatformDev fund block height
// The PlatformDev fund address is expected to be a multisig (P2SH) address
CScript CChainParams::GetPlatformDevFundScriptAtHeight(int nHeight) const {
    assert(nHeight > 0 && nHeight <= consensus.GetLastPlatformDevFundBlockHeight());

    CTxDestination address = DecodeDestination(GetPlatformDevFundAddressAtHeight(nHeight).c_str());
    assert(IsValidDestination(address));
    assert(boost::get<CScriptID>(&address) != nullptr);
    CScriptID scriptID = boost::get<CScriptID>(address); // address is a boost variant
    CScript script = CScript() << OP_HASH160 << ToByteVector(scriptID) << OP_EQUAL;
    return script;
}

std::string CChainParams::GetPlatformDevFundAddressAtIndex(int i) const {
    assert(i >= 0 && i < vPlatformDevFundAddress.size());
    return vPlatformDevFundAddress[i];
}

void UpdateNetworkUpgradeParameters(Consensus::UpgradeIndex idx, int nActivationHeight)
{
    regTestParams.UpdateNetworkUpgradeParameters(idx, nActivationHeight);
}
