#pragma once
#include "../include/graph.h"

// Matrix-based transitive reduction (Aho, Garey, Ullman)
Graph transitiveReductionMatrix(const Graph& g);

// DFS-based transitive reduction
Graph transitiveReductionDFS(const Graph& g);

// Verify reduced graph has same reachability as original
bool verifyReduction(const Graph& original, const Graph& reduced);
