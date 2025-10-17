#ifndef ASTAR_H
#define ASTAR_H

#include "graph.h"
double astar(Graph* graph, int start_id, int goal_id, int** out_path, int* out_len);

#endif