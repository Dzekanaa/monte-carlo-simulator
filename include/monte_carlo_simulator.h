#pragma once
#include "../slot-engine/include/slot_engine.h"
#include "statistics.h"
#include <memory>
#include <tbb/tbb.h>
#include <vector>

/**
 * @brief Monte Carlo simulation engine for slot machine analysis.
 *
 * MonteCarloSimulator is responsible for running large-scale simulations
 * of slot machine spins in order to evaluate statistical properties such as
 * RTP, hit frequency, variance, and distribution of outcomes.
 *
 * It supports sequential execution, parallel execution (TBB parallel_reduce),
 * and flow graph-based execution for performance benchmarking.
 */
class MonteCarloSimulator {
public:
  /**
   * @brief Constructs a Monte Carlo simulator.
   *
   * @param config Slot game configuration used for simulation.
   * @param betPerSpin Bet value used for each simulated spin.
   */
  explicit MonteCarloSimulator(const SlotEngine::GameConfig &config,
                               int betPerSpin = 10);

  ~MonteCarloSimulator() = default;

  /**
   * @brief Runs sequential Monte Carlo simulation.
   *
   * This method executes spins in a single thread and is mainly used
   * as a baseline for performance benchmarking.
   *
   * @param numSpins Number of spins to simulate.
   * @return Aggregated simulation statistics.
   */
  SimulationStatistics RunSequential(int numSpins);

  /**
   * @brief Runs parallel Monte Carlo simulation using TBB parallel_reduce.
   *
   * Splits the workload across multiple threads for improved performance.
   *
   * @param numSpins Number of spins to simulate.
   * @return Aggregated simulation statistics.
   */
  SimulationStatistics RunParallel(int numSpins);

  /**
   * @brief Runs Monte Carlo simulation using TBB flow::graph.
   *
   * Uses a task graph model for parallel execution and workload distribution.
   *
   * @param numSpins Number of spins to simulate.
   * @return Aggregated simulation statistics.
   */
  SimulationStatistics RunFlowGraph(int numSpins);

  /**
   * @brief Stores benchmark results comparing different execution modes.
   */
  struct BenchmarkResult {
    int numThreads;
    double sequentialTime;
    double parallelTime;
    double flowGraphTime;
    double speedupParallel;
    double speedupFlowGraph;
    SimulationStatistics stats;
  };

  /**
   * @brief Benchmarks sequential vs parallel execution strategies.
   *
   * Measures execution time and speedup for different thread counts.
   *
   * @param numSpins Number of spins to simulate.
   * @param maxThreads Maximum number of threads to test.
   * @return Benchmark results containing timing and statistics.
   */
  BenchmarkResult Benchmark(int numSpins, int maxThreads = 16);

  /**
   * @brief Prints formatted simulation report.
   *
   * @param stats Simulation statistics to display.
   */
  void PrintReport(const SimulationStatistics &stats) const;

private:
  SlotEngine::GameConfig m_config;
  int m_betPerSpin;

  /**
   * @brief Executes a single spin using the provided engine instance.
   *
   * Used internally for simulation aggregation.
   *
   * @param engine Slot engine used for spin execution.
   * @return Statistics for a single spin.
   */
  SimulationStatistics PerformSingleSpin(SlotEngine::ISlotEngine &engine);

  /**
   * @brief Internal statistics used for flow graph aggregation.
   */
  struct ComponentStats {
    long long totalBet = 0;
    long long totalWin = 0;
    long long hitCount = 0;
    int spins = 0;
  };
};
