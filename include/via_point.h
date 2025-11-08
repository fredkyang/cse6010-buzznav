#ifndef VIA_POINT_H
#define VIA_POINT_H
#include "graph.h"

double astar_via_points(Graph* graph, int start_id, int goal_id,int* via_ids,
    int num_via,int** out_path,int* out_len);
  // via n point   n can be 0,1,2 or others
#endif