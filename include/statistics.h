#pragma once
#include <atomic>
#include <cmath>
#include <vector>

struct SimulationStatistics {
  // Accumulators
  long long totalBet = 0;
  long long totalWin = 0;
  long long totalBaseWin = 0;
  long long totalScatterWin = 0;
  long long totalBonusWin = 0;
  long long hitCount = 0;
  int totalSpins = 0;

  // For variance calculation (store all wins for histogram)
  std::vector<long long> allWins;

  // Operators for parallel_reduce
  SimulationStatistics &operator+=(const SimulationStatistics &other);

  // Calculate derived statistics
  double GetRTP() const;
  double GetHitFrequency() const;
  double GetVariance() const;
  double GetStandardDeviation() const;

  // Per-component RTP
  double GetBaseRTP() const;
  double GetScatterRTP() const;
  double GetBonusRTP() const;

  // Print formatted report
  void PrintReport() const;
};

// For parallel_reduce identity
inline SimulationStatistics make_identity() { return SimulationStatistics{}; }
inline SimulationStatistics operator+(const SimulationStatistics &a,
                                      const SimulationStatistics &b) {
  SimulationStatistics result = a;
  result += b;
  return result;
}
