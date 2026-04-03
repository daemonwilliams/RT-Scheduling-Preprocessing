#include "../include/algorithms.h"

Graph transitiveReductionMatrix(const Graph &g) {
  int n = g.numNodes();
  Graph result(n);
  result.getTasks() = g.getTasks();

  // Step 1: Copy adjacency matrix
  std::vector<std::vector<bool>> closure(n, std::vector<bool>(n, false));
  for (int i = 0; i < n; ++i)
    for (int j = 0; j < n; ++j)
      closure[i][j] = g.hasEdge(i, j);

  // Step 2: 3 For Loops for transitive closer (Floyd-Warshall)
  for (int k = 0; k < n; ++k)
    for (int i = 0; i < n; ++i)
      for (int j = 0; j < n; ++j)
        if (closure[i][k] && closure[k][j])
          closure[i][j] = true;

  // Step 3: Keep edge u→v only if no intermediate node k exists
  // where u→k is a direct edge and k can reach v
  for (int u = 0; u < n; ++u) {
    for (int v = 0; v < n; ++v) {
      if (!g.hasEdge(u, v))
        continue;

      bool redundant = false;
      for (int k = 0; k < n; ++k) {
        if (k == u || k == v)
          continue;
        if (g.hasEdge(u, k) && closure[k][v]) {
          redundant = true;
          break;
        }
      }
      if (!redundant)
        result.addEdge(u, v);
    }
  }

  return result;
}

// DFS helper: marks all nodes reachable from src
static void dfs(const Graph &g, int src, std::vector<bool> &visited) {
  visited[src] = true;
  for (int next : g.getAdjList()[src]) {
    if (!visited[next])
      dfs(g, next, visited);
  }
}

Graph transitiveReductionDFS(const Graph &g) {
  int n = g.numNodes();
  Graph result(n);
  result.getTasks() = g.getTasks();

  // Step 1: Copy all edges into result
  for (int u = 0; u < n; ++u)
    for (int v : g.getAdjList()[u])
      result.addEdge(u, v);

  // Step 2: For each edge u→v, check if v is reachable
  // from any other successor w of u
  for (int u = 0; u < n; ++u) {
    std::vector<int> successors = g.getAdjList()[u];

    for (int v : successors) {
      if (!result.hasEdge(u, v))
        continue;

      for (int w : successors) {
        if (w == v)
          continue;

        std::vector<bool> visited(n, false);
        dfs(g, w, visited);

        if (visited[v]) {
          result.removeEdge(u, v);
          break;
        }
      }
    }
  }

  return result;
}

bool verifyReduction(const Graph &original, const Graph &reduced) {
  int n = original.numNodes();
  if (reduced.numNodes() != n)
    return false;

  for (int src = 0; src < n; ++src) {
    std::vector<bool> reachOrig(n, false);
    std::vector<bool> reachRed(n, false);
    dfs(original, src, reachOrig);
    dfs(reduced, src, reachRed);

    for (int i = 0; i < n; ++i) {
      if (reachOrig[i] != reachRed[i])
        return false;
    }
  }

  return true;
}
