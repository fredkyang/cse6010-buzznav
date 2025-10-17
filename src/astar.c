#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "astar.h"
#include "graph.h"
#include <stdbool.h>
#include <float.h>
// global variables declaration
/*
*External graph pointer (defined in main.c or other source file)
* provide access to the campus map data structure
*/
extern Graph* graph;

// data structures
/*
*astarnode
*
*g_score:actual distance from start to current node
*f_score: total estimated cost f=g+h
*parent:the node for path reconstruction
*visited: processed or not
*/
typedef struct{
    double g_score;
    double f_score;
    int parent;
    bool visited;
} AStarNode;

// Helper Functions
/*
*find_min_node=fine the unvisited node(min f_score)
*
*Parameters
*nodes   =array of A* node information
*num_nodes   = total number of nodes
*
*return
*min f_score node or -1 for nothing
*
*algorithm
*intialation:min_f=infinity
*go throgh every node
*find_min_node
*/
int find_min_node(AStarNode* nodes, int num_nodes)
{
    int min_idx=-1;
    double min_f=DBL_MAX; //very big 
    //go through every node
    for (int i=0;i<num_nodes;i++)
    {
        if (nodes[i].f_score<min_f && !nodes[i].visited)//smaller min_f and unvisited
        {   
            min_f = nodes[i].f_score;
            min_idx = i;
            /* code */
        }
        
    }
    return min_idx; //find the min node or not
}

double astar(int start, int goal) {
    
}