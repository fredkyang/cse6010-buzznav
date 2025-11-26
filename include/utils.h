#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include "graph.h"
#include "tsp.h"

#define R 6371000.0 // raduis of the Earth in meters

// Struct to hold standard response data to avoid passing many arguments
typedef struct {
    double total_distance;
    int* path_node_ids;
    int path_length;
    int* via_indices;   // Indices in the path array where via points occur
    int num_via_indices;
    char** instructions;
    int instruction_count;
    int status_code;    // 0 = success, 1 = error
    char* error_message;
} NavigationResult;

// Helper to init result with defaults
void init_result(NavigationResult* res);
void free_result(NavigationResult* res);

// Outputs the NavigationResult structure as formatted JSON to STDOUT
void print_json_response(NavigationResult* res, Graph* campus);

// Helper to print errors to stderr and return a JSON error to stdout
void print_json_error(const char* message);

double haversine(double lat1, double lon1, double lat2, double lon2);

#endif