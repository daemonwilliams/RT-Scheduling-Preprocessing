#include "../include/schedule.h"
#include <climits>
#include <numeric>

int computeLCM(const Graph &g) {
  int hp = 1;
  for (int i = 0; i < g.numNodes(); ++i)
    hp = std::lcm(hp, g.getTasks()[i].period);
  return hp;
}

static std::vector<std::vector<int>> buildPredecessors(const Graph &g) {
  int n = g.numNodes();
  std::vector<std::vector<int>> preds(n);
  for (int u = 0; u < n; ++u)
    for (int v : g.getAdjList()[u])
      preds[v].push_back(u);
  return preds;
}

ScheduleResult runScheduler(const Graph &g, SchedulerType type) {
  int n = g.numNodes();
  int hp = computeLCM(g);
  const auto &tasks = g.getTasks();
  auto preds = buildPredecessors(g);

  std::vector<Job> currentJob(n);
  std::vector<bool> jobActive(n, false);
  std::vector<int> timeToNextRelease(n, 0);
  int deadline_misses = 0;

  for (int t = 0; t < hp; ++t) {
    // Release new jobs
    for (int i = 0; i < n; ++i) {
      if (timeToNextRelease[i] == 0) {
        if (jobActive[i] && !currentJob[i].completed &&
            currentJob[i].remaining > 0)
          ++deadline_misses;
        currentJob[i] = {i, t, t + tasks[i].deadline, tasks[i].computation_time,
                         false};
        jobActive[i] = true;
        timeToNextRelease[i] = tasks[i].period;
      }
      timeToNextRelease[i]--;
    }

    // Check deadlines
    for (int i = 0; i < n; ++i) {
      if (jobActive[i] && !currentJob[i].completed &&
          t >= currentJob[i].absolute_deadline && currentJob[i].remaining > 0) {
        ++deadline_misses;
        currentJob[i].completed = true;
      }
    }

    int best = -1;
    int bestPriority = INT_MAX;

    // Find best job to run
    for (int i = 0; i < n; ++i) {
      if (!jobActive[i] || currentJob[i].completed ||
          currentJob[i].remaining <= 0)
        continue;
      // Precedence check - this is the part TR helps
      bool ready = true;
      for (int p : preds[i]) {
        if (jobActive[p] && !currentJob[p].completed) {
          ready = false;
          break;
        }
      }
      if (!ready)
        continue;

      int priority;
      switch (type) {
      case EDF:
        priority = currentJob[i].absolute_deadline;
        break;
      case LLF:
        priority =
            currentJob[i].absolute_deadline - t - currentJob[i].remaining;
        break;
      case RM:
        priority = tasks[i].period;
        break;
      default:
        priority = INT_MAX;
      }

      if (priority < bestPriority) {
        bestPriority = priority;
        best = i;
      }
    }

    if (best >= 0) {
      currentJob[best].remaining--;
      if (currentJob[best].remaining == 0)
        currentJob[best].completed = true;
    }
  }

  for (int i = 0; i < n; ++i) {
    if (jobActive[i] && !currentJob[i].completed && currentJob[i].remaining > 0)
      ++deadline_misses;
  }

  return {deadline_misses == 0, deadline_misses, hp};
}
