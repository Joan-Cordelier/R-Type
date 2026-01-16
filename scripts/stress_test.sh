#!/bin/bash

# Stress test script for R-Type client connections
# Usage: ./stress_test.sh [num_clients] [delay_between_starts]

NUM_CLIENTS=${1:-5}
DELAY=${2:-1}

echo "Starting $NUM_CLIENTS clients with $DELAY second delay between each..."

PIDS=()

for i in $(seq 1 $NUM_CLIENTS); do
    echo "Starting client $i..."
    SDL_AUDIODRIVER=dummy ./build/r-type_client &
    PIDS+=($!)
    sleep $DELAY
done

echo ""
echo "All $NUM_CLIENTS clients started."
echo "PIDs: ${PIDS[*]}"
echo ""
echo "Press Enter to kill all clients..."
read

echo "Killing all clients..."
for pid in "${PIDS[@]}"; do
    kill $pid 2>/dev/null
done

echo "Done."
