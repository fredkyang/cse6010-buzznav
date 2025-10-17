<<<<<<< Updated upstream

// ==================
// Struct Definitions
// ==================
typedef struct {
    int id;
    double lat;
    double lon;
} Node;

typedef struct {
    int from;
    int to;
    double weight;
} Edge;

// ==================
// Function Declarations
// ==================
=======
#ifndef GRAPH_H
#define GRAPH_H
// ========================
// Graph Struct Definitions
// ========================
typedef struct Edge {
    int to; // goal node
    double weight; // road length
    struct Edge* next;
} Edge;

typedef struct Node {
    int id;
    Edge* neighbors;
} Node;

typedef struct Graph {
    int num_nodes;
    Node* nodes;
} Graph;

// ======================
// Graph Function Declarations
// ======================
Graph* create_graph(int num_nodes);
void add_edge(Graph* graph, int from, int )
#endif
>>>>>>> Stashed changes
