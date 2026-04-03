#pragma once
#include "../include/graph.h"
#include <vector>

enum SchedulerType { EDF, LLF, RM };

struct ScheduleResult {
  bool feasible;
  int deadline_misses;
  int total_time_steps;
};

struct Job {
  int task_id;
  int release_time;
  int absolute_deadline;
  int remaining;
  bool completed;
};

ScheduleResult runScheduler(const Graph& g, SchedulerType type);

int computeLCM(const Graph& g);
