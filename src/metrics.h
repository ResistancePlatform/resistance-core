// Copyright (c) 2016 The Zcash developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://www.opensource.org/licenses/mit-license.php .

#include "uint256.h"
#include "consensus/params.h"

#include <atomic>
#include <mutex>
#include <string>

struct AtomicCounter {
    std::atomic<uint64_t> value;

    AtomicCounter() : value {0} { }

    void increment(){
        ++value;
    }

    void decrement(){
        --value;
    }

    int get() const {
        return value.load();
    }
};

class AtomicTimer {
private:
    std::mutex mtx;
    uint64_t threads;
    int64_t start_time;
    int64_t total_time;

public:
    AtomicTimer() : threads(0), start_time(0), total_time(0) {}

    /**
     * Starts timing on first call, and counts the number of calls.
     */
    void start();

    /**
     * Counts number of calls, and stops timing after it has been called as
     * many times as start().
     */
    void stop();

    bool running();

    uint64_t threadCount();

    double rate(const AtomicCounter& count);
};

enum DurationFormat {
    FULL,
    REDUCED
};

extern AtomicCounter transactionsValidated;
extern AtomicCounter ehSolverRuns;
extern AtomicCounter solutionTargetChecks;
extern AtomicTimer miningTimer;

void TrackMinedBlock(uint256 hash);

void MarkStartTime();
double GetLocalSolPS();
int EstimateNetHeight(const Consensus::Params& params, int currentBlockHeight, int64_t currentBlockTime);
boost::optional<int64_t> SecondsLeftToNextEpoch(const Consensus::Params& params, int currentHeight);
std::string DisplayDuration(int64_t time, DurationFormat format);

void TriggerRefresh();

void ConnectMetricsScreen();
void ThreadShowMetricsScreen();

/**
 * Rendering options:
 * Resistance: img2txt -W 52 -H 27 -f utf8 -d none -g 0.7 Resistance-logo.png
 */
const std::string METRICS_ART =
"                   ;t:                             \n"
"                  @:88t                            \n"
"                  88888.                           \n"
"                   :S8;                            \n"
"                   :t8@ %@%:                       \n"
"                  ;;8X88%8X88 t8%:.     ..8X88     \n"
"                 :.88.t;   t8S88@8S8S.  .8:; @;.   \n"
"                t 8S:      .:  ;8 S888@S8.8X 8.    \n"
"             ..88S:   .        .:.  :S 8%          \n"
"              @8 X    :::::::..      .888          \n"
"            :%@ 8.  . XXSXX@@X t;    :88X          \n"
"          .S 88t    .8tXX8Xt8tS@X.  .:888.         \n"
"   .@  888:X S..  . :8X888@8@X%@8.   :88%          \n"
"   t@8888@8X8t   .  .8S8X888%%@@; .  :888          \n"
"   .% 88t:% @%:.   ..8@8Xt8@@.8.    ..888;         \n"
"    .;t.  ..@888..  .%XX8 ::S88.S8.  :8 ;          \n"
"             88 @       .  .8X88%8;  :88:          \n"
"              8S 8..  .      :S@%: . .8 ;          \n"
"               t8@8;.    .       .:%8t@888:        \n"
"               ;;XX 8.     .:X8; XXS 888S8XX S     \n"
"                  88 X. :%8 X88S 8t:%SXt 8@888t    \n"
"                    X88 S X 88%;:       :;8 88:    \n"
"                   :88 8Xt ::            .;t%.     \n"
"                   % 8                             \n"
"                  8X@ 8                            \n"
"                  8S888:                           \n"
"                     S.                            \n";
