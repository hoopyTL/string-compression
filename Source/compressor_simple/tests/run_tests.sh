#!/usr/bin/env bash
# run_tests.sh - Automated functional test runner for lossless compression
# Verifies that decompression(compression(data)) == original_data for all test cases.

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Find directory containing main.cpp
if [ -f "$SCRIPT_DIR/../main.cpp" ]; then
    SRC_DIR="$SCRIPT_DIR/.."
elif [ -f "$SCRIPT_DIR/../Source/compressor_simple/main.cpp" ]; then
    SRC_DIR="$SCRIPT_DIR/../Source/compressor_simple"
else
    SRC_DIR="$SCRIPT_DIR"
fi

EXT=""
if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]]; then
    EXT=".exe"
fi
COMPRESSOR="$SRC_DIR/compressor${EXT}"

echo "========================================================================"
echo "          LOSSLESS COMPRESSION CORRECTNESS TEST SUITE                  "
echo "========================================================================"

# Compile if binary not found
if [ ! -f "$COMPRESSOR" ]; then
    echo "[*] Compiling compressor..."
    g++ "$SRC_DIR"/*.cpp -o "$COMPRESSOR" -std=c++17
fi

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "$TMP_DIR"' EXIT

ALGOS=("rle" "huff" "lzw")
TOTAL=0
PASSED=0
FAILED=0

echo ""
printf "%-32s | %-6s | %-12s | %-12s | %-8s\n" "Test Case" "Algo" "Orig (B)" "Comp (B)" "Status"
echo "---------------------------------+--------+--------------+--------------+--------"

for test_file in "$SCRIPT_DIR"/test_*; do
    [ -f "$test_file" ] || continue
    test_name="$(basename "$test_file")"
    orig_size=$(wc -c < "$test_file" | tr -d ' ')

    for algo in "${ALGOS[@]}"; do
        TOTAL=$((TOTAL + 1))
        comp_file="$TMP_DIR/${test_name}.${algo}.comp"
        decomp_file="$TMP_DIR/${test_name}.${algo}.decomp"

        # Compress
        "$COMPRESSOR" -a "$algo" -m c -i "$test_file" -o "$comp_file" > /dev/null 2>&1 || true

        # Decompress
        "$COMPRESSOR" -a "$algo" -m d -i "$comp_file" -o "$decomp_file" > /dev/null 2>&1 || true

        comp_size=0
        if [ -f "$comp_file" ]; then
            comp_size=$(wc -c < "$comp_file" | tr -d ' ')
        fi

        # Verify lossless restoration
        if cmp -s "$test_file" "$decomp_file"; then
            PASSED=$((PASSED + 1))
            status="PASS"
        else
            FAILED=$((FAILED + 1))
            status="FAIL"
        fi

        printf "%-32s | %-6s | %12s | %12s | %-8s\n" "$test_name" "$algo" "$orig_size" "$comp_size" "$status"
    done
done

echo "========================================================================"
echo "TEST SUMMARY: $PASSED / $TOTAL tests passed."
if [ "$FAILED" -eq 0 ]; then
    echo "RESULT: ALL TESTS PASSED SUCCESSFULLY! (100% Lossless Invariant Maintained)"
    echo "========================================================================"
    exit 0
else
    echo "RESULT: $FAILED TESTS FAILED!"
    echo "========================================================================"
    exit 1
fi
