#ifndef TSP_H
#define TSP_H

#include "graph.h"

/**
 * Find optimal route to visit multiple buildings using Held-Karp algorithm
 * Uses Dijkstra for efficient pairwise distance computation (parallelized with OpenMP)
 * 
 * @param graph         Pointer to campus graph
 * @param mapping       Building name to node ID mapping
 * @param names         Array of building names to visit
 * @param N             Number of buildings to visit
 * @param out_dist      (Output) Total distance of optimal route
 * @param out_path      (Output) Complete path with all intermediate nodes
 * @param out_len       (Output) Length of the complete path
 * @return 0 on success, -1 on error
 */
int find_optimal_multi_building_route(
    Graph* graph,
    BuildingMapping* mapping,
    const char** names,
    int N,
    double* out_dist,
    int** out_path,
    int* out_len
);

#endif // TSP_H