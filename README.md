# Georgia Tech Campus Navigation System

Campus routing prototype for CSE6010. We build a directed, weighted graph from OpenStreetMap data, run A* (with optional via points), and emit the path for quick visualization.

## Features
- Shortest-path search with A* on campus road/building graph
- Optional via points (multiple intermediate stops) computed in parallel by segment
- Outputs node path to terminal and `temp/path_output.txt`
- Python helper to plot the resulting path over campus nodes

## Layout
- `src/` C sources (`main.c`, `astar.c`, `via_point.c`, etc.)
- `include/` headers
- `data/` graph inputs (`adj_list.csv`, `building_mapping.csv`, `node_coordinates.csv`)
- `temp/` path output target (`path_output.txt`)
- `visualization/` Python plotting script

## Build
- Requires a C compiler with OpenMP (e.g., clang with `-fopenmp` or gcc) and POSIX libs.
- Typical build via `make` (see `Makefile`). Example: `make all` then `./bin/buzznav`.

## Running
Quick start:

```sh
make            # build
./bin/main -h   # see CLI usage
./bin/main "<start>" [via1 via2 ...] "<goal>"   # run
```

Examples:
```sh
./bin/main "Student Center" "College of Computing Building"
./bin/main "Student Center" "Tech Tower" "College of Computing Building"
```

- Arguments are ordered start, optional vias, then goal.
- Building names must match `data/building_mapping.csv` exactly.
- `-h` or `--help` prints usage and exits.

## Via points
When vias are provided, the route is solved segment-by-segment in parallel: start → via1 → via2 → … → goal. If any segment is unreachable or a building name is missing, the program exits with an error.

## Visualization
After a run, `temp/path_output.txt` holds the node sequence. Plot it with:

```sh
cd visualization
python main.py
```

This reads `data/node_coordinates.csv` and overlays the path on the campus nodes.

## Data assumptions
- `adj_list.csv`: directed edges with distances (meters) as weights.
- `building_mapping.csv`: building name → node id lookup.
- `node_coordinates.csv`: node id → (lon, lat) for haversine heuristic and plotting.
