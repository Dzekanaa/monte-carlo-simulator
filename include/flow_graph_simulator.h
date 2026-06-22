#pragma once

#include "../slot-engine/include/slot_engine.h"
#include "statistics.h"
#include <tbb/flow_graph.h>
#include <tuple>

/**
 * @brief Monte Carlo simulator based on the TBB flow::graph execution model.
 *
 * FlowGraphSimulator executes slot machine simulations using a directed
 * task graph. The graph structure follows the specification:
 *
 *   broadcast_node<int> ──┬──► base_rtp_node ──► variance_node ──┐
 *                         ├──► bonus_rtp_node ──┐               │
 *                         └──► scatter_rtp_node ─┼──► join_node ─┼──►
 * freespin_impact_node ──┐ └────────────────────┘               │
 *                         │                                     │
 *   variance_node ────────┼─────────────────────────────────────┤
 *                         │                                     │
 *                         └────────► join_node ──► report_node
 *
 * Each node simulates an independent game component, while dependencies
 * are expressed through join nodes and directed edges.
 */
class FlowGraphSimulator {
public:
  /**
   * @brief Constructs a FlowGraphSimulator instance.
   *
   * Creates the flow::graph and initializes all nodes according to the
   * specified graph topology.
   *
   * @param config Slot game configuration used for simulation.
   * @param betPerSpin Bet amount applied to each spin.
   */
  FlowGraphSimulator(const SlotEngine::GameConfig &config, int betPerSpin);

  /**
   * @brief Runs the Monte Carlo simulation using the flow::graph pipeline.
   *
   * Sends the number of spins through the broadcast node, triggering all
   * simulation branches in parallel. The graph executes until completion
   * and the final results are reported through the report node.
   *
   * @param numSpins Number of spins to simulate.
   * @return Aggregated simulation statistics.
   */
  SimulationStatistics Run(int numSpins);

private:
  /// @brief Slot game configuration.
  SlotEngine::GameConfig m_config;

  /// @brief Bet amount per spin.
  int m_betPerSpin;

  /// @brief TBB flow graph managing the simulation pipeline.
  tbb::flow::graph m_graph;

  /// @brief Broadcast node that distributes the spin count to all branches.
  tbb::flow::broadcast_node<int> m_broadcast;

  /// @brief Node responsible for simulating the base game component.
  tbb::flow::function_node<int, SimulationStatistics> m_base_rtp_node;

  /// @brief Node responsible for simulating the bonus game component.
  tbb::flow::function_node<int, SimulationStatistics> m_bonus_rtp_node;

  /// @brief Node responsible for simulating the scatter component.
  tbb::flow::function_node<int, SimulationStatistics> m_scatter_rtp_node;

  /**
   * @brief Contains variance analysis results.
   */
  struct VarianceResult {
    SimulationStatistics stats; ///< Original simulation statistics.
    double variance;            ///< Variance of the win distribution.
    double stddev;              ///< Standard deviation of the win distribution.
  };

  /// @brief Node that computes variance statistics from base game results.
  tbb::flow::function_node<SimulationStatistics, VarianceResult>
      m_variance_node;

  /// @brief Join node combining bonus and scatter statistics.
  tbb::flow::join_node<std::tuple<SimulationStatistics, SimulationStatistics>>
      m_bonus_scatter_join;

  /**
   * @brief Contains free spin impact analysis results.
   */
  struct FreespinImpact {
    double impact; ///< Difference between bonus RTP and scatter RTP
                   ///< expressed in percentage points.
  };

  /// @brief Node that computes the RTP impact of free spins.
  tbb::flow::function_node<
      std::tuple<SimulationStatistics, SimulationStatistics>, FreespinImpact>
      m_freespin_impact_node;

  /// @brief Final join node combining variance and free spin impact results.
  tbb::flow::join_node<std::tuple<VarianceResult, FreespinImpact>> m_final_join;

  /// @brief Report node responsible for generating the final simulation report.
  tbb::flow::function_node<std::tuple<VarianceResult, FreespinImpact>, int>
      m_report_node;

  // ========================================================================
  // Component Simulation Methods
  // ========================================================================

  /**
   * @brief Simulates the base game component.
   *
   * Executes the specified number of spins and collects statistics related
   * to base game wins. All win values are stored for variance calculations.
   *
   * @param numSpins Number of spins to simulate.
   * @return Base game statistics.
   */
  SimulationStatistics SimulateBaseComponent(int numSpins);

  /**
   * @brief Simulates the bonus game component.
   *
   * Executes the specified number of spins and collects statistics related
   * to bonus game wins.
   *
   * @param numSpins Number of spins to simulate.
   * @return Bonus game statistics.
   */
  SimulationStatistics SimulateBonusComponent(int numSpins);

  /**
   * @brief Simulates the scatter component.
   *
   * Executes the specified number of spins and collects statistics related
   * to scatter wins.
   *
   * @param numSpins Number of spins to simulate.
   * @return Scatter statistics.
   */
  SimulationStatistics SimulateScatterComponent(int numSpins);

  // ========================================================================
  // Graph Processing Methods
  // ========================================================================

  /**
   * @brief Computes variance and standard deviation from base game statistics.
   *
   * Uses the allWins vector to calculate the variance of the win distribution.
   *
   * @param baseStats Base game statistics.
   * @return VarianceResult containing the original statistics and computed
   *         variance metrics.
   */
  VarianceResult ComputeVariance(const SimulationStatistics &baseStats);

  /**
   * @brief Computes the RTP impact of free spins.
   *
   * Calculates the difference between bonus RTP and scatter RTP.
   *
   * @param bonusScatter Tuple containing bonus and scatter statistics.
   * @return FreespinImpact containing the calculated RTP contribution.
   */
  FreespinImpact ComputeFreespinImpact(
      const std::tuple<SimulationStatistics, SimulationStatistics>
          &bonusScatter);

  /**
   * @brief Generates and prints the final simulation report.
   *
   * Displays RTP contributions by component, hit frequency, variance,
   * standard deviation, and free spin impact metrics.
   *
   * @param finalData Tuple containing VarianceResult and FreespinImpact.
   */
  void
  GenerateReport(const std::tuple<VarianceResult, FreespinImpact> &finalData);
};
