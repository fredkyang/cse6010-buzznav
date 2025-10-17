#include <stdio.h>
#include <stdlib.h>
#include "graph.h"




int main(int argc, char *argv[]){
    if (argc < 3) {
        printf("Usage: %s <start building name> <goal building name>\n", argv[0]);
        return 1;
    }

    const char* start_building = argv[1];
    const char* goal_building = argv[2];

    // Step 1: Load graph from file
    Graph* campus = load_graph("../data/adj_list.csv"); // Build adjacency list of the campus
    if (!campus) {
        fprintf(stderr, "Failed to load adj_list.csv file.\n");
        return 1;
    }

    BuildingMapping* mapping = load_building("../data/building_mapping.csv"); // Load building mapping
    if (!mapping) {
        fprintf(stderr, "Failed to load building_mapping.csv file.\n");
        return 1;
    }

    // Step 2: Search building node IDs
    const char* start_building = argv[1];
    const char* goal_building = argv[2];
    int start_id = get_node_id(mapping, start_building);
    int goal_id = get_node_id(mapping, goal_building);
    free_building_mapping(mapping); // free the building mapping

    printf("Start node ID: %d\n", start_id);
    printf("Goal node ID: %d\n", goal_id);

    // Step 3: Run algorithm

    // Step 4: Compute route metrics

    // Step 5: Generate turn-by-turn instructions

    // Step 6: Print results

    // Step 7: Clean up
    free_graph(campus);
    return 0;
}