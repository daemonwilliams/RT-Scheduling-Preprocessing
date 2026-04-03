# Transitive Reduction for Real-Time Scheduling

This project investigates using transitive reduction as a preprocessing step to improve the runtime efficiency of real-time task scheduling algorithms. 
By removing redundant edges from task precedence graphs, the scheduler performs fewer dependency checks per time step, resulting in faster scheduling.

This work is to be presented at the **IEEE SusTech 2026 Student Poster Contest**.

**Extended Abstract:** *Using Transitive Reduction of Task Precedence Graphs to Improve Efficiency of Real-Time Scheduling Algorithms*
**Poster:** *Using Transitive Reduction of Task Precedence Graphs to Improve Efficiency of Real-Time Scheduling Algorithms*
**Authors:** Daemon Williams, Dr. Stefan Andrei (Advisor) — Cleveland State University

## Overview

Real-time task schedulers with precedence constraints are commonly modeled as directed acyclic graphs (DAGs). 
Redundant edges in these graphs inflate scheduling runtime by increasing the number of dependency checks. 
This project applies transitive reduction to eliminate those redundant edges and benchmarks the impact on three standard real-time scheduling algorithms:

- **Earliest Deadline First (EDF)**
- **Least Laxity First (LLF)**
- **Rate Monotonic (RM)**

This program also tests Matrix-based TR (Aho et al.) vs DFS-based TR
### Key Findings

- Transitive reduction consistently speeds up scheduling by **28-58%**, scaling with graph density
- The preprocessing cost is amortized after **107-205 LCM iterations**, depending on density
- Results are consistent across all three scheduling algorithms

## Project Structure

```
├── main.cpp           # Entry point — runs all trials
├── config.h           # Experiment parameters (task counts, densities, trials)
├── config.cpp         # Trial runner, CSV output, summary tables
├── graph.h            # Graph class (adjacency matrix + list, task generation)
├── graph.cpp          # DAG generation, random task assignment
├── algorithms.h       # TR function declarations
├── algorithms.cpp     # Matrix-based TR (Aho et al.) and DFS-based TR
├── schedule.h         # Scheduler declarations
└── schedule.cpp       # EDF, LLF, RM scheduler with precedence enforcement
```

## Build & Run

```bash
g++ -O2 -Wall -std=c++17 -o experiment main.cpp config.cpp graph.cpp algorithms.cpp schedule.cpp
./experiment
```
This produces:
- `results.csv` — raw trial data (one row per trial)
- `summary.csv` — aggregated averages per (n, density) combination

## Experiment Configuration

Parameters can be adjusted in `config.h`:

| Parameter | Default | Description |
|-----------|---------|-------------|
| `TASK_COUNTS` | {50, 100, 200, 500, 1000} | Number of nodes per generated DAG |
| `DENSITIES` | {0.10, 0.20, ..., 0.70} | Edge density |
| `TRIALS_PER_COMBO` | 20 | Random graphs generated per n and density |
| `MAX_UTILIZATION` | 0.4 | CPU utilization cap for generated task sets |
| `BASE_SEED` | 12345 | Starting seed for reproducibility |

## Methodology

1. **Graph Generation** — Random DAGs are created, with edges added probabilistically based on the target density
2. **Task Assignment** — Each node receives random periodic task parameters (WCET, period, deadline) scaled to stay within the utilization bound
3. **Transitive Reduction** — Two implementations are benchmarked:
   - *Matrix-based* 
   - *DFS-based* 
4. **Scheduling** — EDF, LLF, and RM are run on both the original and reduced graphs over one LCM hyperperiod, with median runtime reported over 5 iterations
5. **Verification** — Reachability is compared between original and reduced graphs to confirm correctness

## CSV Output Format

`results.csv` columns:

| Column | Description |
|--------|-------------|
| `n`, `density`, `seed` | Trial parameters |
| `edges_original`, `edges_reduced_*` | Edge counts before/after TR |
| `tr_ns_matrix`, `tr_ns_dfs` | TR preprocessing time (nanoseconds) |
| `edf_ns_original`, `edf_ns_reduced` | EDF scheduling time before/after TR |
| `llf_ns_*`, `rm_ns_*` | Same for LLF and RM |
| `*_feasible_*` | Whether the schedule met all deadlines |




