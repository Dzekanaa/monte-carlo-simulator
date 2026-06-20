#pragma once
#include "../slot-engine/include/slot_engine.h"
#include "statistics.h"
#include <memory>
#include <tbb/tbb.h>
#include <vector>

class MonteCarloSimulator {
public:
  explicit MonteCarloSimulator(const SlotEngine::GameConfig &config,
                               int betPerSpin = 10);
  ~MonteCarloSimulator() = default;

  // Sekvencijalna simulacija (za benchmark)
  SimulationStatistics RunSequential(int numSpins);

  // Paralelna simulacija sa TBB parallel_reduce
  SimulationStatistics RunParallel(int numSpins);

  // Paralelna simulacija sa flow::graph
  SimulationStatistics RunFlowGraph(int numSpins);

  // Benchmark različitih pristupa
  struct BenchmarkResult {
    int numThreads;
    double sequentialTime;
    double parallelTime;
    double flowGraphTime;
    double speedupParallel;
    double speedupFlowGraph;
    SimulationStatistics stats;
  };

  BenchmarkResult Benchmark(int numSpins, int maxThreads = 16);

  // Ispis rezultata
  void PrintReport(const SimulationStatistics &stats) const;

private:
  SlotEngine::GameConfig m_config;
  int m_betPerSpin;

  // Helper za simulaciju jednog spina
  SimulationStatistics PerformSingleSpin(SlotEngine::ISlotEngine &engine);

  // Statistika za flow::graph
  struct ComponentStats {
    long long totalBet = 0;
    long long totalWin = 0;
    long long hitCount = 0;
    int spins = 0;
  };
};
