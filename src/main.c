#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graph.h"
#include "astar.h"
#include "via_point.h"
#include "instructions.h"
#include "tsp.h"
#include "utils.h"

// --- Helper Functions ---

// Consolidate resource loading
int load_campus_resources(Graph** campus, BuildingMapping** mapping) {
    fprintf(stderr, "[INFO] Loading graph data...\n");
    
    *campus = load_graph("../data/adj_list.csv");
    if (!*campus) return 0;

    if (load_node_coordinates(*campus, "../data/node_coordinates.csv") != 0) {
        free_graph(*campus);
        return 0;
    }

    *mapping = load_building("../data/building_mapping.csv");
    if (!*mapping) {
        free_graph(*campus);
        return 0;
    }

    return 1; // Success
}

// --- Logic Handlers ---

void handle_tsp_mode(Graph* campus, BuildingMapping* mapping, int argc, char *argv[]) {
    NavigationResult res;
    init_result(&res);

    int num_buildings = argc - 2;
    const char** building_names = (const char**)&argv[2];

    fprintf(stderr, "[INFO] Running TSP for %d buildings\n", num_buildings);

    // Call TSP Solver
    int result_code = find_optimal_multi_building_route(
        campus, mapping, building_names, num_buildings,
        &res.total_distance, &res.path_node_ids, &res.path_length
    );

    if (result_code != 0) {
        res.status_code = 1;
        res.error_message = "TSP optimization failed";
        print_json_response(&res, campus);
        return;
    }

    // Identify ALL building nodes in the final path
    res.num_via_indices = num_buildings - 2;
    if (res.num_via_indices > 0) {
        res.via_indices = malloc(sizeof(int) * res.num_via_indices);
        int via_count = 0;
        
        for (int j = 1; j < res.path_length - 1 && via_count < res.num_via_indices; j++) {
            int node_id = res.path_node_ids[j];
            
            for (int i = 0; i < num_buildings; i++) {
                int b_id = get_building_id(mapping, building_names[i]);
                if (node_id == b_id) {
                    if (j != 0 && j != res.path_length - 1) {
                        res.via_indices[via_count++] = j;
                        break;
                    }
                }
            }
        }
        
        res.num_via_indices = via_count;
    }

    // Generate Instructions
    res.instructions = generate_instructions(
        campus, res.path_node_ids, res.path_length,
        building_names[0], building_names[num_buildings-1],
        mapping, &res.instruction_count
    );

    // Output
    print_json_response(&res, campus);
    
    // Cleanup
    if(res.path_node_ids) free(res.path_node_ids);
    if(res.via_indices) free(res.via_indices);
}

void handle_navigation_mode(Graph* campus, BuildingMapping* mapping, int argc, char *argv[]) {
    NavigationResult res;
    init_result(&res);

    const char* start_name = argv[1];
    const char* end_name = argv[argc - 1];
    int num_via = argc - 3;

    fprintf(stderr, "[INFO] Routing: %s -> %s (Via: %d)\n", start_name, end_name, num_via);

    int start_id = get_building_id(mapping, start_name);
    int end_id = get_building_id(mapping, end_name);

    if (start_id == -1 || end_id == -1) {
        print_json_error("Start or End building not found in mapping");
        return;
    }

    // Process Via Points
    int* via_ids = NULL;
    if (num_via > 0) {
        via_ids = malloc(sizeof(int) * num_via);
        for (int i = 0; i < num_via; i++) {
            via_ids[i] = get_building_id(mapping, argv[2 + i]);
            if (via_ids[i] == -1) {
                print_json_error("One of the via points was not found");
                free(via_ids);
                return;
            }
        }
    }

    // Execute A* (Algorithm is now triggered here, outside main)
    res.total_distance = astar_via_points(
        campus, start_id, end_id, via_ids, num_via, 
        &res.path_node_ids, &res.path_length
    );

    if (res.path_node_ids && res.path_length > 0) {
        // Calculate via indices for UI markers
        if (num_via > 0) {
            res.num_via_indices = num_via;
            res.via_indices = malloc(sizeof(int) * num_via);
            for (int v = 0; v < num_via; v++) {
                res.via_indices[v] = -1;
                for (int i = 0; i < res.path_length; i++) {
                    if (res.path_node_ids[i] == via_ids[v]) {
                        res.via_indices[v] = i;
                        break;
                    }
                }
            }
        }

        // Generate Instructions
        res.instructions = generate_instructions(
            campus, res.path_node_ids, res.path_length,
            start_name, end_name, mapping, &res.instruction_count
        );

        print_json_response(&res, campus);
    } else {
        print_json_error("No path found");
    }

    // Cleanup local allocations
    if(res.path_node_ids) free(res.path_node_ids);
    if(res.via_indices) free(res.via_indices);
    if(via_ids) free(via_ids);
}

// --- Main Entry Point ---

int main(int argc, char *argv[]) {
    // 1. Basic Validation
    if (argc < 2) {
        print_json_error("Invalid arguments provided");
        return 1;
    }

    // 2. Load Resources (Once for the whole execution)
    Graph* campus = NULL;
    BuildingMapping* mapping = NULL;
    
    if (!load_campus_resources(&campus, &mapping)) {
        print_json_error("Failed to load map data or coordinates");
        return 1;
    }

    // 3. Dispatch based on mode
    if (strcmp(argv[1], "--tsp") == 0) {
        if (argc < 4) {
            print_json_error("TSP mode requires at least 2 buildings");
        } else {
            handle_tsp_mode(campus, mapping, argc, argv);
        }
    } else {
        if (argc < 3) {
            print_json_error("Usage: <start> [via...] <end>");
        } else {
            handle_navigation_mode(campus, mapping, argc, argv);
        }
    }

    // 4. Cleanup Global Resources
    free_building(mapping);
    free_graph(campus);

    return 0;
}
