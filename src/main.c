#include <stdio.h>
#include <stdlib.h>
#include "graph.h"
#include "astar.h"

int load_node_coordinates(Graph *g, const char *filename);

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "\033[33m:( Usage: %s \"<start building name>\" \"<goal building name>\"\n     e.g. %s \"Student Center\" \"College of Computing Building\"\n", argv[0], argv[0]);
        return 1;
    }

    const char *start_building = argv[1];
    const char *goal_building = argv[2];

    // Step 1: Load graph and buildingmapping from file
    Graph *campus = load_graph("../data/adj_list.csv"); // Build adjacency list of the campus
    if (!campus)
    {
        free_graph(campus);
        return 1;
    }

    if (load_node_coordinates(campus, "../data/node_coordinates.csv") != 0)
    {
        free_graph(campus);
        return 1;
    }
    // print graph to check its structure
    // print_graph(campus);

    BuildingMapping *mapping = load_building("../data/building_mapping.csv"); // Load building mapping
    if (!mapping)
    {
        free_graph(campus);
        free_building(mapping);
        return 1;
    }

    // Step 2: Search building node IDs
    int start_id = get_building_id(mapping, start_building);
    int goal_id = get_building_id(mapping, goal_building);

    if (start_id == -1 || goal_id == -1)
    {
        free_graph(campus);
        free_building(mapping);
        return 1;
    }

    // Step 3: Run algorithm
    int *path = NULL;
    int path_len = 0;
    double total_dist = astar(campus, start_id, goal_id, &path, &path_len);

    if (path)
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

        // Placeholder for instructions
        printf("  \"instructions\": [\n");
        printf("    \"Instructions are not yet implemented.\"\n");
        printf("  ]\n");

        printf("}\n");

        free(path);
    }
    else
    {
        // No path found
        printf("{\n");
        printf("  \"status\": \"error\",\n");
        printf("  \"message\": \"No path found between %s and %s.\"\n", start_building, goal_building);
        printf("}\n");
    }

    // Step 4: Compute route metrics

    // Step 5: Generate turn-by-turn instructions

    // Step 6: Print results

    // Step 7: Clean up
    free_building(mapping);
    free_graph(campus);
    return 0;
}