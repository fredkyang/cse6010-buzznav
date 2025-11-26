#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

// Haversine Distance
double haversine(double lat1, double lon1, double lat2, double lon2) {
    // Convert degrees to radians
    lat1 = lat1 * M_PI / 180.0;
    lat2 = lat2 * M_PI / 180.0;
    lon1 = lon1 * M_PI / 180.0;
    lon2 = lon2 * M_PI / 180.0;

    double dlat = lat2 - lat1;
    double dlon = lon2 - lon1;

    double hav = pow(sin(dlat / 2), 2) + cos(lat1) * cos(lat2) * pow(sin(dlon /2), 2);
    double distance = 2 * R * asin(sqrt(hav));

    return distance;
}

void init_result(NavigationResult* res) {
    res->total_distance = 0.0;
    res->path_node_ids = NULL;
    res->path_length = 0;
    res->via_indices = NULL;
    res->num_via_indices = 0;
    res->instructions = NULL;
    res->instruction_count = 0;
    res->status_code = 0;
    res->error_message = NULL;
}

void free_result(NavigationResult* res) {
    if (res->path_node_ids) free(res->path_node_ids);
    if (res->via_indices) free(res->via_indices);
    if (res->instructions) {
        // Assuming instructions were allocated individually
        for(int i=0; i<res->instruction_count; i++) free(res->instructions[i]);
        free(res->instructions);
    }
    // Note: error_message is usually a string literal in this context, 
    // but if you malloc it, free it here.
}

void print_json_error(const char* message) {
    // We print the JSON structure to STDOUT for the Python API to catch
    printf("{\n");
    printf("  \"status\": \"error\",\n");
    printf("  \"message\": \"%s\"\n", message);
    printf("}\n");
}

void print_json_response(NavigationResult* res, Graph* campus) {
    if (res->status_code != 0) {
        print_json_error(res->error_message ? res->error_message : "Unknown error");
        return;
    }

    printf("{\n");
    printf("  \"status\": \"success\",\n");
    printf("  \"total_distance\": %.2f,\n", res->total_distance);
    
    // Output Coordinates
    printf("  \"path_coordinates\": [\n");
    for (int i = 0; i < res->path_length; i++) {
        int node_id = res->path_node_ids[i];
        double lat = campus->nodes[node_id].lat;
        double lon = campus->nodes[node_id].lon;
        printf("    [%f, %f]", lat, lon);
        if (i < res->path_length - 1) printf(",\n");
    }
    printf("\n  ],\n");

    // Output Via Indices
    printf("  \"via_point_indices\": [");
    if (res->via_indices && res->num_via_indices > 0) {
        for (int i = 0; i < res->num_via_indices; i++) {
            printf("%d", res->via_indices[i]);
            if (i < res->num_via_indices - 1) printf(", ");
        }
    }
    printf("],\n");

    // Output Instructions
    printf("  \"instructions\": [\n");
    if (res->instructions && res->instruction_count > 0) {
        for (int i = 0; i < res->instruction_count; i++) {
            // Escape quotes if necessary, simplified here
            printf("    \"%s\"", res->instructions[i]);
            if (i < res->instruction_count - 1) printf(",\n");
        }
    } else {
         printf("    \"Follow the path to your destination.\"\n");
    }
    printf("\n  ]\n");
    printf("}\n");
}