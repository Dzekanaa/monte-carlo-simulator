#include "../include/monte_carlo_simulator.h"
#include "../include/rtp_calculator.h"
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>

void PrintBanner() {
  std::cout << R"(
              Monte Carlo RTP Simulator v1.0                      
              Parallel Simulation with Intel TBB                 
------------------------------------------------------------------
)";
}

void PrintUsage() {
  std::cout << "\n Usage:\n";
  std::cout << "   ./monte_carlo_simulator [num_spins] [mode]\n\n";
  std::cout << " Modes:\n";
  std::cout << "   parallel    - Run parallel simulation (default)\n";
  std::cout << "   flowgraph   - Run flow graph simulation\n";
  std::cout << "   benchmark   - Run benchmark (1, 2, 4, 8, 16 threads)\n\n";
  std::cout << " Examples:\n";
  std::cout << "   ./monte_carlo_simulator 1000000 parallel\n";
  std::cout << "   ./monte_carlo_simulator 1000000 flowgraph\n";
  std::cout << "   ./monte_carlo_simulator 1000000 benchmark\n";
}

int main(int argc, char *argv[]) {
  PrintBanner();

  int numSpins = 1000000;
  std::string mode = "parallel"; // parallel, flowgraph, benchmark

  if (argc > 1) {
    numSpins = std::atoi(argv[1]);
    if (numSpins <= 0) {
      numSpins = 1000000;
    }
  }

  if (argc > 2) {
    mode = std::string(argv[2]);
    for (char &c : mode) {
      c = std::tolower(c);
    }
  }

  std::cout << "\n Configuration:\n";
  std::cout << "   Number of spins: " << numSpins << "\n";
  std::cout << "   Mode:            " << mode << "\n\n";

  try {
    SlotEngine::GameConfig config =
        SlotEngine::GameConfig::CreateClassicConfig();

    MonteCarloSimulator simulator(config, 1);

    if (mode == "parallel") {
      std::cout << " Running parallel simulation with TBB parallel_reduce...\n";

      auto start = std::chrono::high_resolution_clock::now();
      SimulationStatistics stats = simulator.RunParallel(numSpins);
      auto elapsed = std::chrono::duration<double>(
                         std::chrono::high_resolution_clock::now() - start)
                         .count();

      std::cout << " Simulation completed in " << std::fixed
                << std::setprecision(2) << elapsed << "s\n";

      simulator.PrintReport(stats);

      RTPCalculator rtpCalc(config);
      double rtp = stats.GetRTP();
      bool valid = rtpCalc.ValidateRTP(rtp, 1.0);

      std::cout << "\n RTP Validation:\n";
      std::cout << "   Expected RTP:      " << config.expectedRTP << "%\n";
      std::cout << "   Simulated RTP:     " << std::fixed
                << std::setprecision(2) << rtp << "%\n";
      std::cout << "   Status:            "
                << (valid ? " VALID" : " REVIEW NEEDED") << "\n";

      // Interval poverenja
      RTPCalculator::ConfidenceInterval ci =
          rtpCalc.CalculateConfidenceInterval(stats);
      std::cout << "   95% Confidence:    [" << std::fixed
                << std::setprecision(2) << ci.lower << ", " << ci.upper
                << "]\n";
    }

    else if (mode == "flowgraph") {
      std::cout << " Running flow graph simulation with TBB flow::graph...\n";

      auto start = std::chrono::high_resolution_clock::now();
      SimulationStatistics stats = simulator.RunFlowGraph(numSpins);
      auto elapsed = std::chrono::duration<double>(
                         std::chrono::high_resolution_clock::now() - start)
                         .count();

      std::cout << " Simulation completed in " << std::fixed
                << std::setprecision(2) << elapsed << "s\n";

      if (stats.totalSpins > 0) {
        simulator.PrintReport(stats);
      } else {
        std::cout << "\n Note: Flow graph report was printed above.\n";
      }

      if (stats.totalSpins > 0) {
        RTPCalculator rtpCalc(config);
        double rtp = stats.GetRTP();
        bool valid = rtpCalc.ValidateRTP(rtp, 1.0);

        std::cout << "\n RTP Validation:\n";
        std::cout << "   Expected RTP:      " << config.expectedRTP << "%\n";
        std::cout << "   Simulated RTP:     " << std::fixed
                  << std::setprecision(2) << rtp << "%\n";
        std::cout << "   Status:            "
                  << (valid ? " VALID" : " REVIEW NEEDED") << "\n";
      }
    }

    else if (mode == "benchmark") {
      std::cout << " Running benchmark with 1, 2, 4, 8, 16 threads...\n";
      std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
                   "━━━━━━━━\n\n";

      auto result = simulator.Benchmark(numSpins, 16);

      std::cout << "\n SPEEDUP SUMMARY:\n";
      std::cout << "   Sequential:        " << std::fixed
                << std::setprecision(2) << result.sequentialTime << "s\n";
      std::cout << "   Parallel (TBB):    " << result.parallelTime << "s ("
                << std::fixed << std::setprecision(2) << result.speedupParallel
                << "x)\n";
      std::cout << "   Flow Graph:        " << result.flowGraphTime << "s ("
                << std::fixed << std::setprecision(2) << result.speedupFlowGraph
                << "x)\n";

      std::cout << "\n DETAILED BENCHMARK RESULTS:\n";
      std::cout << "   Total spins:       " << numSpins << "\n";
      std::cout << "   Sequential time:   " << std::fixed
                << std::setprecision(2) << result.sequentialTime << "s\n";
      std::cout << "   Parallel time:     " << result.parallelTime << "s\n";
      std::cout << "   Flow Graph time:   " << result.flowGraphTime << "s\n";
      std::cout << "   Speedup (Par):     " << std::fixed
                << std::setprecision(2) << result.speedupParallel << "x\n";
      std::cout << "   Speedup (Flow):    " << std::fixed
                << std::setprecision(2) << result.speedupFlowGraph << "x\n";

      std::cout << "\n RTP from sequential simulation:\n";
      std::cout << "   Total RTP:         " << std::fixed
                << std::setprecision(2) << result.stats.GetRTP() << "%\n";
      std::cout << "   Hit frequency:     " << std::fixed
                << std::setprecision(2) << result.stats.GetHitFrequency()
                << "%\n";

      // Opciono: ispisati i flow graph report ako je potrebno
      std::cout << "\n Note: Flow graph report was printed during benchmark.\n";
    }

    else {
      std::cout << " Unknown mode: " << mode << "\n";
      PrintUsage();
      return 1;
    }

  } catch (const std::exception &e) {
    std::cerr << "\n Error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
