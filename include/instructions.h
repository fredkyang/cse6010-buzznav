#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include "graph.h"

// Direction constants
typedef enum {
    DIR_NORTH,
    DIR_NORTHEAST,
    DIR_EAST,
    DIR_SOUTHEAST,
    DIR_SOUTH,
    DIR_SOUTHWEST,
    DIR_WEST,
    DIR_NORTHWEST
} CompassDirection;

// Turn type constants
typedef enum {
    TURN_STRAIGHT,
    TURN_SLIGHT_LEFT,
    TURN_LEFT,
    TURN_SHARP_LEFT,
    TURN_SLIGHT_RIGHT,
    TURN_RIGHT,
    TURN_SHARP_RIGHT,
    TURN_UTURN
} TurnType;

// Calculate bearing (angle) between two points in degrees (0-360)
// 0/360 = North, 90 = East, 180 = South, 270 = West
double calculate_bearing(double lat1, double lon1, double lat2, double lon2);

// Get compass direction name from bearing
const char* get_direction_name(double bearing);

// Calculate turn angle between two bearings (-180 to 180)
// Positive = right turn, Negative = left turn
double calculate_turn_angle(double bearing1, double bearing2);

// Get turn instruction from turn angle
const char* get_turn_instruction(double turn_angle);

// Generate turn-by-turn instructions for a path
// Returns array of instruction strings (must be freed by caller)
char** generate_instructions(Graph* graph, int* path, int path_len, 
                            const char* start_name, const char* end_name,
                            BuildingMapping* mapping,
                            int* instruction_count);

// Free instruction array
void free_instructions(char** instructions, int count);

#endif // INSTRUCTIONS_H