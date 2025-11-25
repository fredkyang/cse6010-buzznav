#include <stdio.h>
#include <stdlib.h>
#include "graph.h"
#include "astar.h"
#include "via_point.h"
#include "instructions.h"

int main(int argc, char *argv[]){
    if (argc < 3) {
        fprintf(stderr, "{\"status\": \"error\", \"message\": \"Usage: %s <start> [via1 via2 ...] <goal>\"}\n", argv[0]);
        return 1;
    }

    const char* start_building = argv[1];
    const char* goal_building = argv[argc - 1];
    int num_via = argc - 3; // arguments between start and goal

    // Step 1: Load graph and buildingmapping from file
    Graph* campus = load_graph("../data/adj_list.csv"); // Build adjacency list of the campus
    if (!campus) {
        fprintf(stderr, "{\"status\": \"error\", \"message\": \"Failed to load graph\"}\n");
        return 1;
    }

    if (load_node_coordinates(campus, "../data/node_coordinates.csv") != 0)
    {
        fprintf(stderr, "{\"status\": \"error\", \"message\": \"Failed to load coordinates\"}\n");
        free_graph(campus);
        return 1;
    }

    BuildingMapping *mapping = load_building("../data/building_mapping.csv"); // Load building mapping
    if (!mapping)
    {
        fprintf(stderr, "{\"status\": \"error\", \"message\": \"Failed to load building mapping\"}\n");
        free_graph(campus);
        return 1;
    }

    // Step 2: Search building node IDs
    int start_id = get_building_id(mapping, start_building);
    int goal_id = get_building_id(mapping, goal_building);

    if (start_id == -1) {
        fprintf(stderr, "{\"status\": \"error\", \"message\": \"Building not found: %s\"}\n", start_building);
        free_building(mapping);
        free_graph(campus);
        return 1;
    }
    
    if (goal_id == -1) {
        fprintf(stderr, "{\"status\": \"error\", \"message\": \"Building not found: %s\"}\n", goal_building);
        free_building(mapping);
        free_graph(campus);
        return 1;
    }

    int* via_ids = NULL;
    if (num_via > 0) {
        via_ids = malloc(sizeof(int) * num_via);
        if (!via_ids) {
            fprintf(stderr, "{\"status\": \"error\", \"message\": \"Failed to allocate via list\"}\n");
            free_building(mapping);
            free_graph(campus);
            return 1;
        }

        for (int i = 0; i < num_via; i++) {
            via_ids[i] = get_building_id(mapping, argv[2 + i]);
            if (via_ids[i] == -1) {
                fprintf(stderr, "{\"status\": \"error\", \"message\": \"Via point not found: %s\"}\n", argv[2 + i]);
                free(via_ids);
                free_building(mapping);
                free_graph(campus);
                return 1;
            }
        }
    }


    // Step 3: Run algorithm
    int *path = NULL;
    int path_len = 0;
    double total_dist = astar_via_points(campus, start_id, goal_id, via_ids, num_via, &path, &path_len);

    if (path && path_len > 0)
    {
        // Print results in JSON format
        printf("{\n");
        printf("  \"status\": \"success\",\n");
        printf("  \"total_distance\": %.2f,\n", total_dist);

        // Print path coordinates
        printf("  \"path_coordinates\": [\n");
        for (int i = 0; i < path_len; i++)
        {
            int node_id = path[i];
            // Get latitude and longitude from the graph
            double lat = campus->nodes[node_id].lat;
            double lon = campus->nodes[node_id].lon;
            printf("    [%f, %f]", lat, lon);
            if (i < path_len - 1)
            {
                printf(",\n");
            }
            else
            {
                printf("\n");
            }
        }
        printf("  ],\n");

        // Find via point indices in the path
        if (num_via > 0) {
            printf("  \"via_point_indices\": [");
            for (int v = 0; v < num_via; v++) {
                // Find where this via point appears in the path
                int via_index = -1;
                for (int i = 0; i < path_len; i++) {
                    if (path[i] == via_ids[v]) {
                        via_index = i;
                        break;
                    }
                }
                printf("%d", via_index);
                if (v < num_via - 1) {
                    printf(", ");
                }
            }
            printf("],\n");
        }

        // Generate instructions
        int instruction_count = 0;
        char** instructions_array = generate_instructions(campus, path, path_len, 
                                                         start_building, goal_building,
                                                         mapping,
                                                         &instruction_count);
        
        printf("  \"instructions\": [\n");
        if (instructions_array) {
            for (int i = 0; i < instruction_count; i++) {
                printf("    \"%s\"", instructions_array[i]);
                if (i < instruction_count - 1) {
                    printf(",\n");
                } else {
                    printf("\n");
                }
            }
            free_instructions(instructions_array, instruction_count);
        } else {
            // Fallback instructions
            printf("    \"Start at %s\",\n", start_building);
            if (num_via > 0) {
                for (int i = 0; i < num_via; i++) {
                    printf("    \"Pass through %s\",\n", argv[2 + i]);
                }
            }
            printf("    \"Follow the path for %.0f meters\",\n", total_dist);
            printf("    \"Arrive at %s\"\n", goal_building);
        }
        printf("  ]\n");

        printf("}\n");

        free(path);
    }
    else
    {
        // No path found
        fprintf(stderr, "{\"status\": \"error\", \"message\": \"No path found between %s and %s\"}\n", 
                start_building, goal_building);
    }

    // Step 7: Clean up
    free(via_ids);
    free_building(mapping);
    free_graph(campus);
    return 0;
}