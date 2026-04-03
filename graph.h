#pragma once
#include <vector>
#include <algorithm>
#include <numeric>
#include <cstdlib>

// Represents a single periodic real-time task with its timing parameters
struct TaskNode {
  int computation_time;  // worst-case execution time (WCET) per period
  int period;            // how often this task releases a new job
  int deadline;          // relative deadline (set equal to period = implicit deadline)
};

// DAG (directed acyclic graph) representing task precedence constraints.
// Stores edges in both an adjacency matrix (for O(1) lookups) and
// an adjacency list (for efficient iteration over successors).
class Graph {
public:
  Graph() = default;
  explicit Graph(int n);

  void addEdge(int u, int v);    // add directed edge u → v
  void removeEdge(int u, int v); // remove directed edge u → v
  bool hasEdge(int u, int v) const;

  int numNodes() const;
  int numEdges() const;

  // Accessors for the two representations and task data
  const std::vector<std::vector<bool>>& getMatrix() const;   // matrix[u][v] = true if edge u→v
  const std::vector<std::vector<int>>& getAdjList() const;    // adjList[u] = {list of v where u→v}
  const std::vector<TaskNode>& getTasks() const;
  std::vector<TaskNode>& getTasks();

  // Generate a random DAG with n nodes and given edge density (0.0–1.0)
  static Graph generateRandomDAG(int n, double density, unsigned int seed);

  // Assign random task parameters; uses seed+1 to keep independent of graph structure
  static void generateRandomTasks(Graph& g, unsigned int seed, double maxUtil = 1.0);

private:
  int n_ = 0;
  std::vector<std::vector<bool>> matrix_;  // adjacency matrix (n×n)
  std::vector<std::vector<int>> adjList_;  // adjacency list (per-node successor lists)
  std::vector<TaskNode> tasks_;            // one TaskNode per graph node

  void rebuildAdjList();  // reconstruct adjList_ from matrix_ after bulk changes
};
