#include "../include/rtp_calculator.h"
#include <cmath>
#include <random>

RTPCalculator::RTPCalculator(const SlotEngine::GameConfig &config)
    : m_config(config), m_expectedRTP(config.expectedRTP) {}

double RTPCalculator::CalculateTheoreticalRTP() const {
  double totalRTP = 0.0;

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dist(0, 1000);

  const int NUM_SAMPLES = 10000;
  long long totalBet = 0;
  long long totalWin = 0;

  auto engine = SlotEngine::CreateSlotEngine(m_config);

  for (int i = 0; i < NUM_SAMPLES; ++i) {
    auto result = engine->Spin();
    totalBet += 10;
    totalWin += result.totalWin;
  }

  return (double)totalWin / (double)totalBet * 100.0;
}

double
RTPCalculator::CalculateSimulatedRTP(const SimulationStatistics &stats) const {
  return stats.GetRTP();
}

bool RTPCalculator::ValidateRTP(double rtp, double tolerance) const {
  double diff = std::abs(rtp - m_expectedRTP);
  return diff <= tolerance;
}

RTPCalculator::ConfidenceInterval
RTPCalculator::CalculateConfidenceInterval(const SimulationStatistics &stats,
                                           double confidenceLevel) const {

  ConfidenceInterval ci;
  ci.confidenceLevel = confidenceLevel;

  double mean = (double)stats.totalWin / stats.totalSpins;
  double stdDev = stats.GetStandardDeviation();
  double n = stats.totalSpins;

  // Z-score for 95% confidence level is 1.96
  double zScore = 1.96;
  if (confidenceLevel == 0.99)
    zScore = 2.576;
  if (confidenceLevel == 0.90)
    zScore = 1.645;

  double marginOfError = zScore * (stdDev / std::sqrt(n));

  ci.lower = mean - marginOfError;
  ci.upper = mean + marginOfError;

  return ci;
}

double RTPCalculator::CalculateWinProbability(const SlotEngine::Symbol &symbol,
                                              int count) const {
  // Verovatnoća dobijanja count simbola na paylini
  // Ovo je pojednostavljena verzija
  return 0.01; // placeholder
}
