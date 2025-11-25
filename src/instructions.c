#include "instructions.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Calculate bearing between two points (0-360 degrees, 0=North)
double calculate_bearing(double lat1, double lon1, double lat2, double lon2) {
    // Convert to radians
    double lat1_rad = lat1 * M_PI / 180.0;
    double lat2_rad = lat2 * M_PI / 180.0;
    double lon_diff = (lon2 - lon1) * M_PI / 180.0;
    
    // Calculate bearing
    double y = sin(lon_diff) * cos(lat2_rad);
    double x = cos(lat1_rad) * sin(lat2_rad) - 
               sin(lat1_rad) * cos(lat2_rad) * cos(lon_diff);
    double bearing_rad = atan2(y, x);
    
    // Convert to degrees (0-360)
    double bearing = fmod((bearing_rad * 180.0 / M_PI) + 360.0, 360.0);
    return bearing;
}

// Get compass direction name from bearing
const char* get_direction_name(double bearing) {
    if (bearing >= 337.5 || bearing < 22.5) return "north";
    if (bearing >= 22.5 && bearing < 67.5) return "northeast";
    if (bearing >= 67.5 && bearing < 112.5) return "east";
    if (bearing >= 112.5 && bearing < 157.5) return "southeast";
    if (bearing >= 157.5 && bearing < 202.5) return "south";
    if (bearing >= 202.5 && bearing < 247.5) return "southwest";
    if (bearing >= 247.5 && bearing < 292.5) return "west";
    if (bearing >= 292.5 && bearing < 337.5) return "northwest";
    return "north";
}

// Calculate turn angle (-180 to 180)
double calculate_turn_angle(double bearing1, double bearing2) {
    double angle = bearing2 - bearing1;
    
    // Normalize to -180 to 180
    while (angle > 180.0) angle -= 360.0;
    while (angle < -180.0) angle += 360.0;
    
    return angle;
}

// Get turn instruction from turn angle
const char* get_turn_instruction(double turn_angle) {
    double abs_angle = fabs(turn_angle);
    
    if (abs_angle < 10.0) {
        return "Continue straight";
    } else if (turn_angle > 0) {
        // Right turn
        if (abs_angle < 45.0) return "Turn slight right";
        if (abs_angle < 135.0) return "Turn right";
        if (abs_angle < 170.0) return "Turn sharp right";
        return "Make a U-turn";
    } else {
        // Left turn
        if (abs_angle < 45.0) return "Turn slight left";
        if (abs_angle < 135.0) return "Turn left";
        if (abs_angle < 170.0) return "Turn sharp left";
        return "Make a U-turn";
    }
}

// Helper function to get building name from node ID
const char* get_building_name_from_node(BuildingMapping* mapping, int node_id) {
    if (!mapping) return NULL;
    
    for (int i = 0; i < mapping->count; i++) {
        if (mapping->entries[i].node_id == node_id) {
            return mapping->entries[i].name;
        }
    }
    return NULL;
}

// Helper function to find nearest building to a node
const char* get_nearest_building_description(Graph* graph, BuildingMapping* mapping, int node_id, double* distance_out) {
    if (!mapping || !graph) return NULL;
    
    double min_dist = 999999.0;
    const char* nearest_building = NULL;
    
    double node_lat = graph->nodes[node_id].lat;
    double node_lon = graph->nodes[node_id].lon;
    
    for (int i = 0; i < mapping->count; i++) {
        int building_node = mapping->entries[i].node_id;
        double dist = haversine(
            node_lat, node_lon,
            graph->nodes[building_node].lat, graph->nodes[building_node].lon
        );
        
        if (dist < min_dist) {
            min_dist = dist;
            nearest_building = mapping->entries[i].name;
        }
    }
    
    if (distance_out) {
        *distance_out = min_dist;
    }
    
    return nearest_building;
}

