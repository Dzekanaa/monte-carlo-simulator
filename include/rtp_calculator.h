#pragma once
#include "../slot-engine/include/slot_engine.h"
#include "statistics.h"
#include <vector>

class RTPCalculator {
public:
  RTPCalculator(const SlotEngine::GameConfig &config);
  ~RTPCalculator() = default;

  // Izračunaj teorijski RTP (analitički)
  double CalculateTheoreticalRTP() const;

  // Izračunaj RTP iz simulacije
  double CalculateSimulatedRTP(const SimulationStatistics &stats) const;

  // Validacija: da li je RTP u prihvatljivom opsegu
  bool ValidateRTP(double rtp, double tolerance = 0.5) const;

  // Generiši confidence interval
  struct ConfidenceInterval {
    double lower;
    double upper;
    double confidenceLevel;
  };

  ConfidenceInterval
  CalculateConfidenceInterval(const SimulationStatistics &stats,
                              double confidenceLevel = 0.95) const;

private:
  SlotEngine::GameConfig m_config;
  double m_expectedRTP;

  double CalculateWinProbability(const SlotEngine::Symbol &symbol,
                                 int count) const;
};
