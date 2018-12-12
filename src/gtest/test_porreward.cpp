#include <gtest/gtest.h>

#include "main.h"
#include "utilmoneystr.h"
#include "chainparams.h"
#include "utilstrencodings.h"
#include "zcash/Address.hpp"
#include "wallet/wallet.h"
#include "amount.h"
#include <memory>
#include <string>
#include <set>
#include <vector>
#include <boost/filesystem.hpp>
#include "util.h"

// To run tests:
// ./resistance-gtest --gtest_filter="por_reward_test.*"

//
// Enable this test to generate and print 48 testnet 2-of-3 multisig addresses.
// The output can be copied into chainparams.cpp.
// The temporary wallet file can be renamed as wallet.dat and used for testing with resistanced.
//
#if 0
TEST(por_reward_test, create_testnet_2of3multisig) {
    SelectParams(CBaseChainParams::TESTNET);
    boost::filesystem::path pathTemp = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
    boost::filesystem::create_directories(pathTemp);
    mapArgs["-datadir"] = pathTemp.string();
    bool fFirstRun;
    auto pWallet = std::make_shared<CWallet>("wallet.dat");
    ASSERT_EQ(DB_LOAD_OK, pWallet->LoadWallet(fFirstRun));
    pWallet->TopUpKeyPool();
    std::cout << "Test wallet and logs saved in folder: " << pathTemp.native() << std::endl;
    
    int numKeys = 48;
    std::vector<CPubKey> pubkeys;
    pubkeys.resize(3);
    CPubKey newKey;
    std::vector<std::string> addresses;
    for (int i = 0; i < numKeys; i++) {
        ASSERT_TRUE(pWallet->GetKeyFromPool(newKey));
        pubkeys[0] = newKey;
        pWallet->SetAddressBook(newKey.GetID(), "", "receive");

        ASSERT_TRUE(pWallet->GetKeyFromPool(newKey));
        pubkeys[1] = newKey;
        pWallet->SetAddressBook(newKey.GetID(), "", "receive");

        ASSERT_TRUE(pWallet->GetKeyFromPool(newKey));
        pubkeys[2] = newKey;
        pWallet->SetAddressBook(newKey.GetID(), "", "receive");

        CScript result = GetScriptForMultisig(2, pubkeys);
        ASSERT_FALSE(result.size() > MAX_SCRIPT_ELEMENT_SIZE);
        CScriptID innerID(result);
        pWallet->AddCScript(result);
        pWallet->SetAddressBook(innerID, "", "receive");

        std::string address = EncodeDestination(innerID);
        addresses.push_back(address);
    }
    
    // Print out the addresses, 4 on each line.
    std::string s = "vPorRewardAddress = {\n";
    int i=0;
    int colsPerRow = 4;
    ASSERT_TRUE(numKeys % colsPerRow == 0);
    int numRows = numKeys/colsPerRow;
    for (int row=0; row<numRows; row++) {
        s += "    ";
        for (int col=0; col<colsPerRow; col++) {
            s += "\"" + addresses[i++] + "\", ";
        }
        s += "\n";
    }
    s += "    };";
    std::cout << s << std::endl;

    pWallet->Flush(true);
}
#endif


// Utility method to check the number of unique addresses from height 1 to maxHeight
void checkNumberOfUniqueAddresses(int nUnique) {
    int maxHeight = Params().GetConsensus().GetLastPorRewardBlockHeight();
    std::set<std::string> addresses;
    for (int i = 1; i <= maxHeight; i++) {
        addresses.insert(Params().GetPorRewardAddressAtHeight(i));
    }
    ASSERT_TRUE(addresses.size() == nUnique);
}


