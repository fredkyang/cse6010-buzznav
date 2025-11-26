#ifndef GRAPH_H
#define GRAPH_H

#define MAX_BUILDINGS 400 // Building capacity

// ========================
// Graph Struct Definitions
// Adjacency List Representation
// ========================

typedef struct Edge
{
    int to;        // destination node
    double weight; // road length
    struct Edge *next;
} Edge;

typedef struct Node
{ // Adjacency list node
    int id;
    double lat, lon;
    Edge *neighbors; // point to the first edge of the linked list
} Node;

typedef struct Graph
{
    int num_nodes;
    Node *nodes; // array of nodes
} Graph;

// ======================
// Graph Function Declarations
// ======================
Graph *create_graph(int num_nodes);
void add_edge(Graph *g, int from, int to, double weight);
Graph *load_graph(const char *filename);
void print_graph(const Graph *g);
void free_graph(Graph *g);
int load_node_coordinates(Graph *g, const char *filename);

// ========================
// Building Struct Definitions
// ========================
typedef struct BuildingEntry
{
    char *name;
    int node_id;
} BuildingEntry;

typedef struct BuildingMapping
{
    BuildingEntry *entries;
    int count;
    int capacity;
} BuildingMapping;

BuildingMapping *load_building(const char *filename);
int get_building_id(const BuildingMapping *mapping, const char *name);
void free_building(BuildingMapping *mapping);
#endif