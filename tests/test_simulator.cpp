#include "../include/monte_carlo_simulator.h"
#include "../include/statistics.h"
#include <gtest/gtest.h>
#include <slot_engine.h>

using namespace SlotEngine;

class SimulatorTest : public ::testing::Test {
protected:
  void SetUp() override {
    config = GameConfig::CreateClassicConfig();
    simulator = std::make_unique<MonteCarloSimulator>(config, 10);
  }

  GameConfig config;
  std::unique_ptr<MonteCarloSimulator> simulator;
};

TEST_F(SimulatorTest, SequentialSimulationRuns) {
  auto stats = simulator->RunSequential(1000);
  EXPECT_GT(stats.totalSpins, 0);
  EXPECT_GT(stats.totalBet, 0);
}

TEST_F(SimulatorTest, ParallelSimulationRuns) {
  auto stats = simulator->RunParallel(1000);
  EXPECT_GT(stats.totalSpins, 0);
  EXPECT_GT(stats.totalBet, 0);
}

TEST_F(SimulatorTest, RTPWithinExpectedRange) {
  auto stats = simulator->RunParallel(10000);
  double rtp = stats.GetRTP();
  // RTP bi trebalo da bude oko 96.5% sa tolerancijom od ±5%
  EXPECT_GT(rtp, 91.5);
  EXPECT_LT(rtp, 101.5);
}

TEST_F(SimulatorTest, StatisticsAccumulate) {
  SimulationStatistics stats1;
  stats1.totalSpins = 100;
  stats1.totalBet = 1000;
  stats1.totalWin = 500;

  SimulationStatistics stats2;
  stats2.totalSpins = 200;
  stats2.totalBet = 2000;
  stats2.totalWin = 1000;

  stats1 += stats2;

  EXPECT_EQ(stats1.totalSpins, 300);
  EXPECT_EQ(stats1.totalBet, 3000);
  EXPECT_EQ(stats1.totalWin, 1500);
}

TEST_F(SimulatorTest, FlowGraphSimulationRuns) {
  auto stats = simulator->RunFlowGraph(1000);
  EXPECT_GT(stats.totalSpins, 0);
  EXPECT_GT(stats.totalBet, 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
