#pragma once
#include <atomic>
#include <cmath>
#include <vector>

/**
 * @brief Aggregates statistics collected during slot machine simulation.
 *
 * SimulationStatistics stores cumulative results from multiple spins,
 * including total bets, wins, hit rate, and detailed breakdowns of
 * base game, scatter, and bonus contributions.
 *
 * It also supports statistical analysis such as RTP, variance, and
 * standard deviation, making it suitable for Monte Carlo simulations
 * and game balancing.
 */
struct SimulationStatistics {
  long long totalBet = 0;
  long long totalWin = 0;
  long long totalBaseWin = 0;
  long long totalScatterWin = 0;
  long long totalBonusWin = 0;
  long long hitCount = 0;
  int totalSpins = 0;

  // For variance calculation (store all wins for histogram)
  std::vector<long long> allWins;

  /**
   * @brief Merges another SimulationStatistics object into this one.
   *
   * Used for parallel reduction when aggregating simulation results
   * across multiple threads.
   *
   * @param other Statistics object to merge.
   * @return Reference to this updated object.
   */
  SimulationStatistics &operator+=(const SimulationStatistics &other);

  /**
   * @brief Calculates Return To Player (RTP).
   *
   * @return RTP as a percentage (totalWin / totalBet * 100).
   */
  double GetRTP() const;

  /**
   * @brief Calculates hit frequency.
   *
   * Hit frequency is the ratio of spins that produced any win.
   *
   * @return Value in range [0, 1].
   */
  double GetHitFrequency() const;

  /**
   * @brief Calculates variance of spin winnings.
   *
   * Measures the dispersion of wins around the mean value.
   *
   * @return Variance of all recorded wins.
   */
  double GetVariance() const;

  /**
   * @brief Calculates standard deviation of winnings.
   *
   * @return Standard deviation derived from variance.
   */
  double GetStandardDeviation() const;

  /**
   * @brief Calculates RTP contribution from base game only.
   */
  double GetBaseRTP() const;

  /**
   * @brief Calculates RTP contribution from scatter wins.
   */
  double GetScatterRTP() const;

  /**
   * @brief Calculates RTP contribution from bonus wins.
   */
  double GetBonusRTP() const;

  /**
   * @brief Prints a formatted simulation report.
   *
   * Outputs all key metrics including RTP, hit rate, and variance.
   */
  void PrintReport() const;
};

/**
 * @brief Creates an identity (zero-initialized) statistics object.
 *
 * Used as a neutral element for parallel reduction operations.
 */
inline SimulationStatistics make_identity() { return SimulationStatistics{}; }

/**
 * @brief Combines two SimulationStatistics objects.
 *
 * @param a First statistics object.
 * @param b Second statistics object.
 * @return Combined statistics containing sums of both inputs.
 */
inline SimulationStatistics operator+(const SimulationStatistics &a,
                                      const SimulationStatistics &b) {
  SimulationStatistics result = a;
  result += b;
  return result;
}
