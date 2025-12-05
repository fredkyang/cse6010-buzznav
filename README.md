[![C/C++ CI](https://github.com/fredkyang/gt_cse6010_navigation_system/workflows/C/C++%20CI/badge.svg)](https://github.com/fredkyang/gt_cse6010_navigation_system/actions)
[![Language](https://img.shields.io/badge/language-C99-orange.svg)]()
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS-lightgrey.svg)]()
# BuzzNav - Georgia Tech Campus Navigation System

Campus routing prototype for CSE6010. 

We build a directed, weighted graph from OpenStreetMap data, run A*, Dijkstra algorithms (with optional via points) to generate turn-by-turn directions web-based visualization.

## Features
- Shortest-path search with A* on GT campus graph
- Optional via points (multiple intermediate stops) in order or optimized order computed in parallel by segment
- Turn-by-turn directions that include distances, directions, and building-based instructions
- Real-time route visualization with OpenStreetMap

## Project Structure

```
cse6010-buzznav/
├── data/               # Graph data files
│   ├── build_list.py          # Python code for building csv files
│   ├── adj_list.csv           # Road network edges with distances
│   ├── building_mapping.csv   # Building name → node ID mapping
│   └── node_coordinates.csv   # Node coordinates (lat/lon)
├── src/                # Backend codes
│   ├── main.c                 # Entry point, JSON output
│   ├── astar.c                # A* pathfinding algorithm
│   ├── graph.c                # Graph data structures
│   ├── via_point.c            # Multi-point routing
│   ├── tsp.c                  # TSP optimization (Held-Karp)
│   ├── instructions.c         # Turn-by-turn generation
│   ├── utils.c                # Haversine distance, helpers
│   └── api.py                 # Flask API server
├── frontend/           # Web interface
│   ├── index.html             # Main page
│   ├── script.js              # Map and routing logic
│   └── style.css              # UI styling
├── include/            # C headers
├── logs/               # Runtime logs (auto-generated)
├── Makefile            # Build configuration
├── run.sh              # One-command launcher
└── README.md
```

## Quick Start

### Requirements
- C Compiler: GCC or Clang with OpenMP support
- Python 3: With Flask (`pip install flask`)
- Linux, macOS: Recommended for development

### Run the Application
Firstly, modify PROJECT_PATH in run.sh file if needed

```bash
# Make script executable (first time only)
chmod +x run.sh

# Launch everything
./run.sh
```

This will:
1. Compile the C backend
2. Start the Flask server
3. Open http://127.0.0.1:5000 in your browser

Press `Ctrl+C` to stop.
