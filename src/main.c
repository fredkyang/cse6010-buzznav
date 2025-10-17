#include <stdio.h>
#include <stdlib.h>
#include "graph.h"

int main(int argc, char *argv[]){
    if (argc != 3) {
        fprintf(stderr, "\033[33m:( Usage: %s \"<start building name>\" \"<goal building name>\"\n     e.g. %s \"Student Center\" \"College of Computing Building\"\n", argv[0], argv[0]);
        return 1;
    }

    const char* start_building = argv[1];
    const char* goal_building = argv[2];

    // Step 1: Load graph from file
    Graph* campus = load_graph("./data/adj_list.csv"); // Build adjacency list of the campus
    if (!campus) {
        return 1;
    }

    // print graph to check its structure
    // print_graph(campus);

    BuildingMapping* mapping = load_building("./data/building_mapping.csv"); // Load building mapping
    if (!mapping) {
        return 1;
    }

    // Step 2: Search building node IDs
    int start_id = get_building_id(mapping, start_building);
    int goal_id = get_building_id(mapping, goal_building);
    free_building(mapping); // free the building mapping

    if (start_id == -1 || goal_id == -1) {
        return 1;
    }

    

    // Step 3: Run algorithm

    // Step 4: Compute route metrics

    // Step 5: Generate turn-by-turn instructions

    // Step 6: Print results

    // Step 7: Clean up
    free_graph(campus);
    return 0;
}