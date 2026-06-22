#pragma once
#include "../slot-engine/include/slot_engine.h"
#include "statistics.h"
#include <atomic>
#include <tbb/flow_graph.h>

/**
 * @brief Monte Carlo simulator based on TBB flow::graph execution model.
 *
 * FlowGraphSimulator executes slot machine simulations using a directed
 * task graph. Each spin passes through a pipeline of nodes:
 *
 *   Spin generation → Spin evaluation → Statistics aggregation
 *
 */
class FlowGraphSimulator {
public:
  /**
   * @brief Constructs a flow graph simulator.
   *
   * @param config Slot game configuration used for simulation.
   * @param betPerSpin Bet value applied to each spin.
   */
  FlowGraphSimulator(const SlotEngine::GameConfig &config, int betPerSpin);

  /**
   * @brief Runs Monte Carlo simulation using a flow graph pipeline.
   *
   * @param numSpins Number of spins to simulate.
   * @return Aggregated simulation statistics.
   */
  SimulationStatistics Run(int numSpins);

private:
  SlotEngine::GameConfig m_config;
  int m_betPerSpin;

  /// @brief TBB flow graph managing execution pipeline.
  tbb::flow::graph m_graph;

  /**
   * @brief Node generating spins (input: spin index → output: SpinResult).
   */
  tbb::flow::function_node<int, SlotEngine::SpinResult> m_spin_node;

  /**
   * @brief Node processing a spin result into statistics.
   */
  tbb::flow::function_node<SlotEngine::SpinResult, SimulationStatistics>
      m_stats_node;

  /**
   * @brief Final aggregation/reporting node.
   */
  tbb::flow::function_node<SimulationStatistics, int> m_report_node;

  // Atomic accumulators for thread-safe aggregation
  std::atomic<long long> m_totalBet{0};
  std::atomic<long long> m_totalWin{0};
  std::atomic<long long> m_totalBaseWin{0};
  std::atomic<long long> m_totalScatterWin{0};
  std::atomic<long long> m_totalBonusWin{0};
  std::atomic<long long> m_hitCount{0};
  std::atomic<int> m_totalSpins{0};

  /**
   * @brief Generates a single spin for the given index.
   *
   * @param spinIndex Index of the spin in simulation.
   * @return Result of generated spin.
   */
  SlotEngine::SpinResult GenerateSpin(int spinIndex);

  /**
   * @brief Converts a spin result into statistical data.
   *
   * @param result Spin result to process.
   * @return Partial simulation statistics.
   */
  SimulationStatistics ProcessSpin(const SlotEngine::SpinResult &result);

  /**
   * @brief Aggregates and reports statistics from pipeline.
   *
   * @param stats Partial statistics from pipeline stage.
   * @return Dummy integer for graph continuation.
   */
  int ReportStats(const SimulationStatistics &stats);
};
