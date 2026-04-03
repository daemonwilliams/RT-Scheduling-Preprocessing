#include "../include/config.h"
#include "../include/algorithms.h"
#include "../include/schedule.h"
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

Trial runTrial(int n, double density, unsigned int seed) {
  Trial t;
  t.n = n;
  t.density = density;
  t.seed = seed;

  // Generate graph and tasks
  Graph g = Graph::generateRandomDAG(n, density, seed);
  Graph::generateRandomTasks(g, seed + 1, MAX_UTILIZATION);

  t.edges_original = g.numEdges();

  // Time matrix-based TR
  auto start = std::chrono::high_resolution_clock::now();
  Graph reduced_matrix = transitiveReductionMatrix(g);
  auto end = std::chrono::high_resolution_clock::now();
  t.tr_ns_matrix =
      std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  t.edges_reduced_matrix = reduced_matrix.numEdges();

  // Time DFS-based TR
  start = std::chrono::high_resolution_clock::now();
  Graph reduced_dfs = transitiveReductionDFS(g);
  end = std::chrono::high_resolution_clock::now();
  t.tr_ns_dfs =
      std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  t.edges_reduced_dfs = reduced_dfs.numEdges();

  // Use matrix-reduced graph for scheduling comparisons
  Graph &reduced = reduced_matrix;

  // Validation: verify TR correctness
  if (!verifyReduction(g, reduced_matrix))
    std::cerr << "WARNING: Matrix TR failed verification (seed=" << seed << ")"
              << std::endl;
  if (!verifyReduction(g, reduced_dfs))
    std::cerr << "WARNING: DFS TR failed verification (seed=" << seed << ")"
              << std::endl;
  if (reduced_matrix.numEdges() != reduced_dfs.numEdges())
    std::cerr << "WARNING: TR methods disagree on edge count (seed=" << seed
              << ")" << std::endl;

  // Helper to time a scheduler call with warmup and multiple iterations
  auto timeScheduler = [&](const Graph &graph, SchedulerType type) {
    // Warmup
    runScheduler(graph, type);

    const int iterations = SCHEDULER_TIMING_ITERATIONS;
    std::vector<long long> samples;
    samples.reserve(iterations);
    ScheduleResult last_r;

    for (int i = 0; i < iterations; ++i) {
      auto s = std::chrono::high_resolution_clock::now();
      last_r = runScheduler(graph, type);
      auto e = std::chrono::high_resolution_clock::now();
      long long dur =
          std::chrono::duration_cast<std::chrono::nanoseconds>(e - s).count();
      samples.push_back(dur);
    }
    std::sort(samples.begin(), samples.end());
    long long median_ns;
    if (samples.size() % 2 == 1) {
      median_ns = samples[samples.size() / 2];
    } else {
      long long lo = samples[samples.size() / 2 - 1];
      long long hi = samples[samples.size() / 2];
      median_ns = (lo + hi) / 2;
    }
    return std::make_pair(median_ns, last_r);
  };

  // Time EDF on original
  auto res = timeScheduler(g, EDF);
  t.edf_ns_original = res.first;
  t.edf_feasible_original = res.second.feasible;

  // Time EDF on reduced
  res = timeScheduler(reduced, EDF);
  t.edf_ns_reduced = res.first;
  t.edf_feasible_reduced = res.second.feasible;

  // Time LLF on original
  res = timeScheduler(g, LLF);
  t.llf_ns_original = res.first;
  t.llf_feasible_original = res.second.feasible;

  // Time LLF on reduced
  res = timeScheduler(reduced, LLF);
  t.llf_ns_reduced = res.first;
  t.llf_feasible_reduced = res.second.feasible;

  // Time RM on original
  res = timeScheduler(g, RM);
  t.rm_ns_original = res.first;
  t.rm_feasible_original = res.second.feasible;

  // Time RM on reduced
  res = timeScheduler(reduced, RM);
  t.rm_ns_reduced = res.first;
  t.rm_feasible_reduced = res.second.feasible;

  // Validation: feasibility should match between original and reduced
  if (t.edf_feasible_original != t.edf_feasible_reduced)
    std::cerr << "WARNING: EDF feasibility mismatch (seed=" << seed << ")"
              << std::endl;
  if (t.llf_feasible_original != t.llf_feasible_reduced)
    std::cerr << "WARNING: LLF feasibility mismatch (seed=" << seed << ")"
              << std::endl;
  if (t.rm_feasible_original != t.rm_feasible_reduced)
    std::cerr << "WARNING: RM feasibility mismatch (seed=" << seed << ")"
              << std::endl;

  return t;
}

