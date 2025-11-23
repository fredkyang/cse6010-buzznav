import pandas as pd
import matplotlib.pyplot as plt

# Load data
coords = pd.read_csv('data/node_coordinates.csv')

with open("./temp/path_output.txt", "r") as f:
    path = list(map(int, f.read().strip().split()))

#path = [int(node.strip()) for node in path.split('->')]
# print(path)

# Match coordinates in order
ordered_path = coords.set_index('node_id').loc[path]

# Plot
plt.figure(figsize=(10, 8))

# Route line
plt.plot(ordered_path['x'], ordered_path['y'], '-', linewidth=2)

# Highlight start and end nodes
plt.scatter(ordered_path['x'].iloc[0], ordered_path['y'].iloc[0], s=80, color='green', label='Start')
plt.scatter(ordered_path['x'].iloc[-1], ordered_path['y'].iloc[-1], s=80, color='red', label='End')

plt.scatter(coords['x'], coords['y'], s=5, color='gray', alpha=0.4)  # background nodes
plt.title("Shortest Path Visualization")
plt.xlabel("X Coordinate")
plt.ylabel("Y Coordinate")
plt.legend()
plt.show()
