#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include "graph.h"
#include "utils.h"
#include "astar.h"

/**
 * A* Node in the priority queue
 */
typedef struct {
    int node_id;
    double f_score;
} PQNode;

/**
 * Simple min-heap priority queue
 */
typedef struct {
    PQNode* data;
    int size;
    int capacity;
} PriorityQueue;

static PriorityQueue* create_pq(int capacity) {
    PriorityQueue* pq = malloc(sizeof(PriorityQueue));
    pq->data = malloc(sizeof(PQNode) * capacity);
    pq->size = 0;
    pq->capacity = capacity;
    return pq;
}

static void free_pq(PriorityQueue* pq) {
    if (!pq) return;
    free(pq->data);
    free(pq);
}

static void pq_swap(PQNode* a, PQNode* b) {
    PQNode tmp = *a; *a = *b; *b = tmp;
}

static void pq_push(PriorityQueue* pq, int node_id, double f_score) {
    if (pq->size >= pq->capacity) return;
    pq->data[pq->size].node_id = node_id;
    pq->data[pq->size].f_score = f_score;
    int i = pq->size++;
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (pq->data[parent].f_score <= pq->data[i].f_score) break;
        pq_swap(&pq->data[parent], &pq->data[i]);
        i = parent;
    }
}

static int pq_pop(PriorityQueue* pq) {
    if (pq->size == 0) return -1;
    int node_id = pq->data[0].node_id;
    pq->data[0] = pq->data[--pq->size];
    int i = 0;
    while (1) {
        int l = 2*i + 1, r = 2*i + 2, smallest = i;
        if (l < pq->size && pq->data[l].f_score < pq->data[smallest].f_score) smallest = l;
        if (r < pq->size && pq->data[r].f_score < pq->data[smallest].f_score) smallest = r;
        if (smallest == i) break;
        pq_swap(&pq->data[i], &pq->data[smallest]);
        i = smallest;
    }
    return node_id;
}

static int pq_empty(PriorityQueue* pq) {
    return pq->size == 0;
}

/**
 * Reconstruct path by backtracking
 */
static int* reconstruct_path(int* came_from, int start, int goal, int* out_length) {
    int* path = malloc(sizeof(int) * 1000);
    int count = 0;
    for (int curr = goal; curr != -1; curr = came_from[curr])
        path[count++] = curr;

    // reverse path
    for (int i = 0; i < count / 2; i++) {
        int tmp = path[i];
        path[i] = path[count - i - 1];
        path[count - i - 1] = tmp;
    }

    *out_length = count;
    return path;
}

/**
 * Core A* Algorithm
 * 
 * @param graph      Pointer to graph
 * @param start_id   Start node ID
 * @param goal_id    Goal node ID
 * @param out_path   (Output) Array of node IDs in shortest path
 * @param out_len    (Output) Number of nodes in path
 * @return total distance of shortest path (or DBL_MAX if no path)
 */
double astar(Graph* graph, int start_id, int goal_id, int** out_path, int* out_len) {
    int n = graph->num_nodes;
    double* g_score = malloc(sizeof(double) * n);
    double* f_score = malloc(sizeof(double) * n);
    int* came_from = malloc(sizeof(int) * n);
    int* visited = calloc(n, sizeof(int));

    for (int i = 0; i < n; i++) {
        g_score[i] = DBL_MAX;
        f_score[i] = DBL_MAX;
        came_from[i] = -1;
    }

    g_score[start_id] = 0.0;
    f_score[start_id] = haversine(
        graph->nodes[start_id].lat, graph->nodes[start_id].lon,
        graph->nodes[goal_id].lat, graph->nodes[goal_id].lon
    );

    PriorityQueue* open_set = create_pq(n);
    pq_push(open_set, start_id, f_score[start_id]);

    while (!pq_empty(open_set)) {
        int current = pq_pop(open_set);
        if (current == goal_id) {
            *out_path = reconstruct_path(came_from, start_id, goal_id, out_len);
            double total_dist = g_score[goal_id];
            free(g_score); free(f_score); free(came_from); free(visited); free_pq(open_set);
            return total_dist;
        }

        if (visited[current]) continue;
        visited[current] = 1;

        for (Edge* e = graph->nodes[current].neighbors; e; e = e->next) {
            int neighbor = e->to;
            double tentative_g = g_score[current] + e->weight;
            if (tentative_g < g_score[neighbor]) {
                came_from[neighbor] = current;
                g_score[neighbor] = tentative_g;
                f_score[neighbor] = tentative_g + haversine(
                    graph->nodes[neighbor].lat, graph->nodes[neighbor].lon,
                    graph->nodes[goal_id].lat, graph->nodes[goal_id].lon
                );
                pq_push(open_set, neighbor, f_score[neighbor]);
            }
        }
    }

    // If no path found
    free(g_score); free(f_score); free(came_from); free(visited); free_pq(open_set);
    *out_path = NULL;
    *out_len = 0;
    return DBL_MAX;
}
