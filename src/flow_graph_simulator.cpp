#include "../include/flow_graph_simulator.h"
#include <cmath>
#include <iomanip>
#include <iostream>

using namespace tbb::flow;

// ============================================================================
// Component simulation helpers
// ============================================================================

SimulationStatistics FlowGraphSimulator::SimulateBaseComponent(int numSpins) {
  SimulationStatistics stats;
  stats.totalBet = static_cast<long long>(numSpins) * m_betPerSpin;
  stats.totalSpins = numSpins;

  auto engine = SlotEngine::CreateSlotEngine(m_config);

  for (int i = 0; i < numSpins; ++i) {
    auto result = engine->Spin();
    stats.totalWin += result.totalWin;
    stats.totalBaseWin += result.baseGameWin;
    if (result.IsHit()) {
      stats.hitCount++;
    }
    stats.allWins.push_back(result.baseGameWin);
  }
  return stats;
}

SimulationStatistics FlowGraphSimulator::SimulateBonusComponent(int numSpins) {
  SimulationStatistics stats;
  stats.totalBet = static_cast<long long>(numSpins) * m_betPerSpin;
  stats.totalSpins = numSpins;

  auto engine = SlotEngine::CreateSlotEngine(m_config);

  for (int i = 0; i < numSpins; ++i) {
    auto result = engine->Spin();
    stats.totalWin += result.bonusWin;
    stats.totalBonusWin += result.bonusWin;
  }
  return stats;
}
SimulationStatistics
FlowGraphSimulator::SimulateScatterComponent(int numSpins) {
  SimulationStatistics stats;
  stats.totalBet = static_cast<long long>(numSpins) * m_betPerSpin;
  stats.totalSpins = numSpins;

  auto engine = SlotEngine::CreateSlotEngine(m_config);

  for (int i = 0; i < numSpins; ++i) {
    auto result = engine->Spin();
    stats.totalWin += result.scatterWin;
    stats.totalScatterWin += result.scatterWin;
  }
  return stats;
}

// ============================================================================
// Constructor – builds the graph
// ============================================================================

FlowGraphSimulator::FlowGraphSimulator(const SlotEngine::GameConfig &config,
                                       int betPerSpin)
    : m_config(config), m_betPerSpin(betPerSpin), m_broadcast(m_graph),
      m_base_rtp_node(
          m_graph, unlimited,
          [this](int spins) { return SimulateBaseComponent(spins); }),
      m_bonus_rtp_node(
          m_graph, unlimited,
          [this](int spins) { return SimulateBonusComponent(spins); }),
      m_scatter_rtp_node(
          m_graph, unlimited,
          [this](int spins) { return SimulateScatterComponent(spins); }),
      m_variance_node(m_graph, serial,
                      [this](const SimulationStatistics &baseStats) {
                        return ComputeVariance(baseStats);
                      }),
      m_bonus_buffer(m_graph), m_scatter_buffer(m_graph),
      m_bonus_scatter_join(m_graph),
      m_freespin_impact_node(
          m_graph, serial,
          [this](const std::tuple<SimulationStatistics, SimulationStatistics>
                     &data) { return ComputeFreespinImpact(data); }),
      m_final_join(m_graph),
      m_report_node(
          m_graph, serial,
          [this](const std::tuple<VarianceResult, FreespinImpact> &data) {
            GenerateReport(data);
            return 0;
          }) {
  // Broadcast → all three components
  make_edge(m_broadcast, m_base_rtp_node);
  make_edge(m_broadcast, m_bonus_rtp_node);
  make_edge(m_broadcast, m_scatter_rtp_node);

  // Base → Variance
  make_edge(m_base_rtp_node, m_variance_node);

  // Bonus → Buffer → Join (port 0)
  make_edge(m_bonus_rtp_node, m_bonus_buffer);
  make_edge(m_bonus_buffer, input_port<0>(m_bonus_scatter_join));

  // Scatter → Buffer → Join (port 1)
  make_edge(m_scatter_rtp_node, m_scatter_buffer);
  make_edge(m_scatter_buffer, input_port<1>(m_bonus_scatter_join));

  // Join → Freespin Impact
  make_edge(m_bonus_scatter_join, m_freespin_impact_node);

  // Variance → Final Join (port 0)
  make_edge(m_variance_node, input_port<0>(m_final_join));

  // Freespin Impact → Final Join (port 1)
  make_edge(m_freespin_impact_node, input_port<1>(m_final_join));

  // Final Join → Report
  make_edge(m_final_join, m_report_node);
}

// ============================================================================
// Node computations
// ============================================================================

FlowGraphSimulator::VarianceResult
FlowGraphSimulator::ComputeVariance(const SimulationStatistics &baseStats) {
  VarianceResult result;
  result.stats = baseStats;

  if (baseStats.allWins.empty()) {
    result.variance = 0.0;
    result.stddev = 0.0;
    return result;
  }

  double mean =
      static_cast<double>(baseStats.totalBaseWin) / baseStats.totalSpins;
  double sumSq = 0.0;
  for (long long win : baseStats.allWins) {
    double diff = static_cast<double>(win) - mean;
    sumSq += diff * diff;
  }
  result.variance = sumSq / baseStats.allWins.size();
  result.stddev = std::sqrt(result.variance);
  return result;
}

FlowGraphSimulator::FreespinImpact FlowGraphSimulator::ComputeFreespinImpact(
    const std::tuple<SimulationStatistics, SimulationStatistics>
        &bonusScatter) {

  const auto &bonusStats = std::get<0>(bonusScatter);
  const auto &scatterStats = std::get<1>(bonusScatter);

  FreespinImpact impact;

  double bonusRTP =
      (bonusStats.totalBet > 0)
          ? (static_cast<double>(bonusStats.totalWin) / bonusStats.totalBet) *
                100.0
          : 0.0;

  double scatterRTP = (scatterStats.totalBet > 0)
                          ? (static_cast<double>(scatterStats.totalWin) /
                             scatterStats.totalBet) *
                                100.0
                          : 0.0;

  impact.impact = bonusRTP - scatterRTP;
  return impact;
}

void FlowGraphSimulator::GenerateReport(
    const std::tuple<VarianceResult, FreespinImpact> &finalData) {

  const auto &varRes = std::get<0>(finalData);
  const auto &imp = std::get<1>(finalData);

  std::cout << "\n----- FLOW GRAPH REPORT -----\n";
  std::cout << std::fixed << std::setprecision(2);
  std::cout << "Base RTP (simulated): " << varRes.stats.GetBaseRTP() << "%\n";
  std::cout << "Bonus RTP (simulated): " << varRes.stats.GetBonusRTP() << "%\n";
  std::cout << "Scatter RTP (simulated): " << varRes.stats.GetScatterRTP()
            << "%\n";
  std::cout << "Total RTP: " << varRes.stats.GetRTP() << "%\n";
  std::cout << "Hit frequency: " << varRes.stats.GetHitFrequency() << "%\n";
  std::cout << "Variance (base wins): " << varRes.variance << "\n";
  std::cout << "Standard deviation: " << varRes.stddev << "\n";
  std::cout << "Free‑spin impact (bonus RTP - scatter RTP): " << imp.impact
            << " p.p.\n";
  std::cout << "--------------------------------\n";
}

SimulationStatistics FlowGraphSimulator::Run(int numSpins) {
  // Send spin count to broadcast node
  m_broadcast.try_put(numSpins);

  // Wait for graph to complete
  m_graph.wait_for_all();

  // dummy because report is already printed
  SimulationStatistics stats;

  return stats;
}
