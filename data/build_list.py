import osmnx as ox
import numpy as np
import networkx as nx
from shapely.geometry import LineString

# ========== 1. Download campus road network ==========
place = "Georgia Institute of Technology, Atlanta, GA"
G = ox.graph_from_place(place, network_type="drive")

# ========== 2. Helper functions ==========
def haversine(lat1, lon1, lat2, lon2):
    """Great-circle distance between two points in meters"""
    from math import radians, sin, cos, atan2, sqrt
    R = 6371000
    dlat = radians(lat2 - lat1)
    dlon = radians(lon2 - lon1)
    a = sin(dlat/2)**2 + cos(radians(lat1)) * cos(radians(lat2)) * sin(dlon/2)**2
    c = 2 * atan2(sqrt(a), sqrt(1 - a))
    return R * c

def closest_point_on_segment(x1, y1, x2, y2, px, py):
    """Return closest point from (px,py) to line segment (x1,y1)-(x2,y2)."""
    dx, dy = x2 - x1, y2 - y1
    if dx == 0 and dy == 0:
        return x1, y1
    t = ((px - x1) * dx + (py - y1) * dy) / (dx*dx + dy*dy)
    t = max(0, min(1, t))  
    return x1 + t * dx, y1 + t * dy

def split_curved_edges(G):
    """Split curved edges with geometry into straight line segments"""
    new_edges = []
    edges_to_remove = []
    for u, v, k, data in list(G.edges(keys=True, data=True)):
        geom = data.get("geometry", None)
        if isinstance(geom, LineString) and len(geom.coords) > 2:
            coords = list(geom.coords)
            edges_to_remove.append((u, v, k))
            prev_node = u
            for i in range(1, len(coords) - 1):
                new_node = max(G.nodes) + 1
                x, y = coords[i]
                G.add_node(new_node, x=x, y=y, type="road")
                length = haversine(G.nodes[prev_node]["y"], G.nodes[prev_node]["x"], y, x)
                new_edges.append((prev_node, new_node, {"length": length}))
                prev_node = new_node
            x_v, y_v = coords[-1]
            length = haversine(G.nodes[prev_node]["y"], G.nodes[prev_node]["x"], y_v, x_v)
            new_edges.append((prev_node, v, {"length": length}))
    for (u, v, k) in edges_to_remove:
        G.remove_edge(u, v, k)
    for u, v, data in new_edges:
        G.add_edge(u, v, **data)
    return G

def find_nearest_edge_by_projection(G, px, py):
    """Find nearest edge based on true projection distance (point-to-segment)."""
    min_dist = float("inf")
    best_u, best_v = None, None
    best_proj = None

    for u, v, data in G.edges(data=True):
        x_u, y_u = G.nodes[u]["x"], G.nodes[u]["y"]
        x_v, y_v = G.nodes[v]["x"], G.nodes[v]["y"]
        proj_x, proj_y = closest_point_on_segment(x_u, y_u, x_v, y_v, px, py)
        d = np.hypot(px - proj_x, py - proj_y)
        if d < min_dist:
            min_dist = d
            best_u, best_v = u, v
            best_proj = (proj_x, proj_y)
    return best_u, best_v, best_proj, min_dist

def add_building_projection(G, building_row, building_to_proj, next_id):
    """Project building centroid to nearest road edge using true projection."""
    name = building_row["name"]
    geom = building_row["geometry"]
    centroid = geom.centroid

    # find nearest edge by projection
    u, v, (lon_proj, lat_proj), _ = find_nearest_edge_by_projection(G, centroid.x, centroid.y)

    new_node = next_id
    G.add_node(new_node, x=lon_proj, y=lat_proj, type="road")

    if G.has_edge(u, v):
        G.remove_edge(u, v)

    dist_up = haversine(G.nodes[u]["y"], G.nodes[u]["x"], lat_proj, lon_proj)
    dist_pv = haversine(G.nodes[v]["y"], G.nodes[v]["x"], lat_proj, lon_proj)

    G.add_edge(u, new_node, length=dist_up)
    G.add_edge(new_node, u, length=dist_up)
    G.add_edge(v, new_node, length=dist_pv)
    G.add_edge(new_node, v, length=dist_pv)

    building_to_proj[name] = new_node
    return G, building_to_proj, next_id + 1

def build_adjacency_list(G):
    """Build adjacency list from graph G"""
    adj = {n: [] for n in G.nodes}
    for u, v, data in G.edges(data=True):
        dist = data.get("length", None)
        if dist is None:
            lat1, lon1 = G.nodes[u]["y"], G.nodes[u]["x"]
            lat2, lon2 = G.nodes[v]["y"], G.nodes[v]["x"]
            dist = haversine(lat1, lon1, lat2, lon2)
        adj[u].append((v, dist))
    return adj

# ========== 3. Simplify + Relabel ==========
G = split_curved_edges(G)
G.remove_edges_from(nx.selfloop_edges(G))
mapping = {old: i for i, old in enumerate(G.nodes)}
G = nx.relabel_nodes(G, mapping, copy=True)
next_id = len(G.nodes)

# ========== 4. Download building data ==========
buildings = ox.features_from_place(place, tags={"building": True, "name": True})
buildings = buildings[buildings["name"].notnull()]
#target_buildings = ["Student Center", "Tech Tower"]
#buildings = buildings[buildings["name"].isin(target_buildings)]
buildings = buildings.drop_duplicates(subset=["name"], keep="first")

# ========== 5. Add building projections ==========
building_to_proj = {}
for _, row in buildings.iterrows():
    G, building_to_proj, next_id = add_building_projection(G, row, building_to_proj, next_id)

# ========== 6. Build adjacency list ==========
adj_list = build_adjacency_list(G)

print("Number of nodes:", len(adj_list))
print("Building → Projection mapping:", building_to_proj)
for node, neighbors in list(adj_list.items())[:]:
    print(f"Node {node} neighbors: {neighbors}")
    
import os
import csv

# ========== 7. Export adjacency list to CSV ==========
output_file = "adj_list.csv"

# If file already exists, remove it first
if os.path.exists(output_file):
    os.remove(output_file)
    print(f"Old file '{output_file}' removed.")

# Write adjacency list to CSV
with open(output_file, "w", newline="") as f:
    writer = csv.writer(f)
    writer.writerow(["src", "dst", "length"])  # header row

    edge_count = 0
    for src, neighbors in adj_list.items():
        for dst, dist in neighbors:
            writer.writerow([src, dst, dist])
            edge_count += 1

print(f"Exported {edge_count} edges to '{output_file}' successfully.")

map_output = "building_mapping.csv"
if os.path.exists(map_output):
    os.remove(map_output)
    print(f"Old file '{map_output}' removed.")

with open(map_output, "w", newline="") as f:
    writer = csv.writer(f)
    writer.writerow(["building_name", "node_id"])
    for name, node_id in building_to_proj.items():
        writer.writerow([name, node_id])
print(f"Exported {len(building_to_proj)} building mappings to '{map_output}' successfully.")
