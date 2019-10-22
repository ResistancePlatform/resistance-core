// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://www.opensource.org/licenses/mit-license.php .

#ifndef BITCOIN_CONSENSUS_PARAMS_H
#define BITCOIN_CONSENSUS_PARAMS_H

#include "uint256.h"

#include <boost/optional.hpp>

namespace Consensus {

/**
 * Index into Params.vUpgrades and NetworkUpgradeInfo
 *
 * Being array indices, these MUST be numbered consecutively.
 *
 * The order of these indices MUST match the order of the upgrades on-chain, as
 * several functions depend on the enum being sorted.
 */
enum UpgradeIndex : uint32_t {
    // Sprout must be first
    BASE_SPROUT,
    UPGRADE_TESTDUMMY,
    UPGRADE_OVERWINTER,
    UPGRADE_SAPLING,
    UPGRADE_BLOSSOM,
    // NOTE: Also add new upgrades to NetworkUpgradeInfo in upgrades.cpp
    MAX_NETWORK_UPGRADES
};

struct NetworkUpgrade {
    /**
     * The first protocol version which will understand the new consensus rules
     */
    int nProtocolVersion;

    /**
     * Height of the first block for which the new consensus rules will be active
     */
    int nActivationHeight;

    /**
     * Special value for nActivationHeight indicating that the upgrade is always active.
     * This is useful for testing, as it means tests don't need to deal with the activation
     * process (namely, faking a chain of somewhat-arbitrary length).
     *
     * New blockchains that want to enable upgrade rules from the beginning can also use
     * this value. However, additional care must be taken to ensure the genesis block
     * satisfies the enabled rules.
     */
    static constexpr int ALWAYS_ACTIVE = 0;

    /**
     * Special value for nActivationHeight indicating that the upgrade will never activate.
     * This is useful when adding upgrade code that has a testnet activation height, but
     * should remain disabled on mainnet.
     */
    static constexpr int NO_ACTIVATION_HEIGHT = -1;

    /**
     * The hash of the block at height nActivationHeight, if known. This is set manually
     * after a network upgrade activates.
     *
     * We use this in IsInitialBlockDownload to detect whether we are potentially being
     * fed a fake alternate chain. We use NU activation blocks for this purpose instead of
     * the checkpoint blocks, because network upgrades (should) have significantly more
     * scrutiny than regular releases. nMinimumChainWork MUST be set to at least the chain
     * work of this block, otherwise this detection will have false positives.
     */
    boost::optional<uint256> hashActivationBlock;
};

/** ZIP208 block target interval in seconds. */
static const unsigned int PRE_BLOSSOM_POW_TARGET_SPACING = 60;
static const unsigned int POST_BLOSSOM_POW_TARGET_SPACING = 60;
#if 0
static_assert(POST_BLOSSOM_POW_TARGET_SPACING < PRE_BLOSSOM_POW_TARGET_SPACING, "Blossom target spacing must be less than pre-Blossom target spacing.");
static const unsigned int PRE_BLOSSOM_HALVING_INTERVAL = 840000;
static const unsigned int PRE_BLOSSOM_REGTEST_HALVING_INTERVAL = 150;
#endif
static const int BLOSSOM_POW_TARGET_SPACING_RATIO = PRE_BLOSSOM_POW_TARGET_SPACING / POST_BLOSSOM_POW_TARGET_SPACING;
static_assert(BLOSSOM_POW_TARGET_SPACING_RATIO * POST_BLOSSOM_POW_TARGET_SPACING == PRE_BLOSSOM_POW_TARGET_SPACING, "Invalid BLOSSOM_POW_TARGET_SPACING_RATIO");
#if 0
static const unsigned int POST_BLOSSOM_HALVING_INTERVAL = PRE_BLOSSOM_HALVING_INTERVAL * BLOSSOM_POW_TARGET_SPACING_RATIO;
static const unsigned int POST_BLOSSOM_REGTEST_HALVING_INTERVAL = PRE_BLOSSOM_REGTEST_HALVING_INTERVAL * BLOSSOM_POW_TARGET_SPACING_RATIO;
#endif

/**
 * Parameters that influence chain consensus.
 */
struct Params {
    /**
     * Returns true if the given network upgrade is active as of the given block
     * height. Caller must check that the height is >= 0 (and handle unknown
     * heights).
     */
    bool NetworkUpgradeActive(int nHeight, Consensus::UpgradeIndex idx) const;

    uint256 hashGenesisBlock;

    bool fCoinbaseMustBeProtected;

    int nSubsidySlowStartHeight;
    int nSubsidySlowStartInterval;
    int nSubsidyHalvingInterval;
    /**
     * PoR reward percentage of block reward
     */
    int nPorRewardPercentage;
    /**
     * PoR reward percentage of transaction fees
     */
    int nPorRewardTxPercentage;
    int GetLastPorRewardBlockHeight() const { return 0x7fffffff; }
    /**
     * Masternode reward percentage of block reward
     */
    int nMasternodeRewardPercentage;
    /**
     * Masternode reward percentage of transaction fees
     */
    int nMasternodeRewardTxPercentage;
    int GetLastMasternodeRewardBlockHeight() const { return 0x7fffffff; }
    /**
     * PlatformDev fund percentage of block reward
     */
    int nPlatformDevFundPercentage;
    /**
     * PlatformDev fund percentage of transaction fees
     */
    int nPlatformDevFundTxPercentage;
    int GetLastPlatformDevFundBlockHeight() const { return 0x7fffffff; }
    /** Coinbase transaction outputs can only be spent after this number of new blocks (network rule) */
    int nCoinbaseMaturity;

    /** Used to check majorities for block version upgrade */
    int nMajorityEnforceBlockUpgrade;
    int nMajorityRejectBlockOutdated;
    int nMajorityWindow;
    NetworkUpgrade vUpgrades[MAX_NETWORK_UPGRADES];
    /** Proof of work parameters */
    unsigned int nEquihashN = 0;
    unsigned int nEquihashK = 0;
    uint256 powLimit;
    boost::optional<uint32_t> nPowAllowMinDifficultyBlocksAfterHeight;
    int64_t nPowAveragingWindow;
    int64_t nPowMaxAdjustDown;
    int64_t nPowMaxAdjustUp;
    int64_t nPreBlossomPowTargetSpacing;
    int64_t nPostBlossomPowTargetSpacing;

    int64_t PoWTargetSpacing(int nHeight) const;
    int64_t AveragingWindowTimespan(int nHeight) const;
    int64_t MinActualTimespan(int nHeight) const;
    int64_t MaxActualTimespan(int nHeight) const;

    uint256 nMinimumChainWork;
};
} // namespace Consensus

#endif // BITCOIN_CONSENSUS_PARAMS_H
