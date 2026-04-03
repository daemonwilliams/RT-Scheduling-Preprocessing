#include "../include/graph.h"

Graph::Graph(int n)
    : n_(n), matrix_(n, std::vector<bool>(n, false)), adjList_(n), tasks_(n) {}

void Graph::addEdge(int u, int v) {
  if (!matrix_[u][v]) {
    matrix_[u][v] = true;
    adjList_[u].push_back(v);
  }
}

void Graph::removeEdge(int u, int v) {
  if (matrix_[u][v]) {
    matrix_[u][v] = false;
    auto &list = adjList_[u];
    list.erase(std::find(list.begin(), list.end(), v));
  }
}

bool Graph::hasEdge(int u, int v) const { return matrix_[u][v]; }

int Graph::numNodes() const { return n_; }

int Graph::numEdges() const {
  int count = 0;
  for (int u = 0; u < n_; ++u)
    count += static_cast<int>(adjList_[u].size());
  return count;
}

const std::vector<std::vector<bool>> &Graph::getMatrix() const {
  return matrix_;
}
const std::vector<std::vector<int>> &Graph::getAdjList() const {
  return adjList_;
}
const std::vector<TaskNode> &Graph::getTasks() const { return tasks_; }
std::vector<TaskNode> &Graph::getTasks() { return tasks_; }

void Graph::rebuildAdjList() {
  for (int u = 0; u < n_; ++u) {
    adjList_[u].clear();
    for (int v = 0; v < n_; ++v) {
      if (matrix_[u][v])
        adjList_[u].push_back(v);
    }
  }
}

Graph Graph::generateRandomDAG(int n, double density, unsigned int seed) {
  Graph g(n);
  srand(seed);

  // Create a random topological ordering via Fisher-Yates shuffle
  std::vector<int> order(n);
  std::iota(order.begin(), order.end(), 0);
  for (int i = n - 1; i > 0; --i) {
    int j = rand() % (i + 1);
    std::swap(order[i], order[j]);
  }

  // Edge from order[i] to order[j] valid only if i < j (ensures acyclicity)
  for (int i = 0; i < n; ++i) {
    for (int j = i + 1; j < n; ++j) {
      double r = static_cast<double>(rand()) / RAND_MAX;
      if (r < density) {
        g.addEdge(order[i], order[j]);
      }
    }
  }

  return g;
}

void Graph::generateRandomTasks(Graph &g, unsigned int seed, double maxUtil) {
  srand(seed);
  const int period_choices[] = {50, 100, 200, 500, 1000};
  const int num_periods = 5;

  for (int i = 0; i < g.numNodes(); ++i) {
    TaskNode &t = g.getTasks()[i];
    t.period = period_choices[rand() % num_periods];
    t.computation_time = 1 + rand() % 10;
    if (t.computation_time > t.period)
      t.computation_time = t.period;
    t.deadline = t.period;
  }

  // Compute total utilization and scale down if needed
  double totalUtil = 0.0;
  for (int i = 0; i < g.numNodes(); ++i) {
    TaskNode &t = g.getTasks()[i];
    totalUtil += static_cast<double>(t.computation_time) / t.period;
  }

  if (totalUtil > maxUtil) {
    double scale = maxUtil / totalUtil;
    for (int i = 0; i < g.numNodes(); ++i) {
      TaskNode &t = g.getTasks()[i];
      t.computation_time = static_cast<int>(t.computation_time * scale);
      if (t.computation_time < 1)
        t.computation_time = 1;
    }
  }
}