void writeCSV(const std::vector<Trial> &trials, const char *filename) {
  std::ofstream out(filename);

  out << "n,density,seed,"
      << "edges_original,edges_reduced_matrix,edges_reduced_dfs,"
      << "tr_ns_matrix,tr_ns_dfs,"
      << "edf_ns_original,edf_ns_reduced,"
      << "llf_ns_original,llf_ns_reduced,"
      << "rm_ns_original,rm_ns_reduced,"
      << "edf_feasible_original,edf_feasible_reduced,"
      << "llf_feasible_original,llf_feasible_reduced,"
      << "rm_feasible_original,rm_feasible_reduced\n";

  for (const Trial &t : trials) {
    out << t.n << "," << t.density << "," << t.seed << "," << t.edges_original
        << "," << t.edges_reduced_matrix << "," << t.edges_reduced_dfs << ","
        << t.tr_ns_matrix << "," << t.tr_ns_dfs << "," << t.edf_ns_original
        << "," << t.edf_ns_reduced << "," << t.llf_ns_original << ","
        << t.llf_ns_reduced << "," << t.rm_ns_original << "," << t.rm_ns_reduced
        << "," << t.edf_feasible_original << "," << t.edf_feasible_reduced
        << "," << t.llf_feasible_original << "," << t.llf_feasible_reduced
        << "," << t.rm_feasible_original << "," << t.rm_feasible_reduced
        << "\n";
  }

  out.close();
}