TEST(por_reward_test, general) {
    SelectParams(CBaseChainParams::TESTNET);

    CChainParams params = Params();
    
    // Fourth testnet reward:
    // address = t2ENg7hHVqqs9JwU5cgjvSbxnT2a9USNfhy
    // script.ToString() = OP_HASH160 55d64928e69829d9376c776550b6cc710d427153 OP_EQUAL
    // HexStr(script) = a91455d64928e69829d9376c776550b6cc710d42715387
    EXPECT_EQ(HexStr(params.GetPorRewardScriptAtHeight(1)), "a914914331d3e17790c0b11126147e038453a4a9adee87");
    EXPECT_EQ(params.GetPorRewardAddressAtHeight(1), "rs8MA1zvQTjx5VHKmoDwqMWNbnG35GvcmEs");
    EXPECT_EQ(HexStr(params.GetPorRewardScriptAtHeight(53126)), "a914914331d3e17790c0b11126147e038453a4a9adee87");
    EXPECT_EQ(params.GetPorRewardAddressAtHeight(53126), "rs8MA1zvQTjx5VHKmoDwqMWNbnG35GvcmEs");
    EXPECT_EQ(HexStr(params.GetPorRewardScriptAtHeight(53127)), "a914914331d3e17790c0b11126147e038453a4a9adee87");
    EXPECT_EQ(params.GetPorRewardAddressAtHeight(53127), "rs8MA1zvQTjx5VHKmoDwqMWNbnG35GvcmEs");

    int maxHeight = params.GetConsensus().GetLastPorRewardBlockHeight();
    
    // If the block height parameter is out of bounds, there is an assert.
    EXPECT_DEATH(params.GetPorRewardScriptAtHeight(0), "nHeight");
    EXPECT_DEATH(params.GetPorRewardScriptAtHeight(maxHeight+1), "nHeight");
    EXPECT_DEATH(params.GetPorRewardAddressAtHeight(0), "nHeight");
    EXPECT_DEATH(params.GetPorRewardAddressAtHeight(maxHeight+1), "nHeight"); 
}


#define NUM_MAINNET_POR_ADDRESSES 1

TEST(por_reward_test, mainnet) {
    SelectParams(CBaseChainParams::MAIN);
    checkNumberOfUniqueAddresses(NUM_MAINNET_POR_ADDRESSES);
}


#define NUM_TESTNET_POR_ADDRESSES 1

TEST(por_reward_test, testnet) {
    SelectParams(CBaseChainParams::TESTNET);
    checkNumberOfUniqueAddresses(NUM_TESTNET_POR_ADDRESSES);
}


#define NUM_REGTEST_POR_ADDRESSES 1

TEST(por_reward_test, regtest) {
    SelectParams(CBaseChainParams::REGTEST);
    checkNumberOfUniqueAddresses(NUM_REGTEST_POR_ADDRESSES);
}



// Test that PoR reward is fully rewarded after the first halving and slow start shift.
TEST(por_reward_test, slow_start_subsidy) {
#if 0
    SelectParams(CBaseChainParams::MAIN);
    CChainParams params = Params();

    int maxHeight = params.GetConsensus().GetLastPorRewardBlockHeight();    
    CAmount totalSubsidy = 0;
    for (int nHeight = 1; nHeight <= maxHeight; nHeight++) {
        CAmount nSubsidy = GetBlockSubsidy(nHeight, params.GetConsensus()) / 5;
        totalSubsidy += nSubsidy;
    }
    
    ASSERT_TRUE(totalSubsidy == MAX_MONEY/10.0);
#endif
}


// For use with mainnet and testnet which each have 48 addresses.
// Verify the number of rewards each individual address receives.
void verifyNumberOfRewards() {
#if 0
    CChainParams params = Params();
    int maxHeight = params.GetConsensus().GetLastPorRewardBlockHeight();
    std::multiset<std::string> ms;
    for (int nHeight = 1; nHeight <= maxHeight; nHeight++) {
        ms.insert(params.GetPorRewardAddressAtHeight(nHeight));
    }

    ASSERT_TRUE(ms.count(params.GetPorRewardAddressAtIndex(0)) == maxHeight);
    for (int i = 1; i <= 46; i++) {
        ASSERT_TRUE(ms.count(params.GetPorRewardAddressAtIndex(i)) == maxHeight);
    }
    ASSERT_TRUE(ms.count(params.GetPorRewardAddressAtIndex(47)) == maxHeight);
#endif
}

// Verify the number of rewards going to each mainnet address
TEST(por_reward_test, per_address_reward_mainnet) {
    SelectParams(CBaseChainParams::MAIN);
    verifyNumberOfRewards();
}

// Verify the number of rewards going to each testnet address
TEST(por_reward_test, per_address_reward_testnet) {
    SelectParams(CBaseChainParams::TESTNET);
    verifyNumberOfRewards();
}
