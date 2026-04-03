#pragma once
#include "../include/graph.h"
#include <vector>

// ---- Easy-to-edit constants ----

// Number of tasks (nodes) in each generated graph
const int TASK_COUNTS[] = {50, 100, 200, 500, 1000};
const int NUM_TASK_COUNTS = 5;

// Edge density: ratio of edges to max possible edges (0.0 = none, 1.0 = fully
// connected)
const double DENSITIES[] = {0.10, 0.20, 0.30, 0.40, 0.50, 0.60, 0.70};
const int NUM_DENSITIES = 7;

// How many random graphs to generate per (n, density) combination
const int TRIALS_PER_COMBO = 20;

// Scheduler timing iterations (median reported)
const int SCHEDULER_TIMING_ITERATIONS = 5;

// Starting seed for random generation
const unsigned int BASE_SEED = 12345;

const char OUTPUT_FILE[] = "results.csv";

// Maximum total CPU utilization (sum of C_i/T_i) for generated task sets
// Tasks are scaled down if they exceed this. Must be <= 1.0.
const double MAX_UTILIZATION = 0.4;

// -------------------------------------------

struct Trial {
  int n;
  double density;
  unsigned int seed;

  int edges_original;
  int edges_reduced_matrix;
  int edges_reduced_dfs;

  long long tr_ns_matrix;
  long long tr_ns_dfs;

  long long edf_ns_original;
  long long edf_ns_reduced;
  long long llf_ns_original;
  long long llf_ns_reduced;
  long long rm_ns_original;
  long long rm_ns_reduced;

  bool edf_feasible_original;
  bool edf_feasible_reduced;
  bool llf_feasible_original;
  bool llf_feasible_reduced;
  bool rm_feasible_original;
  bool rm_feasible_reduced;
};

Trial runTrial(int n, double density, unsigned int seed);
void writeCSV(const std::vector<Trial> &trials, const char *filename);
void printSummary(const std::vector<Trial> &trials);
void writeSummaryCSV(const std::vector<Trial> &trials, const char *filename);
void runAllTrials();