void printSummary(const std::vector<Trial> &trials) {
  // ── Table 1: Transitive Reduction  (matrix vs DFS) ──
  {
    const char *top = "┌─────────┬─────────┬─────────┬─────────┬──────────────┬"
                      "──────────────┬────────┐";
    const char *sep = "─────────┬─────────┬─────────┬─────────┬──────────────┬"
                      "──────────────┬────────";
    const char *bot = "└─────────┴─────────┴─────────┴─────────┴──────────────┴"
                      "──────────────┴────────┘";

    std::cout << "\n  ══ Transitive Reduction: Matrix vs DFS ══\n" << std::endl;
    std::cout << top << std::endl;
    printf("│ %7s │ %7s │ %7s │ %7s │ %12s │ %12s │ %6s │\n", "tasks",
           "density", "edges", "reduced", "matrix(us)", "dfs(us)", "winner");
    std::cout << "├" << sep << "┤" << std::endl;

    for (int ni = 0; ni < NUM_TASK_COUNTS; ++ni) {
      for (int di = 0; di < NUM_DENSITIES; ++di) {
        double sum_edges = 0, sum_reduced = 0;
        double sum_tr_mat = 0, sum_tr_dfs = 0;
        int count = 0;

        for (const Trial &t : trials) {
          if (t.n == TASK_COUNTS[ni] && t.density == DENSITIES[di]) {
            sum_edges += t.edges_original;
            sum_reduced += t.edges_reduced_matrix;
            sum_tr_mat += t.tr_ns_matrix;
            sum_tr_dfs += t.tr_ns_dfs;
            ++count;
          }
        }
        if (count == 0)
          continue;

        double avg_edges = sum_edges / count;
        double avg_reduced = sum_reduced / count;
        double avg_mat = sum_tr_mat / count / 1000.0;
        double avg_dfs = sum_tr_dfs / count / 1000.0;
        const char *winner = (avg_mat <= avg_dfs) ? "matrix" : "   dfs";

        printf("│ %7d │ %7.1f │ %7.0f │ %7.0f │ %12.1f │ %12.1f │ %6s │\n",
               TASK_COUNTS[ni], DENSITIES[di], avg_edges, avg_reduced, avg_mat,
               avg_dfs, winner);
      }
      if (ni < NUM_TASK_COUNTS - 1)
        std::cout << "├" << sep << "┤" << std::endl;
    }

    std::cout << bot << std::endl;
    std::cout << "  Times are averaged over " << TRIALS_PER_COMBO
              << " trial(s) per combo.\n"
              << std::endl;
  }

  // ── Table 2: Scheduling after TR ──
  {
    const char *top = "┌─────────┬─────────┬──────────────┬──────────────┬─────"
                      "─────────┬──────────┬──────────┐";
    const char *sep =
        "─────────┬─────────┬──────────────┬──────────────┬───────"
        "───────┬──────────┬──────────";
    const char *bot = "└─────────┴─────────┴──────────────┴──────────────┴─────"
                      "─────────┴──────────┴──────────┘";

    std::cout << "  ══ Scheduling After Transitive Reduction ══\n" << std::endl;
    std::cout << top << std::endl;
    printf("│ %7s │ %7s │ %12s │ %12s │ %12s │ %8s │ %8s │\n", "tasks",
           "density", "EDF spd(us)", "LLF spd(us)", "RM spd(us)", "EDF feas",
           "RM feas");
    std::cout << "├" << sep << "┤" << std::endl;

    for (int ni = 0; ni < NUM_TASK_COUNTS; ++ni) {
      for (int di = 0; di < NUM_DENSITIES; ++di) {
        double sum_edf_spd = 0, sum_llf_spd = 0, sum_rm_spd = 0;
        int count = 0;
        int edf_llf_feasible = 0, rm_feasible = 0;

        for (const Trial &t : trials) {
          if (t.n == TASK_COUNTS[ni] && t.density == DENSITIES[di]) {
            sum_edf_spd += (t.edf_ns_original - t.edf_ns_reduced);
            sum_llf_spd += (t.llf_ns_original - t.llf_ns_reduced);
            sum_rm_spd += (t.rm_ns_original - t.rm_ns_reduced);
            if (t.edf_feasible_original || t.llf_feasible_original)
              ++edf_llf_feasible;
            if (t.rm_feasible_original)
              ++rm_feasible;
            ++count;
          }
        }
        if (count == 0)
          continue;

        double avg_edf = sum_edf_spd / count / 1000.0;
        double avg_llf = sum_llf_spd / count / 1000.0;
        double avg_rm = sum_rm_spd / count / 1000.0;
        double edf_llf_pct = 100.0 * edf_llf_feasible / count;
        double rm_pct = 100.0 * rm_feasible / count;

        printf(
            "│ %7d │ %7.1f │ %12.1f │ %12.1f │ %12.1f │ %7.0f%% │ %7.0f%% │\n",
            TASK_COUNTS[ni], DENSITIES[di], avg_edf, avg_llf, avg_rm,
            edf_llf_pct, rm_pct);
      }
      if (ni < NUM_TASK_COUNTS - 1)
        std::cout << "├" << sep << "┤" << std::endl;
    }

    std::cout << bot << std::endl;
    std::cout << "  Speedup = original_time - reduced_time (positive = TR "
                 "helped scheduling)"
              << std::endl;
    std::cout << "  EDF feas = % feasible under EDF or LLF  |  RM feas = % "
                 "feasible under RM\n"
              << std::endl;
  }

  // ── Table 3: TR Worth It? (min(matrix,dfs) vs each scheduler speedup) ──
  {
    const char *top =
        "┌─────────┬─────────┬──────────────┬──────────┬──────────┬──────────┐";
    const char *sep =
        "─────────┬─────────┬──────────────┬──────────┬──────────┬──────────";
    const char *bot =
        "└─────────┴─────────┴──────────────┴──────────┴──────────┴──────────┘";

    std::cout << "  ══ min(matrix,dfs) < Scheduler Speedup ══\n" << std::endl;
    std::cout << top << std::endl;
    printf("│ %7s │ %7s │ %12s │ %8s │ %8s │ %8s │\n", "tasks", "density",
           "min_TR(us)", "< EDF", "< LLF", "< RM");
    std::cout << "├" << sep << "┤" << std::endl;

    for (int ni = 0; ni < NUM_TASK_COUNTS; ++ni) {
      for (int di = 0; di < NUM_DENSITIES; ++di) {
        double sum_tr_mat = 0, sum_tr_dfs = 0;
        double sum_edf_spd = 0, sum_llf_spd = 0, sum_rm_spd = 0;
        int count = 0;

        for (const Trial &t : trials) {
          if (t.n == TASK_COUNTS[ni] && t.density == DENSITIES[di]) {
            sum_tr_mat += t.tr_ns_matrix;
            sum_tr_dfs += t.tr_ns_dfs;
            sum_edf_spd += (t.edf_ns_original - t.edf_ns_reduced);
            sum_llf_spd += (t.llf_ns_original - t.llf_ns_reduced);
            sum_rm_spd += (t.rm_ns_original - t.rm_ns_reduced);
            ++count;
          }
        }
        if (count == 0)
          continue;

        double avg_tr_mat = sum_tr_mat / count / 1000.0;
        double avg_tr_dfs = sum_tr_dfs / count / 1000.0;
        double best_tr = (avg_tr_mat < avg_tr_dfs) ? avg_tr_mat : avg_tr_dfs;
        double avg_edf = sum_edf_spd / count / 1000.0;
        double avg_llf = sum_llf_spd / count / 1000.0;
        double avg_rm = sum_rm_spd / count / 1000.0;

        const char *edf_ok = (best_tr < avg_edf) ? "  yes" : "   no";
        const char *llf_ok = (best_tr < avg_llf) ? "  yes" : "   no";
        const char *rm_ok = (best_tr < avg_rm) ? "  yes" : "   no";

        printf("│ %7d │ %7.1f │ %12.1f │ %8s │ %8s │ %8s │\n", TASK_COUNTS[ni],
               DENSITIES[di], best_tr, edf_ok, llf_ok, rm_ok);
      }
      if (ni < NUM_TASK_COUNTS - 1)
        std::cout << "├" << sep << "┤" << std::endl;
    }

    std::cout << bot << std::endl;
    std::cout << "  yes = min(matrix,dfs) < scheduler speedup\n" << std::endl;
  }
}

