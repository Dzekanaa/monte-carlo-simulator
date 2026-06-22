#pragma once
#include "../slot-engine/include/slot_engine.h"
#include "statistics.h"
#include <vector>

/**
 * @brief Calculates and validates Return To Player (RTP) metrics.
 *
 * RTPCalculator is responsible for computing both theoretical RTP
 * (based on game configuration and probabilities) and simulated RTP
 * (based on Monte Carlo simulation results). It also provides validation
 * and statistical confidence interval estimation for RTP analysis.
 */
class RTPCalculator {
public:
  /**
   * @brief Constructs RTP calculator using a game configuration.
   *
   * @param config Slot game configuration containing reels, paytable and
   * paylines.
   */
  RTPCalculator(const SlotEngine::GameConfig &config);

  ~RTPCalculator() = default;

  /**
   * @brief Calculates theoretical RTP based on game math model.
   *
   * This method estimates RTP analytically using symbol probabilities,
   * paytable values, and game rules without running simulations.
   *
   * @return Theoretical RTP value.
   */
  double CalculateTheoreticalRTP() const;

  /**
   * @brief Calculates RTP from simulation results.
   *
   * @param stats Aggregated simulation statistics.
   * @return Empirical RTP computed from simulated spins.
   */
  double CalculateSimulatedRTP(const SimulationStatistics &stats) const;

  /**
   * @brief Validates whether RTP is within acceptable tolerance range.
   *
   * @param rtp RTP value to validate.
   * @param tolerance Allowed deviation from expected RTP (in percentage
   * points).
   * @return true if RTP is within tolerance range, false otherwise.
   */
  bool ValidateRTP(double rtp, double tolerance = 0.5) const;

  /**
   * @brief Represents confidence interval for RTP estimation.
   */
  struct ConfidenceInterval {
    double lower;
    double upper;
    double confidenceLevel;
  };

  /**
   * @brief Calculates confidence interval for simulated RTP.
   *
   * Uses statistical methods (typically normal approximation) to estimate
   * the range in which the true RTP lies with a given confidence level.
   *
   * @param stats Simulation statistics.
   * @param confidenceLevel Confidence level (default 0.95).
   * @return Confidence interval for RTP.
   */
  ConfidenceInterval
  CalculateConfidenceInterval(const SimulationStatistics &stats,
                              double confidenceLevel = 0.95) const;

private:
  SlotEngine::GameConfig m_config;
  double m_expectedRTP;

  /**
   * @brief Calculates probability contribution of a symbol occurrence.
   *
   * Used as a helper for theoretical RTP calculation.
   *
   * @param symbol Slot symbol.
   * @param count Number of consecutive matches.
   * @return Probability-weighted contribution to RTP.
   */
  double CalculateWinProbability(const SlotEngine::Symbol &symbol,
                                 int count) const;
};
