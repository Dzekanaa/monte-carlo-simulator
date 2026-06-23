#include "../include/monte_carlo_simulator.h"
#include "../include/flow_graph_simulator.h"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>

MonteCarloSimulator::MonteCarloSimulator(const SlotEngine::GameConfig &config,
                                         int betPerSpin)
    : m_config(config), m_betPerSpin(betPerSpin) {}

SimulationStatistics
MonteCarloSimulator::PerformSingleSpin(SlotEngine::ISlotEngine &engine) {
  SimulationStatistics stats;

  auto result = engine.Spin();

  stats.totalBet = m_betPerSpin;
  stats.totalWin = result.totalWin;
  stats.totalBaseWin = result.baseGameWin;
  stats.totalScatterWin = result.scatterWin;
  stats.totalBonusWin = result.bonusWin;
  stats.hitCount = result.IsHit() ? 1 : 0;
  stats.totalSpins = 1;
  stats.allWins.push_back(result.totalWin);

  return stats;
}

SimulationStatistics MonteCarloSimulator::RunSequential(int numSpins) {
  std::unique_ptr<SlotEngine::ISlotEngine> engine =
      SlotEngine::CreateSlotEngine(m_config);
  SimulationStatistics totalStats;

  for (int i = 0; i < numSpins; ++i) {
    SimulationStatistics spinStats = PerformSingleSpin(*engine);
    totalStats += spinStats;
  }

  return totalStats;
}

SimulationStatistics MonteCarloSimulator::RunParallel(int numSpins) {
  using namespace tbb;

  enumerable_thread_specific<std::unique_ptr<SlotEngine::ISlotEngine>>
      enginePool;

  SimulationStatistics totalStats = parallel_reduce(
      blocked_range<int>(0, numSpins), SimulationStatistics(),
      [&](const blocked_range<int> &range, SimulationStatistics localStats) {
        if (!enginePool.local()) {
          enginePool.local() = SlotEngine::CreateSlotEngine(m_config);
        }
        SlotEngine::ISlotEngine &engine = *enginePool.local();

        for (int i = range.begin(); i < range.end(); ++i) {
          SimulationStatistics spinStats = PerformSingleSpin(engine);
          localStats += spinStats;
        }
        return localStats;
      },
      [](const SimulationStatistics &a, const SimulationStatistics &b) {
        return a + b;
      });

  return totalStats;
}

SimulationStatistics MonteCarloSimulator::RunFlowGraph(int numSpins) {
  FlowGraphSimulator simulator(m_config, m_betPerSpin);
  return simulator.Run(numSpins);
}

MonteCarloSimulator::BenchmarkResult
MonteCarloSimulator::Benchmark(int numSpins, int maxThreads) {
  BenchmarkResult result;

  std::cout << "\n BENCHMARKING with " << numSpins << " spins\n";
  std::cout << "---------------------------------------------------------------"
               "---\n\n";

  // Sequential
  std::chrono::time_point start = std::chrono::high_resolution_clock::now();
  SimulationStatistics seqStats = RunSequential(numSpins);
  double seqTime = std::chrono::duration<double>(
                       std::chrono::high_resolution_clock::now() - start)
                       .count();
  result.sequentialTime = seqTime;
  result.stats = seqStats;

  std::cout << "Sequential:    " << std::fixed << std::setprecision(2)
            << seqTime << "s\n";

  // Parallel with different number of threads
  std::vector<double> parallelTimes;
  std::vector<double> flowGraphTimes;

  for (int threads = 2; threads <= maxThreads; threads *= 2) {
    tbb::global_control control(tbb::global_control::max_allowed_parallelism,
                                threads);

    // Parallel reduce
    std::chrono::time_point startPar =
        std::chrono::high_resolution_clock::now();
    SimulationStatistics parStats = RunParallel(numSpins);
    double parTime = std::chrono::duration<double>(
                         std::chrono::high_resolution_clock::now() - startPar)
                         .count();
    parallelTimes.push_back(parTime);

    // Flow graph
    std::chrono::time_point startFlow =
        std::chrono::high_resolution_clock::now();
    SimulationStatistics flowStats = RunFlowGraph(numSpins);
    double flowTime = std::chrono::duration<double>(
                          std::chrono::high_resolution_clock::now() - startFlow)
                          .count();
    flowGraphTimes.push_back(flowTime);

    std::cout << " Parallel (" << std::setw(2) << threads
              << " threads): " << std::fixed << std::setprecision(2) << parTime
              << "s (speedup: " << seqTime / parTime << "x)\n";
    std::cout << " FlowGraph (" << std::setw(2) << threads
              << " threads): " << flowTime
              << "s (speedup: " << seqTime / flowTime << "x)\n\n";
  }

  result.parallelTime =
      parallelTimes.empty() ? 0 : parallelTimes[parallelTimes.size() - 1];
  result.flowGraphTime =
      flowGraphTimes.empty() ? 0 : flowGraphTimes[flowGraphTimes.size() - 1];
  result.speedupParallel =
      result.parallelTime > 0 ? seqTime / result.parallelTime : 0;
  result.speedupFlowGraph =
      result.flowGraphTime > 0 ? seqTime / result.flowGraphTime : 0;

  return result;
}

void MonteCarloSimulator::PrintReport(const SimulationStatistics &stats) const {
  stats.PrintReport();
}