void writeSummaryCSV(const std::vector<Trial> &trials, const char *filename) {
  std::ofstream out(filename);

  out << "n,density,avg_edges_original,avg_edges_reduced,"
      << "avg_tr_us_matrix,avg_tr_us_dfs,"
      << "avg_edf_speedup_us,avg_llf_speedup_us,avg_rm_speedup_us,"
      << "tr_worth_it,tr_worth_edf,tr_worth_llf,tr_worth_rm\n";

  for (int ni = 0; ni < NUM_TASK_COUNTS; ++ni) {
    for (int di = 0; di < NUM_DENSITIES; ++di) {
      double sum_edges = 0, sum_reduced = 0;
      double sum_tr_mat = 0, sum_tr_dfs = 0;
      double sum_edf_spd = 0, sum_llf_spd = 0, sum_rm_spd = 0;
      int count = 0;

      for (const Trial &t : trials) {
        if (t.n == TASK_COUNTS[ni] && t.density == DENSITIES[di]) {
          sum_edges += t.edges_original;
          sum_reduced += t.edges_reduced_matrix;
          sum_tr_mat += t.tr_ns_matrix;
          sum_tr_dfs += t.tr_ns_dfs;
          sum_edf_spd += (t.edf_ns_original - t.edf_ns_reduced);
          sum_llf_spd += (t.llf_ns_original - t.llf_ns_reduced);
          sum_rm_spd += (t.rm_ns_original - t.rm_ns_reduced);
          ++count;
        }
      }

      if (count == 0)
        continue;

      double avg_edges = sum_edges / count;
      double avg_reduced = sum_reduced / count;
      double avg_tr_mat = sum_tr_mat / count / 1000.0;
      double avg_tr_dfs = sum_tr_dfs / count / 1000.0;
      double avg_edf_spd = sum_edf_spd / count / 1000.0;
      double avg_llf_spd = sum_llf_spd / count / 1000.0;
      double avg_rm_spd = sum_rm_spd / count / 1000.0;
      double best_tr = (avg_tr_mat < avg_tr_dfs) ? avg_tr_mat : avg_tr_dfs;
      double best_spd = (avg_edf_spd > avg_llf_spd) ? avg_edf_spd : avg_llf_spd;
      if (avg_rm_spd > best_spd)
        best_spd = avg_rm_spd;
      int worth = (best_spd > best_tr) ? 1 : 0;
      int worth_edf = (best_tr < avg_edf_spd) ? 1 : 0;
      int worth_llf = (best_tr < avg_llf_spd) ? 1 : 0;
      int worth_rm = (best_tr < avg_rm_spd) ? 1 : 0;

      out << TASK_COUNTS[ni] << "," << DENSITIES[di] << "," << avg_edges << ","
          << avg_reduced << "," << avg_tr_mat << "," << avg_tr_dfs << ","
          << avg_edf_spd << "," << avg_llf_spd << "," << avg_rm_spd << ","
          << worth << "," << worth_edf << "," << worth_llf << "," << worth_rm
          << "\n";
    }
  }

  out.close();
}

