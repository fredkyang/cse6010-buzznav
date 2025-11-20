#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <omp.h>
#include "via_point.h"
#include "astar.h"

// color 
#define color_direct   "\033[34m"  
#define color_parallel "\033[33m"  
#define color_success  "\033[32m"  
#define color_error    "\033[31m"  
#define color_reset    "\033[0m"   
/*
* split into num_via+1 segments
* compute each segment in parallel using A* algorithm
* merge all segments into a complete path
*/

/*
* parallelization
* each segment is independment can be computed sametimes
* speedup, about *num_via+1 (in ideal condition)
*/

double astar_via_points(Graph* graph,int start_id,int goal_id,
int*via_ids,int num_via,int**out_path, int* out_len)
{
// no via points use direct A*
 if (num_via==0)
 {
    // blue=direct
    printf(color_direct"direct route \n" color_reset);
    return astar(graph,start_id,goal_id,out_path,out_len);
    /* code */
 }
 // how many segments
  int num_segments=num_via+1;
  // allocate arrays to store results for each segment
  // segment_paths[i] is pointer to the i segment path array
  int** segment_paths=malloc(sizeof(int*)*num_segments);
  // lens = nodes in i segment
  int* segment_lens=malloc(sizeof(int*)*num_segments);
  // distance of i segment
  double* segment_dists=malloc(sizeof(double*)*num_segments);

  // intialize
  for (int i = 0; i < num_segments; i++)
  {
    segment_dists[i]=DBL_MAX;
    segment_lens[i]=0;
    segment_paths[i]=NULL;
    /* code */
  }
  // yellow = start parrallel
  #pragma omp parallel for num_threads(num_segments)
   for (int i = 0; i < num_segments; i++)
   {

    int from_id, to_id;
    if (i==0)
    {// start to first via
        from_id=start_id;
        to_id=via_ids[0];

        /* code */
    }
    else if (i==num_segments-1)
    {//last via to goal buiding
        from_id=via_ids[num_via-1];
        to_id=goal_id;
        /* code */
    }
    else
    {
        from_id=via_ids[i-1];
        to_id=via_ids[i];
    }
    //show segment computation progress
    int thread_id= omp_get_thread_num();
    printf(color_parallel"[Thread %d] Segment %d: %d->%d\n"
    color_reset,thread_id,i,from_id,to_id);
    /* code */
    // use A* for each segment
    segment_dists[i]=astar(graph,from_id,to_id,
        &segment_paths[i],&segment_lens[i]);
        if (segment_dists[i]!=DBL_MAX)
        {
            //greeen= success
            printf(color_success"[Thread %d] ok Segment %d complete: %.2f m\n" color_reset, 
                   thread_id, i, segment_dists[i]);
        } else {
            //red = failure
            printf(color_error "[Thread %d] sad Segment %d failed: No path\n" color_reset, 
                   thread_id, i);
            /* code */
        }
        // yellow parallel finished

   }
   
   printf(color_parallel"parallel finished! please check \n" color_reset);

   int all_success=1;
   double total_distance=0;
   // check each segment
   for (int i = 0; i < num_segments; i++)
   {
    if (segment_dists[i]==DBL_MAX||segment_paths[i]==NULL)
    {
        printf(color_error"segement %d failed" color_reset, i);
        all_success=0;
        break;
        /* code */
    }
    total_distance=total_distance+segment_dists[i];
    /* code */
   }
   // if failed, clean and report error
   if (! all_success)
   {
    printf(color_error"clean \n" color_reset);
    for (int i = 0; i < num_segments; i++)
    {if (segment_paths[i])
    {free(segment_paths[i]);
        /* code */
    }
    
        /* code */
    }
    // free container arrays
    free(segment_dists);
    free(segment_lens);
    free(segment_paths);

    *out_len=0;
    *out_path=NULL;
    return DBL_MAX;
    /* code */
   }
   // merge segments
   // a b +b d = a b d
   printf(color_success" merge segment\n" color_reset);

   int total_len=0;
   for (int i = 0; i < num_segments; i++)
   {
    total_len+=segment_lens[i];
    if(i>0)
    {total_len--;}
    /* code */
   }
   // allocate merged path
   int*merged_path= malloc(sizeof(int)*total_len);
   int merge_index=0;
   for (int i = 0; i < num_segments; i++)
   {int start_copu;
    // start copy for first or skip this one
    //  just like a b +b d = a b d, the second b is skipped
      if (i==0)

      {start_copu=0;
        /* code */
      }else{start_copu=1;}
      for (int k = start_copu; k <segment_lens[i]; k++)
      {merged_path[merge_index]=segment_paths[i][k];
        merge_index++;
        /* code */
      }
      
    /* code */
   }
   
// clean memory
for (int i = 0; i < num_segments; i++)
    
    {free(segment_paths[i]);
        /* code */
    
    
        /* code */
    }
    // free container arrays
    free(segment_dists);
    free(segment_lens);
    free(segment_paths);
*out_len=total_len;
*out_path=merged_path;
// print what you want
printf(color_success"total distance:%.2f \n" color_reset, total_distance);
printf(color_success"total nodes %d \n" color_reset, total_len);
return total_distance;

}