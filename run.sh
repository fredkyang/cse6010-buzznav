#!/bin/bash

# ===== GLOBAL CONFIGURATION =====
PROJECT_PATH="/mnt/c/Users/user/Desktop/GT 6010 Final Proj/cse6010-buzznav"
LOG_FILE="${PROJECT_PATH}/logs/buzznav_$(date +%Y%m%d_%H%M).log"
mkdir -p "$(dirname "$LOG_FILE")"

# ===== Setup Logging =====
exec > >(tee -a "$LOG_FILE") 2>&1

echo "=========================================="
echo "BuzzNav - $(date)"
echo "=========================================="

# ===== Change to Project Directory =====
cd "$PROJECT_PATH" || { echo "ERROR: Failed to cd to $PROJECT_PATH"; exit 1; }
echo "Project directory: $(pwd)"

# ===== Clean and Build =====
echo ""
echo "===== Building C Backend ====="
make clean
make || { echo "ERROR: Build failed"; exit 1; }
echo "✓ Build successful"

# ===== Check Python Environment =====
echo ""
echo "===== Checking Python Environment ====="
if ! command -v python3 &> /dev/null && ! command -v python &> /dev/null; then
    echo "ERROR: Python not found. Please install Python 3."
    exit 1
fi

PYTHON_CMD=$(command -v python3 || command -v python)
echo "Using Python: $PYTHON_CMD"

# Check if Flask is installed
if ! $PYTHON_CMD -c "import flask" 2>/dev/null; then
    echo "WARNING: Flask not installed. Installing..."
    pip install flask || { echo "ERROR: Failed to install Flask"; exit 1; }
fi
echo "✓ Flask is available"

# ===== Start Flask Server =====
echo ""
echo "===== Starting Flask Server ====="
cd "${PROJECT_PATH}/src" || { echo "ERROR: Failed to cd to src"; exit 1; }

echo ""
echo "=========================================="
echo "BuzzNav is starting!"
echo "Open your browser to: http://127.0.0.1:5000"
echo "Press Ctrl+C to stop the server"
echo "=========================================="
echo ""

# Run Flask server (this blocks until Ctrl+C)
$PYTHON_CMD api.py

# ===== Cleanup on Exit =====
echo ""
echo "=========================================="
echo "BuzzNav stopped at $(date)"
echo "Log saved to: $LOG_FILE"
echo "=========================================="