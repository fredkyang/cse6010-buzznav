#include <stdio.h>
#include <stdlib.h>
#include "graph.h"
#include "astar.h"

int main(int argc, char *argv[]){
    if (argc != 3) {
        fprintf(stderr, "\033[33m:( Usage: %s \"<start building name>\" \"<goal building name>\"\n     e.g. %s \"Student Center\" \"College of Computing Building\"\n", argv[0], argv[0]);
        return 1;
    }

    const char* start_building = argv[1];
    const char* goal_building = argv[2];

    // Step 1: Load graph and buildingmapping from file
    Graph* campus = load_graph("./data/adj_list.csv"); // Build adjacency list of the campus
    if (!campus) {
        free_graph(campus);
        return 1;
    }

    // print graph to check its structure
    // print_graph(campus);

    BuildingMapping* mapping = load_building("./data/building_mapping.csv"); // Load building mapping
    if (!mapping) {
        free_graph(campus);
        free_building(mapping);
        return 1;
    }

    // Step 2: Search building node IDs
    int start_id = get_building_id(mapping, start_building);
    int goal_id = get_building_id(mapping, goal_building);

    if (start_id == -1 || goal_id == -1) {
        free_graph(campus);
        free_building(mapping);
        return 1;
    }


    // Step 3: Run algorithm
    int* path = NULL;
    int path_len = 0;
    double total_dist = astar(campus, start_id, goal_id, &path, &path_len);

    if (path) {
        // ---- (A) Print to terminal ----
        printf("\nShortest Path (%.2f m): ", total_dist);
        for (int i = 0; i < path_len; i++) {
            printf("%d", path[i]);
            if (i < path_len - 1) printf(" -> ");
        }
        printf("\n");

        // ---- (B) Write path to file ----
        FILE* fout = fopen("./temp/path_output.txt", "w");
        if (fout == NULL) {
            perror("Error opening path_output.txt");
        } else {
            for (int i = 0; i < path_len; i++) {
                fprintf(fout, "%d", path[i]);
                if (i < path_len - 1) fprintf(fout, " ");
            }
            fprintf(fout, "\n");
            fclose(fout);
        }

        free(path);
    } else {
        printf("No path found.\n");
    }



    // Step 4: Compute route metrics

    // Step 5: Generate turn-by-turn instructions

    // Step 6: Print results

    // Step 7: Clean up
    free_building(mapping);
    free_graph(campus);
    return 0;
}