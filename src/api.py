import csv
import subprocess
import json
from flask import Flask, request, jsonify, send_from_directory
import os

frontend_dir = os.path.join(os.path.dirname(__file__), '..', 'frontend')
EXECUTABLE_PATH = os.path.join('..', 'bin', 'main')
app = Flask(__name__, static_folder=frontend_dir, static_url_path='')

# --- API 1: Get All Buildings ---
@app.route("/api/buildings")
def get_buildings():
    """
    Read CSV from ../data/
    """
    try:
        buildings = []
        csv_path = os.path.join(os.path.dirname(
            __file__), '..', 'data', 'building_mapping.csv')
        with open(csv_path, mode='r', encoding='utf-8') as f:
            reader = csv.DictReader(f)
            for row in reader:
                buildings.append(row['building_name'])
        return jsonify(buildings)
    except FileNotFoundError:
        return jsonify({"error": "'../data/building_mapping.csv' not found. Make sure it's in the 'data/' folder."}), 404
    except Exception as e:
        return jsonify({"error": str(e)}), 500

# --- API 2: Navigate with Via Points ---
@app.route("/api/navigate")
def get_navigation():
    """
    Call the C executable with start, optional via points, and end
    URL format: /api/navigate?start=A&via=B&via=C&end=D
    """
    start_building = request.args.get('start')
    end_building = request.args.get('end')
    
    # Get all via points (multiple values with same key)
    via_points = request.args.getlist('via')

    if not start_building or not end_building:
        return jsonify({"error": "Missing 'start' or 'end' parameter"}), 400

    try:
        executable_path = EXECUTABLE_PATH

        # Build command: ./main "start" "via1" "via2" ... "end"
        command = [executable_path, start_building]
        
        # Add via points in order
        for via in via_points:
            if via.strip():  # Only add non-empty via points
                command.append(via)
        
        command.append(end_building)

        # Debug: log the command being executed
        print(f"Executing: {' '.join(command)}")

        process = subprocess.run(
            command,
            capture_output=True,
            text=True,
            check=False
        )

        print(f"Return code: {process.returncode}")
        print(f"STDOUT: {process.stdout}")
        print(f"STDERR: {process.stderr}")

        if process.returncode != 0:
            return jsonify({"status": "error", "message": process.stderr}), 500

        stdout = process.stdout
        
        # Extract JSON from output
        import re
        match = re.search(r'(\{.*\})', stdout, re.DOTALL)
        if match:
            json_str = match.group(1)
            response_data = json.loads(json_str)
            return jsonify(response_data)
        else:
            return jsonify({"error": "No JSON found in C program output", "stdout": stdout}), 500

    except FileNotFoundError:
        return jsonify({"error": f"Pathfinding executable '{EXECUTABLE_PATH}' not found. Did you compile the code?"}), 500
    except json.JSONDecodeError:
        return jsonify({"error": "Failed to parse C program output", "details": stdout}), 500
    except Exception as e:
        print(f"Unexpected error: {str(e)}")
        return jsonify({"error": str(e)}), 500


# --- Static File Server ---
@app.route("/")
def serve_index():
    return send_from_directory(app.static_folder, 'index.html')


@app.route("/<path:path>")
def serve_static(path):
    if path.endswith(('.js', '.css', '.html')):
        return send_from_directory(app.static_folder, path)
    return "Not Found", 404


if __name__ == '__main__':
    print("===== C Navigation API Server (Running from src/) =====")
    print("\n1. Compile your C code at 'CSE6010-buzznav/' directory using Makefile:")
    print("   make clean")
    print("   make")
    print("\n2. Run the server at 'CSE6010-buzznav/src' directory):")
    print("   python api.py")
    print("\n3. Open this URL in your browser:")
    print("   http://127.0.0.1:5000")
    print("==============================================")
    app.run(debug=True, port=5000)