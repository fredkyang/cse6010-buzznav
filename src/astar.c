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
           
        }
        
    }
    return min_idx; //find the min node or not
}

double astar(int start, int goal) {
    //step 1 check everything ok at first
    /*
    I do not know whether need this one
     if (!graph) {
        printf("error: graph not initialized\n");
        return -1;
    }
    */
   // node Id is valid or not
   if (start<0||start>=graph->num_nodes||goal<0||goal>=graph->num_nodes)
   {
     printf("error: invalid node ID (start=%d, goal=%d, max=%d)\n", 
        start, goal, graph->num_nodes - 1);
        return -1;
    /* code */
   }
   int num_nodes=graph->num_nodes;

   // step 2  memory allocation and intiallization
   AStarNode* nodes = (AStarNode*)calloc(num_nodes, sizeof(AStarNode));
   if (!nodes)
   {
    printf("error:memory allocation fail\n");
    return -1;
    /* code */
   }
   for (int i = 0; i < num_nodes; i++)
   {nodes[i].f_score=DBL_MAX;
    nodes[i].g_score=DBL_MAX;
    nodes[i].parent=-1;
    nodes[i].visited=false;
    /* code */
   }

   //step 3 initialize start
   nodes[start].f_score=0;
   nodes[start].g_score=0;
   
   //step 4
/*
*
Since we already computed distances between adjacent buildings from the adjacency list, 
our A* effectively behaves like Dijkstra’s algorithm. 
However, we now have precise latitude–longitude coordinates for the nodes, 
so if required we can compute edge costs (or a heuristic) directly from 
geodesic distances using the Haversine function. 
This lets us incorporate a proper heuristic and run true A* rather than pure Dijkstra.
*
*/
// main a* loop
/*
* reach goal or all nodes
*/

while (1)
{
    /* code */
}



}