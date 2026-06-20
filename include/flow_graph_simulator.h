#pragma once
#include "../slot-engine/include/slot_engine.h"
#include "statistics.h"
#include <atomic>
#include <tbb/flow_graph.h>

class FlowGraphSimulator {
public:
  FlowGraphSimulator(const SlotEngine::GameConfig &config, int betPerSpin);
  SimulationStatistics Run(int numSpins);

private:
  SlotEngine::GameConfig m_config;
  int m_betPerSpin;

  tbb::flow::graph m_graph;

  // Declare nodes – they will be initialized in the constructor
  tbb::flow::function_node<int, SlotEngine::SpinResult> m_spin_node;
  tbb::flow::function_node<SlotEngine::SpinResult, SimulationStatistics>
      m_stats_node;
  tbb::flow::function_node<SimulationStatistics, int> m_report_node;

  std::atomic<long long> m_totalBet{0};
  std::atomic<long long> m_totalWin{0};
  std::atomic<long long> m_totalBaseWin{0};
  std::atomic<long long> m_totalScatterWin{0};
  std::atomic<long long> m_totalBonusWin{0};
  std::atomic<long long> m_hitCount{0};
  std::atomic<int> m_totalSpins{0};

  SlotEngine::SpinResult GenerateSpin(int);
  SimulationStatistics ProcessSpin(const SlotEngine::SpinResult &);
  int ReportStats(const SimulationStatistics &);
};
