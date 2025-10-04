# GeorgiaTech-Campus-Navigation-Systems

**🗺️ Georgia Tech Campus Navigation System 🗺️**

Welcome to team 6's GitHub Organization! <br>
This is the home of our Campus Navigation System project — a fun (and hopefully useful) tool that helps students, staff, and visitors easily find their way around campus. 

**What’s this project about?**

We aim to build a navigation system that makes campus routes easy to understand and provide clear and friendly route guidance. 

Use cases: <br>
⏰ Students rushing to class <br>
🏛️ Visitors exploring campus <br>
⚡ Staff finding the fastest path between buildings <br>

**What’s inside?**

This organization has multiple repositories, each focusing on a different piece of the puzzle:

graph-construction → where we turn the campus map into a graph

pathfinding-algorithms → all the shortest path magic happens here

frontend-ui → pretty maps and user-friendly directions

integration → putting everything together into a working system

**Our team**

We’re a group of students collaborating to bring this idea to life:

Member A – Algorithm wizard

Member B – Data wrangler
We construct a road–building adjacency graph for the Georgia Tech campus 
using OpenStreetMap (OSM) data.

- **Roads**: only motor-vehicle-accessible roads are kept.  
- **Buildings**: each building is represented by its geometric centroid, which is projected to the nearest road segment (using point-to-segment distance).  
- **Projection**: the projection point is added to the graph, and the corresponding road edge is split to preserve distances.  
- **Final Output**: an adjacency list **Adj**, where
  - For each node *i*, we store its set of neighbors that are directly connected.  
  - Each neighbor *j* is stored together with the corresponding edge distance *d(i,j)*.  
  - Formally:
    ```
    Adj[i] = { (j, d(i,j)) | j ∈ Neighbors(i) }
    ```
  
This pipeline transforms raw OSM data into a unified road–building graph model,
which can be used for shortest-path search, parking analysis, and other 
network-based computations.


Member C – UI/UX ninja

Member D – Integration guru

Member E – Documentation master (and README writer ✍️)

**Cool features**

🐾 Step-by-step directions with estimated travel time <br>
🛴 Easy-to-follow turn-by-turn instructions <br>
🚌 Future expansion: choose your transportation mode (walk, bike, shuttle!) <br>
