#include "../include/statistics.h"
#include <algorithm>
#include <iomanip>
#include <iostream>

SimulationStatistics &
SimulationStatistics::operator+=(const SimulationStatistics &other) {
  totalBet += other.totalBet;
  totalWin += other.totalWin;
  totalBaseWin += other.totalBaseWin;
  totalScatterWin += other.totalScatterWin;
  totalBonusWin += other.totalBonusWin;
  hitCount += other.hitCount;
  totalSpins += other.totalSpins;

  allWins.insert(allWins.end(), other.allWins.begin(), other.allWins.end());

  return *this;
}

double SimulationStatistics::GetRTP() const {
  if (totalBet == 0)
    return 0.0;
  return (double)totalWin / (double)totalBet * 100.0;
}

double SimulationStatistics::GetHitFrequency() const {
  if (totalSpins == 0)
    return 0.0;
  return (double)hitCount / (double)totalSpins * 100.0;
}

double SimulationStatistics::GetVariance() const {
  if (allWins.empty())
    return 0.0;

  double mean = (double)totalWin / totalSpins;
  double sumSquaredDiff = 0.0;

  for (long long win : allWins) {
    double diff = (double)win - mean;
    sumSquaredDiff += diff * diff;
  }

  return sumSquaredDiff / allWins.size();
}

double SimulationStatistics::GetStandardDeviation() const {
  return std::sqrt(GetVariance());
}

double SimulationStatistics::GetBaseRTP() const {
  if (totalBet == 0)
    return 0.0;
  return (double)totalBaseWin / (double)totalBet * 100.0;
}

double SimulationStatistics::GetScatterRTP() const {
  if (totalBet == 0)
    return 0.0;
  return (double)totalScatterWin / (double)totalBet * 100.0;
}

double SimulationStatistics::GetBonusRTP() const {
  if (totalBet == 0)
    return 0.0;
  return (double)totalBonusWin / (double)totalBet * 100.0;
}

void SimulationStatistics::PrintReport() const {
  std::cout << "                    MONTE CARLO RTP REPORT                    "
            << std::endl;
  std::cout
      << "-----------------------------------------------------------------\n";

  std::cout << std::fixed << std::setprecision(2);

  std::cout << " GENERAL STATISTICS:\n";
  std::cout << "   Total spins:        " << totalSpins << "\n";
  std::cout << "   Total bet:          " << totalBet << " coins\n";
  std::cout << "   Total win:          " << totalWin << " coins\n";
  std::cout << "   Net result:         " << (totalWin - totalBet)
            << " coins\n\n";

  std::cout << " RTP BREAKDOWN:\n";
  std::cout << "   Total RTP:          " << GetRTP() << "%\n";
  std::cout << "   Base game RTP:      " << GetBaseRTP() << "%\n";
  std::cout << "   Scatter RTP:        " << GetScatterRTP() << "%\n";
  std::cout << "   Bonus RTP:          " << GetBonusRTP() << "%\n\n";

  std::cout << " VOLATILITY:\n";
  std::cout << "   Hit frequency:      " << GetHitFrequency() << "%\n";
  std::cout << "   Variance:           " << GetVariance() << "\n";
  std::cout << "   Std deviation:      " << GetStandardDeviation() << "\n\n";

  std::cout << " WIN DISTRIBUTION (top 10):\n";
  std::vector<long long> sortedWins = allWins;
  std::sort(sortedWins.begin(), sortedWins.end(), std::greater<long long>());

  for (int i = 0; i < std::min(10, (int)sortedWins.size()); ++i) {
    std::cout << "   #" << std::setw(2) << i + 1 << ": " << sortedWins[i]
              << " coins\n";
  }
  std::cout << "\n";
}
