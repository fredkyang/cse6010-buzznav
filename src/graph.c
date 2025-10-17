#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graph.h"

Graph* create_graph(int num_nodes) { // Initialize graph represented by adjacency list
    Graph* g = malloc(sizeof(Graph));
    g->num_nodes = num_nodes;
    g->nodes = malloc(num_nodes * sizeof(Node));
    for (int i = 0; i < num_nodes; i++) {
        g->nodes[i].id = i;
        g->nodes[i].neighbors = NULL;
    }
    return g;
}

void add_edge(Graph* g, int from, int to, double weight) { // Add directed edge
    Edge* e = malloc(sizeof(Edge));
    e->to = to;
    e->weight = weight;

    // inset edge into linked list
    e->next = g->nodes[from].neighbors;
    g->nodes[from].neighbors = e;
}

Graph* load_graph(const char* filename) {
    FILE* input = fopen(filename, "r");
    if (!input) {
        fprintf(stderr, "\033[31m:( Failed loading graph file '%s'\n", filename);
        return NULL;
    }

    char buffer[256]; // safe size buffer
    int from, to;
    double weight;
    int max_id = 0;

    fgets(buffer, sizeof(buffer), input); // ignore header

    // Find max node ID
    while (fscanf(input, "%d,%d,%lf", &from, &to, &weight) == 3) {
        if (from > max_id) max_id = from;
        if (to > max_id) max_id = to;
    }

    rewind(input);
    fgets(buffer, sizeof(buffer), input); // ignore header

    Graph* g = create_graph(max_id + 1);

    // Add edges
    while (fscanf(input, "%d,%d,%lf", &from, &to, &weight) == 3) {
        add_edge(g, from, to, weight);
    }

    fclose(input);
    fprintf(stdout, "\033[32m:) Load campus graph suscessfully\n");
    return g;
}

// Print graph structure
void print_graph(const Graph* g) {
    for (int i = 0; i < g->num_nodes; i++) {
        Edge* e = g->nodes[i].neighbors;
        if (e) {
            printf("Node %d:", i);
            while (e) {
                printf(" -> %d(%.2f)", e->to, e->weight);
                e = e->next;
            }
            printf("\n");
        }
    }
}

void free_graph(Graph* g) {
    for (int i = 0; i < g->num_nodes; i++) {
        Edge* e = g->nodes[i].neighbors;
        while(e) {
            Edge* temp = e;
            e = e->next;
            free(temp);
        }
    }
    free(g->nodes);
    free(g);
    fprintf(stdout, "\033[32m:) Free campus graph\n");
}

BuildingMapping* load_building(const char* filename) {
    FILE* input = fopen(filename, "r");
    if (!input) {
        fprintf(stderr, "\033[31m:( Failed reading building mapping file '%s", filename);
        return NULL;
    }

    char buffer[256];
    char name[200];
    int id;

    BuildingMapping* mapping = malloc(sizeof(BuildingMapping));
    mapping->entries = malloc(MAX_BUILDINGS * sizeof(BuildingEntry));
    mapping->count = 0;
    mapping->capacity = MAX_BUILDINGS;

    fgets(buffer, sizeof(buffer), input); // ignore header

    while (fgets(buffer, sizeof(buffer), input)) {
        char* token = strtok(buffer, ","); // split string by ","
        if (!token) continue;
        strcpy(name, token);
        token = strtok(NULL, ",");
        if (!token) continue;
        id = atoi(token); // convert to int

        mapping->entries[mapping->count].name = strdup(name);
        mapping->entries[mapping->count].node_id = id;
        mapping->count++;
    }

    fclose(input);
    return mapping;
}

int get_building_id(const BuildingMapping* mapping, const char* name) {
    for (int i = 0; i < mapping->count; i++) {
        if (strcmp(mapping->entries[i].name, name) == 0) {
            fprintf(stdout, "\033[32m:) Note down location of '%s'\n", name);
            return mapping->entries[i].node_id;
        }
    }
    fprintf(stderr, "\033[31m:(\033[0m Building '%s' not found in mapping\n", name);
    return -1; // no name matching
}

void free_building(BuildingMapping* mapping) {
    for (int i = 0; i < mapping->count; i++) {
        free(mapping->entries[i].name);
    }
    free(mapping->entries);
    free(mapping);
    fprintf(stdout, "\033[32m:) Free building mapping\n");
}