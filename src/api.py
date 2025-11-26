import subprocess
import json
import os
import csv
from flask import Flask, request, jsonify, send_from_directory

app = Flask(__name__, static_folder=os.path.join(os.path.dirname(__file__), '..', 'frontend'), static_url_path='')
EXECUTABLE_PATH = os.path.join(os.path.dirname(__file__), '..', 'bin', 'main')

# ==========================================
# 1. Centralized Response Handler
# ==========================================
class ResponseHandler:
    """
    Centralizes all API responses to ensure consistency.
    """
    
    @staticmethod
    def success(data):
        """Standard success response (HTTP 200)."""
        # If the data itself contains a status field (from C), we can leave it or override it.
        # Here we trust the data payload but ensure it's wrapped cleanly if needed.
        return jsonify(data), 200

    @staticmethod
    def error(message, details=None, status_code=500):
        """Standard error response (HTTP 4xx/5xx)."""
        payload = {
            "status": "error",
            "message": message
        }
        if details:
            payload["details"] = details
        return jsonify(payload), status_code

    @staticmethod
    def parse_c_output(stdout_str, stderr_str):
        """
        Parses raw string output from C. 
        Returns (data_dict, is_success, error_msg).
        """
        if not stdout_str or not stdout_str.strip():
            return None, False, "C program returned empty output."

        try:
            # 1. Attempt Clean Parse
            data = json.loads(stdout_str)
        except json.JSONDecodeError:
            # 2. Attempt "Fuzzy" Parse (extract JSON from mixed logs)
            try:
                start = stdout_str.find('{')
                end = stdout_str.rfind('}')
                if start != -1 and end != -1:
                    data = json.loads(stdout_str[start:end+1])
                else:
                    return None, False, "No JSON object found in output."
            except Exception:
                return None, False, f"Failed to parse JSON: {stdout_str[:100]}..."

        # 3. Check Logic Error from C (e.g. {"status": "error", ...})
        if data.get("status") == "error":
            msg = data.get("message", "Unknown error from C backend")
            return None, False, msg

        return data, True, None

# ==========================================
# 2. C Execution Helper
# ==========================================
def execute_backend(args):
    """
    Helper to run the C executable and return a Flask Response immediately.
    """
    if not os.path.exists(EXECUTABLE_PATH):
        return ResponseHandler.error(
            f"Executable not found", 
            details=f"Expected at: {EXECUTABLE_PATH}",
            status_code=500
        )

    try:
        # Run C Program
        process = subprocess.run(
            [EXECUTABLE_PATH] + args,
            capture_output=True,
            text=True
        )

        # Log C STDERR to Python Console (for Developer Debugging)
        if process.stderr:
            print(f"--- [C LOGS] ---\n{process.stderr}\n----------------")

        # Parse Output using Centralized Logic
        data, is_success, err_msg = ResponseHandler.parse_c_output(process.stdout, process.stderr)

        if is_success:
            return ResponseHandler.success(data)
        else:
            # If it's a logic error (e.g. building not found), usually 400 is better than 500
            # You can decide to return 400 or 500 based on the message content if you want.
            return ResponseHandler.error(err_msg, status_code=400)

    except Exception as e:
        return ResponseHandler.error(f"Server Execution Error: {str(e)}", status_code=500)


# ==========================================
# 3. Clean API Routes
# ==========================================

@app.route("/api/buildings")
def get_buildings():
    try:
        buildings = []
        csv_path = os.path.join(os.path.dirname(__file__), '..', 'data', 'building_mapping.csv')
        
        if not os.path.exists(csv_path):
            return ResponseHandler.error("Database file (CSV) not found", status_code=404)

        with open(csv_path, mode='r', encoding='utf-8') as f:
            reader = csv.DictReader(f)
            for row in reader:
                buildings.append(row['building_name'])
        
        return ResponseHandler.success(buildings)
    except Exception as e:
        return ResponseHandler.error(str(e), status_code=500)

@app.route("/api/navigate")
def get_navigation():
    # 1. Extract
    start = request.args.get('start')
    end = request.args.get('end')
    via_points = request.args.getlist('via')

    # 2. Validate
    if not start or not end:
        return ResponseHandler.error("Missing required parameters: 'start' and 'end'", status_code=400)

    # 3. Clean
    clean_via = [v for v in via_points if v.strip()]
    
    # 4. Execute
    cmd_args = [start] + clean_via + [end]
    return execute_backend(cmd_args)

@app.route("/api/navigate-tsp")
def get_navigation_tsp():
    # 1. Extract
    buildings = []
    i = 1
    while True:
        b = request.args.get(f'building{i}')
        if not b: break
        buildings.append(b.strip())
        i += 1
    
    # 2. Validate
    if len(buildings) < 2:
        return ResponseHandler.error("TSP requires at least 2 buildings", status_code=400)
    
    # 3. Execute
    cmd_args = ["--tsp"] + buildings
    return execute_backend(cmd_args)


# ==========================================
# 4. Static Files
# ==========================================

@app.route("/")
def serve_index():
    return send_from_directory(app.static_folder, 'index.html')

@app.route("/<path:path>")
def serve_static(path):
    if path.endswith(('.js', '.css', '.html')):
        return send_from_directory(app.static_folder, path)
    return ResponseHandler.error("File not found", status_code=404)

if __name__ == '__main__':
    print("===== Python API Server Running =====")
    print(f"Expected Executable: {EXECUTABLE_PATH}")
    app.run(debug=True, port=5000)