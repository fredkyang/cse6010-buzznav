#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <string.h>
#include <omp.h>
#include "graph.h"
#include "tsp.h"

// Color definitions
#define color_direct   "\033[34m"  
#define color_parallel "\033[33m"  
#define color_success  "\033[32m"  
#define color_error    "\033[31m"  
#define color_reset    "\033[0m"

/******************************************************
 * Priority Queue for Dijkstra
 ******************************************************/
typedef struct {
    int node_id;
    double distance;
} PQNode;

typedef struct {
    PQNode* data;
    int size;
    int cap;
} PQ;

static PQ* pq_create(int cap) {
    PQ* pq = malloc(sizeof(PQ));
    pq->data = malloc(sizeof(PQNode) * cap);
    pq->size = 0;
    pq->cap = cap;
    return pq;
}

static void pq_free(PQ* pq) {
    free(pq->data);
    free(pq);
}

static void pq_swap(PQNode* a, PQNode* b) {
    PQNode t = *a; *a = *b; *b = t;
}

static void pq_push(PQ* pq, int id, double dist) {
    if (pq->size >= pq->cap) return;
    pq->data[pq->size].node_id = id;
    pq->data[pq->size].distance = dist;
    int i = pq->size++;
    while (i > 0) {
        int p = (i - 1) / 2;
        if (pq->data[p].distance <= pq->data[i].distance) break;
        pq_swap(&pq->data[p], &pq->data[i]);
        i = p;
    }
}

static int pq_empty(PQ* pq) { return pq->size == 0; }

static int pq_pop(PQ* pq) {
    if (pq->size == 0) return -1;
    int id = pq->data[0].node_id;
    pq->data[0] = pq->data[--pq->size];

    int i = 0;
    for (;;) {
        int l = 2*i + 1, r = 2*i + 2, s = i;
        if (l < pq->size && pq->data[l].distance < pq->data[s].distance) s = l;
        if (r < pq->size && pq->data[r].distance < pq->data[s].distance) s = r;
        if (s == i) break;
        pq_swap(&pq->data[i], &pq->data[s]);
        i = s;
    }
    return id;
}

/******************************************************
 * Dijkstra
 ******************************************************/
static void dijkstra(Graph* g, int src, double* dist, int* prev) {
    int n = g->num_nodes;
    int* vis = calloc(n, sizeof(int));

    for (int i = 0; i < n; i++) {
        dist[i] = DBL_MAX;
        prev[i] = -1;
    }
    dist[src] = 0;

    PQ* pq = pq_create(n);
    pq_push(pq, src, 0);

    while (!pq_empty(pq)) {
        int u = pq_pop(pq);
        if (vis[u]) continue;
        vis[u] = 1;

        for (Edge* e = g->nodes[u].neighbors; e; e = e->next) {
            int v = e->to;
            double nd = dist[u] + e->weight;
            if (nd < dist[v]) {
                dist[v] = nd;
                prev[v] = u;
                pq_push(pq, v, nd);
            }
        }
    }

    free(vis);
    pq_free(pq);
}

/******************************************************
 * Reconstruct path
 ******************************************************/
static int* build_path(int* prev, int src, int dst, int* out_len) {
    if (src != dst && prev[dst] == -1) {
        *out_len = 0;
        return NULL;
    }

    int len = 0;
    for (int cur = dst; cur != -1; cur = prev[cur]) {
        len++;
        if (cur == src) break;
    }

    int* p = malloc(sizeof(int) * len);
    int idx = len - 1;
    for (int cur = dst; cur != -1; cur = prev[cur]) {
        p[idx--] = cur;
        if (cur == src) break;
    }

    *out_len = len;
    return p;
}

/******************************************************
 * Multi-building TSP (Held-Karp)
 ******************************************************/
