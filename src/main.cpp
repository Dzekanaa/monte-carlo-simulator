#include "../include/monte_carlo_simulator.h"
#include "../include/rtp_calculator.h"
#include <cstdlib>
#include <iomanip>
#include <iostream>

void PrintBanner() {
  std::cout << R"(
╔══════════════════════════════════════════════════════════════════╗
║                                                                  ║
║     ███╗   ███╗ ██████╗ ███╗   ██╗████████╗███████╗             ║
║     ████╗ ████║██╔═══██╗████╗  ██║╚══██╔══╝██╔════╝             ║
║     ██╔████╔██║██║   ██║██╔██╗ ██║   ██║   █████╗               ║
║     ██║╚██╔╝██║██║   ██║██║╚██╗██║   ██║   ██╔══╝               ║
║     ██║ ╚═╝ ██║╚██████╔╝██║ ╚████║   ██║   ███████╗             ║
║     ╚═╝     ╚═╝ ╚═════╝ ╚═╝  ╚═══╝   ╚═╝   ╚══════╝             ║
║                                                                  ║
║              Monte Carlo RTP Simulator v1.0                      ║
║              Parallel Simulation with Intel TBB                 ║
║                                                                  ║
╚══════════════════════════════════════════════════════════════════╝
)";
}

int main(int argc, char *argv[]) {
  PrintBanner();

  // Parametri
  int numSpins = 1000000; // 1 million by default
  bool runBenchmark = false;

  if (argc > 1) {
    numSpins = std::atoi(argv[1]);
    if (numSpins <= 0)
      numSpins = 1000000;
  }

  if (argc > 2 && std::string(argv[2]) == "--benchmark") {
    runBenchmark = true;
  }

  std::cout << "\n📋 Configuration:\n";
  std::cout << "   Number of spins: " << numSpins << "\n";
  std::cout << "   Benchmark mode:  " << (runBenchmark ? "ON" : "OFF")
            << "\n\n";

  try {
    // Kreiraj konfiguraciju
    auto config = SlotEngine::GameConfig::CreateClassicConfig();

    // Kreiraj simulator
    MonteCarloSimulator simulator(config, 1);

    if (runBenchmark) {
      // Benchmark mode
      auto result = simulator.Benchmark(numSpins, 8);
      simulator.PrintReport(result.stats);

      std::cout << "\n⚡ SPEEDUP SUMMARY:\n";
      std::cout << "   Sequential:        " << std::fixed
                << std::setprecision(2) << result.sequentialTime << "s\n";
      std::cout << "   Parallel (TBB):    " << result.parallelTime << "s ("
                << result.speedupParallel << "x)\n";
      std::cout << "   Flow Graph:        " << result.flowGraphTime << "s ("
                << result.speedupFlowGraph << "x)\n";
    } else {
      // Standard mode - parallel
      std::cout << "⏳ Running parallel simulation with TBB...\n";
      auto start = std::chrono::high_resolution_clock::now();

      auto stats = simulator.RunParallel(numSpins);

      auto elapsed = std::chrono::duration<double>(
                         std::chrono::high_resolution_clock::now() - start)
                         .count();

      std::cout << "✅ Simulation completed in " << std::fixed
                << std::setprecision(2) << elapsed << "s\n";

      simulator.PrintReport(stats);

      // Validacija RTP
      RTPCalculator rtpCalc(config);
      double rtp = stats.GetRTP();
      bool valid = rtpCalc.ValidateRTP(rtp);

      std::cout << "\n🔍 RTP Validation:\n";
      std::cout << "   Expected RTP:      " << config.expectedRTP << "%\n";
      std::cout << "   Simulated RTP:     " << rtp << "%\n";
      std::cout << "   Status:            "
                << (valid ? "✅ VALID" : "⚠️  REVIEW NEEDED") << "\n";

      // Confidence interval
      auto ci = rtpCalc.CalculateConfidenceInterval(stats);
      std::cout << "   95% Confidence:    [" << ci.lower << ", " << ci.upper
                << "]\n";
    }

  } catch (const std::exception &e) {
    std::cerr << "❌ Error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