// Generate turn-by-turn instructions
char** generate_instructions(Graph* graph, int* path, int path_len, 
                            const char* start_name, const char* end_name,
                            BuildingMapping* mapping,
                            int* instruction_count) {
    if (path_len < 2) {
        *instruction_count = 0;
        return NULL;
    }
    
    // Allocate instruction array (max possible instructions)
    char** instructions = malloc(sizeof(char*) * (path_len + 2));
    int count = 0;
    
    // Start instruction
    instructions[count] = malloc(256);
    sprintf(instructions[count], "Start at %s", start_name);
    count++;
    
    // Initial direction
    if (path_len >= 2) {
        int node1 = path[0];
        int node2 = path[1];
        double bearing = calculate_bearing(
            graph->nodes[node1].lat, graph->nodes[node1].lon,
            graph->nodes[node2].lat, graph->nodes[node2].lon
        );
        const char* dir = get_direction_name(bearing);
        
        // Calculate initial segment distance
        double segment_dist = haversine(
            graph->nodes[node1].lat, graph->nodes[node1].lon,
            graph->nodes[node2].lat, graph->nodes[node2].lon
        );
        
        // Only add direction instruction if distance is significant
        if (segment_dist >= 5.0) {
            instructions[count] = malloc(256);
            sprintf(instructions[count], "Head %s for %.1f meters", dir, segment_dist);
            count++;
        }
    }
    
    // Process each turn
    double prev_bearing = 0.0;
    if (path_len >= 2) {
        prev_bearing = calculate_bearing(
            graph->nodes[path[0]].lat, graph->nodes[path[0]].lon,
            graph->nodes[path[1]].lat, graph->nodes[path[1]].lon
        );
    }
    
    double accumulated_distance = 0.0;
    
    for (int i = 1; i < path_len - 1; i++) {
        int curr_node = path[i];
        int next_node = path[i + 1];
        
        // Calculate bearing to next node
        double curr_bearing = calculate_bearing(
            graph->nodes[curr_node].lat, graph->nodes[curr_node].lon,
            graph->nodes[next_node].lat, graph->nodes[next_node].lon
        );
        
        // Calculate turn angle
        double turn_angle = calculate_turn_angle(prev_bearing, curr_bearing);
        
        // Calculate segment distance
        double segment_dist = haversine(
            graph->nodes[curr_node].lat, graph->nodes[curr_node].lon,
            graph->nodes[next_node].lat, graph->nodes[next_node].lon
        );
        
        // If significant turn (> 20 degrees), create turn instruction
        if (fabs(turn_angle) > 20.0) {
            const char* turn_text = get_turn_instruction(turn_angle);
            instructions[count] = malloc(300);
            
            // Try to get building name for this node
            const char* building_name = get_building_name_from_node(mapping, curr_node);
            
            if (building_name) {
                sprintf(instructions[count], "%s at %s", turn_text, building_name);
            } else {
                // Node is not a building - find nearest building for reference
                double nearest_dist = 0.0;
                const char* nearest_building = get_nearest_building_description(graph, mapping, curr_node, &nearest_dist);
                
                if (nearest_building && nearest_dist < 100.0) {
                    // Reference nearby building if within 100 meters
                    sprintf(instructions[count], "%s near %s", turn_text, nearest_building);
                } else {
                    // Fall back to intersection description
                    sprintf(instructions[count], "%s at the intersection", turn_text);
                }
            }
            count++;
            
            // Add continuing direction with distance (only if significant)
            if (segment_dist >= 10.0) {
                const char* dir = get_direction_name(curr_bearing);
                instructions[count] = malloc(256);
                sprintf(instructions[count], "Continue %s for %.1f meters", dir, segment_dist);
                count++;
            }
            
            accumulated_distance = 0.0;
        } else {
            // Accumulate distance for straight segments
            accumulated_distance += segment_dist;
        }
        
        prev_bearing = curr_bearing;
    }
    
    // Final instruction
    instructions[count] = malloc(256);
    sprintf(instructions[count], "You have reached %s", end_name);
    count++;
    
    *instruction_count = count;
    return instructions;
}

// Free instruction array
void free_instructions(char** instructions, int count) {
    if (instructions) {
        for (int i = 0; i < count; i++) {
            free(instructions[i]);
        }
        free(instructions);
    }
}