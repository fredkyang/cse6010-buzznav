#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <omp.h>
#include "via_point.h"
#include "astar.h"

// color 
#define color_direct   "\033[34m"  
#define color_parallel "\033[33m"  
#define color_success  "\033[32m"  
#define color_error    "\033[31m"  
#define color_reset    "\033[0m"   
/*
* split into num_via+1 segments
* compute each segment in parallel using A* algorithm
* merge all segments into a complete path
*/

/*
* parallelization
* each segment is independment can be computed sametimes
* speedup, about *num_via+1 (in ideal condition)
*/

double astar_via_points(Graph* graph,int start_id,int goal_id,
int*via_ids,int num_via,int**out_path, int* out_len)
{
    // no via points use direct A*
    if (num_via == 0) {
        printf(color_direct"direct route \n" color_reset);
        return astar(graph, start_id, goal_id, out_path, out_len);
    }

    int num_segments = num_via + 1;
    int** segment_paths = calloc(num_segments, sizeof(int*));
    int* segment_lens = calloc(num_segments, sizeof(int));
    double* segment_dists = calloc(num_segments, sizeof(double));
    if (!segment_paths || !segment_lens || !segment_dists) {
        printf(color_error"memory allocation failed\n" color_reset);
        free(segment_paths);
        free(segment_lens);
        free(segment_dists);
        *out_len = 0;
        *out_path = NULL;
        return DBL_MAX;
    }

    for (int i = 0; i < num_segments; i++) {
        segment_dists[i] = DBL_MAX;
    }

    #pragma omp parallel for num_threads(num_segments)
    for (int i = 0; i < num_segments; i++) {
        int from_id = (i == 0) ? start_id : via_ids[i - 1];
        int to_id = (i == num_segments - 1) ? goal_id : via_ids[i];

        int thread_id = omp_get_thread_num();
        printf(color_parallel"[Thread %d] Segment %d: %d->%d\n" color_reset,
               thread_id, i, from_id, to_id);

        double dist = astar(graph, from_id, to_id, &segment_paths[i], &segment_lens[i]);
        segment_dists[i] = dist;

        if (dist != DBL_MAX && segment_paths[i]) {
            printf(color_success"[Thread %d] ok Segment %d complete: %.2f m\n" color_reset,
                   thread_id, i, dist);
        } else {
            printf(color_error"[Thread %d] sad Segment %d failed: No path\n" color_reset,
                   thread_id, i);
        }
    }

    printf(color_parallel"parallel finished! please check \n" color_reset);

    int all_success = 1;
    double total_distance = 0.0;
    for (int i = 0; i < num_segments; i++) {
        if (segment_dists[i] == DBL_MAX || segment_paths[i] == NULL) {
            printf(color_error"segment %d failed\n" color_reset, i);
            all_success = 0;
            break;
        }
        total_distance += segment_dists[i];
    }

    if (!all_success) {
        printf(color_error"clean \n" color_reset);
        for (int i = 0; i < num_segments; i++) {
            free(segment_paths[i]);
        }
        free(segment_dists);
        free(segment_lens);
        free(segment_paths);
        *out_len = 0;
        *out_path = NULL;
        return DBL_MAX;
    }

    printf(color_success" merge segment\n" color_reset);
    int total_len = 0;
    for (int i = 0; i < num_segments; i++) {
        total_len += segment_lens[i];
        if (i > 0) {
            total_len--;
        }
    }

    int* merged_path = malloc(sizeof(int) * total_len);
    if (!merged_path) {
        printf(color_error"memory allocation failed when merging\n" color_reset);
        for (int i = 0; i < num_segments; i++) {
            free(segment_paths[i]);
        }
        free(segment_dists);
        free(segment_lens);
        free(segment_paths);
        *out_len = 0;
        *out_path = NULL;
        return DBL_MAX;
    }

    int merge_index = 0;
    for (int i = 0; i < num_segments; i++) {
        int start_copy = (i == 0) ? 0 : 1;
        for (int k = start_copy; k < segment_lens[i]; k++) {
            merged_path[merge_index++] = segment_paths[i][k];
        }
        free(segment_paths[i]);
    }

    free(segment_dists);
    free(segment_lens);
    free(segment_paths);

    *out_len = total_len;
    *out_path = merged_path;

    printf(color_success"total distance:%.2f \n" color_reset, total_distance);
    printf(color_success"total nodes %d \n" color_reset, total_len);
    return total_distance;
}
