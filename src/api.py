import csv
import subprocess
import json
from flask import Flask, request, jsonify, send_from_directory
import os  # Import os for reliable path handling


# --- Path 1: Frontend Folder ---
# __file__ is .../src/api.py
# We need to go up one level (..) to find 'frontend/'
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
        # --- Path 2: CSV File ---
        # From .../src/ go up (..) to 'data/'
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

# --- API 2: Navigate ---


@app.route("/api/navigate")
def get_navigation():
    """
    Call the C executable at ./main (in the same folder as api.py)
    """
    start_building = request.args.get('start')
    goal_building = request.args.get('end')

    if not start_building or not goal_building:
        return jsonify({"error": "Missing 'start' or 'end' parameter"}), 400

    try:
        # --- Path 3: C Executable (Corrected) ---
        # Assume the 'main' executable is in 'src/' alongside 'api.py'
        # os.path.dirname(__file__) gets the 'src/' directory
        # executable_path = os.path.join(os.path.dirname(__file__), 'main')
        executable_path = EXECUTABLE_PATH

        #print(f"Executing pathfinding: {executable_path} \"{start_building}\" \"{goal_building}\"")
        #print(f"Arg types: start_building={type(start_building)}, goal_building={type(goal_building)}")
        process = subprocess.run(
            [executable_path, f"{start_building}", f"{goal_building}"],
            capture_output=True,
            text=True,
            check=False
        )

        #print(f"Process Return Code: {process.returncode}")

        if process.returncode != 0:
            # print("C program error output:", process.stderr)
            return jsonify({"status": "error", "message": process.stderr}), 500

        stdout = process.stdout
        # 正則抽出 JSON
        import re
        match = re.search(r'(\{.*\})', stdout, re.DOTALL)
        if match:
            json_str = match.group(1)
            response_data = json.loads(json_str)
            return jsonify(response_data)
        else:
            return jsonify({"error": "No JSON found in C program output", "stdout": stdout}), 500

    except FileNotFoundError:
        return jsonify({"error": "Pathfinding executable './main' not found in 'src/' folder. Did you run the compile command inside 'src/'?"}), 500
    except json.JSONDecodeError as e:
        return jsonify({"error": "Failed to parse C program output", "details": stdout}), 500
    except Exception as e:
        # print("Unexpected error:", str(e))
        return jsonify({"error": str(e)}), 500

# --- Static File Server ---


@app.route("/")
def serve_index():
    # Serve index.html from '../frontend'
    return send_from_directory(app.static_folder, 'index.html')


@app.route("/<path:path>")
def serve_static(path):
    # Serve files like 'script.js' and 'style.css'
    # (Assuming your JS/CSS files are directly under frontend/)
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