void runAllTrials() {
  int total = NUM_TASK_COUNTS * NUM_DENSITIES * TRIALS_PER_COMBO;
  int current = 0;
  std::vector<Trial> trials;
  trials.reserve(total);

  for (int ni = 0; ni < NUM_TASK_COUNTS; ++ni) {
    for (int di = 0; di < NUM_DENSITIES; ++di) {
      for (int ti = 0; ti < TRIALS_PER_COMBO; ++ti) {
        unsigned int seed = BASE_SEED + ni * 10000 + di * 100 + ti;
        ++current;

        std::cout << "Trial " << current << "/" << total
                  << ": n=" << TASK_COUNTS[ni] << ", d=" << DENSITIES[di]
                  << ", seed=" << seed << std::endl;

        Trial result = runTrial(TASK_COUNTS[ni], DENSITIES[di], seed);

        std::cout << "  Edges: " << result.edges_original << " -> "
                  << result.edges_reduced_matrix << " (matrix)"
                  << ", " << result.edges_reduced_dfs << " (dfs)" << std::endl;
        std::cout << "  TR time: matrix=" << result.tr_ns_matrix / 1000.0
                  << "μs"
                  << ", dfs=" << result.tr_ns_dfs / 1000.0 << "μs" << std::endl;
        std::cout << "  EDF: " << result.edf_ns_original / 1000.0 << "μs -> "
                  << result.edf_ns_reduced / 1000.0 << "μs"
                  << (result.edf_feasible_original ? " [feasible]"
                                                   : " [infeasible]")
                  << std::endl;
        std::cout << "  LLF: " << result.llf_ns_original / 1000.0 << "μs -> "
                  << result.llf_ns_reduced / 1000.0 << "μs"
                  << (result.llf_feasible_original ? " [feasible]"
                                                   : " [infeasible]")
                  << std::endl;
        std::cout << "  RM:  " << result.rm_ns_original / 1000.0 << "μs -> "
                  << result.rm_ns_reduced / 1000.0 << "μs"
                  << (result.rm_feasible_original ? " [feasible]"
                                                  : " [infeasible]")
                  << std::endl;

        trials.push_back(result);
      }
    }
  }

  writeCSV(trials, OUTPUT_FILE);
  printSummary(trials);
  writeSummaryCSV(trials, "summary.csv");
  std::cout << "Done. " << total << " trials written to " << OUTPUT_FILE
            << std::endl;
  std::cout << "Summary written to summary.csv" << std::endl;
}