int find_optimal_multi_building_route(
    Graph* graph,
    BuildingMapping* mapping,
    const char** names,
    int N,
    double* out_dist,
    int** out_path,
    int* out_len
) {
    int* ids = malloc(sizeof(int) * N);
    for (int i = 0; i < N; i++) {
        ids[i] = get_building_id(mapping, names[i]);
        if (ids[i] < 0) {
            fprintf(stderr, "Building not found: %s\n", names[i]);
            free(ids);
            return -1;
        }
    }

    printf(color_parallel"Computing pairwise distances using Dijkstra (parallel with OpenMP)...\n"color_reset);

    double** dist = malloc(sizeof(double*) * N);
    int*** segs = malloc(sizeof(int**) * N);
    int** seglen = malloc(sizeof(int*) * N);

    // Parallelize Dijkstra computation
    double start_time = omp_get_wtime();
    
    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < N; i++) {
        // Each thread needs independent temporary arrays
        double* d_local = malloc(sizeof(double) * graph->num_nodes);
        int* prev_local = malloc(sizeof(int) * graph->num_nodes);
        
        dist[i] = malloc(sizeof(double) * N);
        segs[i] = malloc(sizeof(int*) * N);
        seglen[i] = malloc(sizeof(int) * N);

        // Show computation progress
        int thread_id = omp_get_thread_num();
        printf(color_parallel"[Thread %d] Computing distances from building %d (%s)\n"color_reset,
               thread_id, i, names[i]);

        dijkstra(graph, ids[i], d_local, prev_local);

        for (int j = 0; j < N; j++) {
            if (i == j) {
                dist[i][j] = 0;
                segs[i][j] = NULL;
                seglen[i][j] = 0;
                continue;
            }

            if (d_local[ids[j]] == DBL_MAX) {
                dist[i][j] = 1e15;  
                segs[i][j] = NULL;
                seglen[i][j] = 0;
            } else {
                dist[i][j] = d_local[ids[j]];
                segs[i][j] = build_path(prev_local, ids[i], ids[j], &seglen[i][j]);
            }
        }
        
        printf(color_success"[Thread %d] Building %d complete\n"color_reset, thread_id, i);
        
        free(d_local);
        free(prev_local);
    }
    
    double end_time = omp_get_wtime();
    printf(color_parallel"Parallel computation finished! Time: %.3f seconds\n"color_reset, end_time - start_time);
    printf(color_success"Pairwise matrix computed.\n"color_reset);

    /******************************************************
     * Held-Karp (TSP DP)
     ******************************************************/
    printf(color_parallel"Solving TSP using Held-Karp...\n"color_reset);

    int FULL = 1 << N;
    double** dp = malloc(sizeof(double*) * FULL);
    int** parent = malloc(sizeof(int*) * FULL);

    for (int m = 0; m < FULL; m++) {
        dp[m] = malloc(sizeof(double) * N);
        parent[m] = malloc(sizeof(int) * N);
        for (int i = 0; i < N; i++) {
            dp[m][i] = 1e18;
            parent[m][i] = -1;
        }
    }

    for (int i = 0; i < N; i++) {
        dp[1 << i][i] = 0;
    }

    for (int mask = 1; mask < FULL; mask++) {
        for (int last = 0; last < N; last++) {
            if (!(mask & (1 << last))) continue;

            int pm = mask ^ (1 << last);
            if (pm == 0) continue;

            for (int prevb = 0; prevb < N; prevb++) {
                if (!(pm & (1 << prevb))) continue;

                double c = dp[pm][prevb] + dist[prevb][last];
                if (c < dp[mask][last]) {
                    dp[mask][last] = c;
                    parent[mask][last] = prevb;
                }
            }
        }
    }

    int last = -1;
    double best = 1e18;
    for (int i = 0; i < N; i++) {
        if (dp[FULL - 1][i] < best) {
            best = dp[FULL - 1][i];
            last = i;
        }
    }

    if (best > 1e14) {
        fprintf(stderr, color_error"ERROR: Not all buildings are reachable in the graph.\n"color_reset);
        
        // Clean up memory
        for (int mask = 0; mask < FULL; mask++) {
            free(dp[mask]);
            free(parent[mask]);
        }
        free(dp);
        free(parent);
        
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) free(segs[i][j]);
            free(segs[i]);
            free(seglen[i]);
            free(dist[i]);
        }
        free(segs);
        free(seglen);
        free(dist);
        free(ids);
        
        return -1;
    }

    int* order = malloc(sizeof(int) * N);
    int m = FULL - 1, cur = last;
    for (int i = N - 1; i >= 0; i--) {
        order[i] = cur;
        int p = parent[m][cur];
        m ^= (1 << cur);
        cur = p;
    }

    /******************************************************
     * Build full node path
     ******************************************************/
    printf(color_success"Merging segments...\n"color_reset);
    
    int total = 0;
    for (int i = 0; i < N - 1; i++) {
        int a = order[i], b = order[i+1];
        if (i == 0) {
            total += seglen[a][b];  
        } else {
            total += seglen[a][b] - 1; 
        }
    }

    int* full = malloc(sizeof(int) * total);
    int idx = 0;

    for (int i = 0; i < N - 1; i++) {
        int a = order[i], b = order[i+1];
        
        int start = (i == 0) ? 0 : 1;
        
        for (int j = start; j < seglen[a][b]; j++) {
            full[idx++] = segs[a][b][j];
        }
    }

    *out_dist = best;
    *out_path = full;
    *out_len = idx;

    printf("\n");
    printf(color_success"=== Optimal Route Found ===\n"color_reset);
    printf(color_success"Total Distance: %.2f m\n"color_reset, best);
    printf(color_success"Total Nodes: %d\n"color_reset, idx);
    printf(color_success"Visit Order: "color_reset);
    for (int i = 0; i < N; i++) {
        printf("%s", names[order[i]]);
        if (i < N - 1) printf(" -> ");
    }
    printf("\n");

    // Clean up memory
    for (int mask = 0; mask < FULL; mask++) {
        free(dp[mask]);
        free(parent[mask]);
    }
    free(dp);
    free(parent);

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) free(segs[i][j]);
        free(segs[i]);
        free(seglen[i]);
        free(dist[i]);
    }
    free(segs);
    free(seglen);
    free(dist);

    free(order);
    free(ids);

    return 0;
}
