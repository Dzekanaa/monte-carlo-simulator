#include "../include/flow_graph_simulator.h"

using namespace tbb::flow;

FlowGraphSimulator::FlowGraphSimulator(const SlotEngine::GameConfig &config,
                                       int betPerSpin)
    : m_config(config), m_betPerSpin(betPerSpin),

      m_spin_node(m_graph, unlimited, [this](int) { return GenerateSpin(0); }),
      m_stats_node(
          m_graph, unlimited,
          [this](const SlotEngine::SpinResult &r) { return ProcessSpin(r); }),
      m_report_node(m_graph, serial, [this](const SimulationStatistics &s) {
        return ReportStats(s);
      }) {

  // Connect the nodes
  make_edge(m_spin_node, m_stats_node);
  make_edge(m_stats_node, m_report_node);
}

SlotEngine::SpinResult FlowGraphSimulator::GenerateSpin(int) {
  auto engine = SlotEngine::CreateSlotEngine(m_config);
  return engine->Spin();
}

SimulationStatistics
FlowGraphSimulator::ProcessSpin(const SlotEngine::SpinResult &result) {
  m_totalBet += m_betPerSpin;
  m_totalWin += result.totalWin;
  m_totalBaseWin += result.baseGameWin;
  m_totalScatterWin += result.scatterWin;
  m_totalBonusWin += result.bonusWin;
  if (result.IsHit())
    m_hitCount++;
  m_totalSpins++;

  SimulationStatistics stats;
  stats.totalBet = m_betPerSpin;
  stats.totalWin = result.totalWin;
  stats.totalBaseWin = result.baseGameWin;
  stats.totalScatterWin = result.scatterWin;
  stats.totalBonusWin = result.bonusWin;
  stats.hitCount = result.IsHit() ? 1 : 0;
  stats.totalSpins = 1;
  stats.allWins.push_back(result.totalWin);

  return stats;
}

int FlowGraphSimulator::ReportStats(const SimulationStatistics &) { return 1; }

SimulationStatistics FlowGraphSimulator::Run(int numSpins) {
  m_totalBet = 0;
  m_totalWin = 0;
  m_totalBaseWin = 0;
  m_totalScatterWin = 0;
  m_totalBonusWin = 0;
  m_hitCount = 0;
  m_totalSpins = 0;

  // Inject spins into the graph
  for (int i = 0; i < numSpins; ++i) {
    m_spin_node.try_put(i);
  }

  // Wait for all work to finish
  m_graph.wait_for_all();

  SimulationStatistics stats;
  stats.totalBet = m_totalBet.load();
  stats.totalWin = m_totalWin.load();
  stats.totalBaseWin = m_totalBaseWin.load();
  stats.totalScatterWin = m_totalScatterWin.load();
  stats.totalBonusWin = m_totalBonusWin.load();
  stats.hitCount = m_hitCount.load();
  stats.totalSpins = m_totalSpins.load();

  return stats;
}
