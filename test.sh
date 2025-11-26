#!/bin/bash

# ===== GLOBAL CONFIGURATION =====
PROJECT_PATH="/mnt/c/Users/user/Desktop/GT 6010 Final Proj/cse6010-buzznav"
LOG_FILE="${PROJECT_PATH}/logs/tsp_test_$(date +%Y%m%d_%H%M).log"
mkdir -p "$(dirname "$LOG_FILE")"

# ===== Setup Logging =====
exec > >(tee -a "$LOG_FILE") 2>&1

echo "=========================================="
echo "TSP Test Run - $(date)"
echo "=========================================="

# ===== Save Original Directory =====
ORIG_DIR=$(pwd)
echo "Original directory: $ORIG_DIR"

# ===== Clean and Build =====
echo ""
echo "===== Building Project ====="
cd "$PROJECT_PATH" || { echo "Failed to cd to $PROJECT_PATH"; exit 1; }
echo "Current directory: $(pwd)"
make clean
make || { echo "Build failed"; cd "$ORIG_DIR"; exit 1; }

# ===== Run TSP Test =====
echo ""
echo "===== Running TSP Test ====="
cd "${PROJECT_PATH}/bin" || { echo "Failed to cd to bin"; cd "$ORIG_DIR"; exit 1; }

./main --tsp "Sideways" "Wreck Techs" "AT&T"

# ===== Return to Original Directory =====
echo ""
echo "===== Returning to Original Directory ====="
cd "$ORIG_DIR"
echo "Back to: $(pwd)"

echo ""
echo "=========================================="
echo "Test completed at $(date)"
echo "Log saved to: $LOG_FILE"
echo "=========================================="