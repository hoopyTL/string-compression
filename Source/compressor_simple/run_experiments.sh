#!/usr/bin/env bash
# run_experiments.sh
#
# Generates test files for Scenario 1 (file size) and Scenario 2 (data
# entropy), runs the compressor over every file x every algorithm, and
# writes everything into results.csv for your report tables/charts.
#
# Usage: place this script in the same folder as your compiled compressor
# (or the .cpp/.h source files, and it will compile them for you), then:
#   chmod +x run_experiments.sh
#   ./run_experiments.sh

set -x

# ---------------------------------------------------------------------------
# Config - tweak these if you want different sizes / repeat counts
# ---------------------------------------------------------------------------
NUM_RUNS=3                                  # times to repeat each compress for averaging
ALGOS=(rle huff lzw)
SIZES_KB=(10 100 500 1024 5120 10240)       # Scenario 1 sizes: 10KB .. 10MB
SCENARIO2_SIZE_KB=1024                      # Scenario 2 fixed size: 1MB

DATA_DIR="bench_data"
RESULTS_CSV="results.csv"

# ---------------------------------------------------------------------------
# Find or build the compressor binary
# ---------------------------------------------------------------------------
EXT=""
if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]]; then
    EXT=".exe"
fi
COMPRESSOR="./compressor${EXT}"

echo "Compiling compressor${EXT} "
g++ *.cpp -o "compressor${EXT}" -std=c++17

mkdir -p "$DATA_DIR"

# ---------------------------------------------------------------------------
# Data generators
# ---------------------------------------------------------------------------

# repetitive: a short pattern repeated to fill the requested number of bytes
gen_repetitive() {
    local outFile="$1"
    local sizeBytes="$2"
    local pattern="AAAAABBBBBCCCCCDDDDDEEEEE"
    yes "$pattern" 2>/dev/null | tr -d '\n' | head -c "$sizeBytes" > "$outFile"
}

# random: true random bytes (worst case for all three algorithms)
gen_random() {
    local outFile="$1"
    local sizeBytes="$2"
    head -c "$sizeBytes" /dev/urandom > "$outFile"
}

# Offline fallback used only if downloading a real book fails (e.g. no
# internet access). Produces readable, non-repeating-ish pseudo-English by
# picking random words from a small list. Less realistic than a real book,
# but keeps the experiment runnable without network access.
gen_fallback_english() {
    local outFile="$1"
    local sizeBytes="$2"
    local words=(the quick brown fox jumps over lazy dog while forest
                  river flows quietly beneath old stone bridge every
                  morning sun rises slowly above distant mountains
                  travelers walk along winding roads searching for
                  answers hidden within ancient books written long ago
                  science teaches us patience and careful observation
                  algorithms describe precise steps to solve problems)
    : > "$outFile"
    while [ "$(wc -c < "$outFile")" -lt "$sizeBytes" ]; do
        line=""
        for i in $(seq 1 12); do
            line="$line ${words[$RANDOM % ${#words[@]}]}"
        done
        echo "$line." >> "$outFile"
    done
    if ! truncate -s "$sizeBytes" "$outFile" 2>/dev/null; then
        head -c "$sizeBytes" "$outFile" > "${outFile}.tmp"
        mv "${outFile}.tmp" "$outFile"
    fi
}

# Downloads a few different public-domain books from Project Gutenberg and
# concatenates them (distinct content first, so we don't create artificial
# repetition until we genuinely run out of source text).
build_english_source() {
    local target="$DATA_DIR/english_source.txt"
    local maxSizeBytes=$1
    if [ -s "$target" ] && [ "$(wc -c < "$target")" -ge "$maxSizeBytes" ]; then
        return
    fi

    echo "Building English text source (~$((maxSizeBytes / 1024 / 1024)) MB)..."
    : > "$target"
    local ids=(1342 84 11 2701 98)  # Pride and Prejudice, Frankenstein, Alice, Moby Dick, A Tale of Two Cities
    local downloaded=0

    if command -v curl >/dev/null 2>&1; then
        for id in "${ids[@]}"; do
            [ "$(wc -c < "$target")" -ge "$maxSizeBytes" ] && break
            if curl -fsSL --connect-timeout 5 --max-time 20 "https://www.gutenberg.org/cache/epub/${id}/pg${id}.txt" >> "$target" 2>/dev/null; then
                downloaded=1
                echo "  downloaded book id $id"
            fi
        done
    fi

    if [ "$downloaded" -eq 0 ] || [ ! -s "$target" ]; then
        echo "  no internet access detected - using offline fallback text generator instead"
        gen_fallback_english "$target" "$maxSizeBytes"
    fi

    # If downloaded text is still shorter than needed, repeat it to fill the gap
    while [ "$(wc -c < "$target")" -lt "$maxSizeBytes" ]; do
        cat "$target" "$target" > "${target}.tmp" && mv "${target}.tmp" "$target"
    done
}

# ---------------------------------------------------------------------------
# Benchmark runner
# ---------------------------------------------------------------------------

# Runs one (file, algorithm) pair NUM_RUNS times, averages execution time,
# verifies the round trip, and appends one row to the CSV.
run_case() {
    local scenario="$1"
    local dataType="$2"
    local inFile="$3"
    local algo="$4"

    local origSize compSize ratio savings
    local times=()
    local compFile="${inFile}.${algo}"
    local restoredFile="${inFile}.${algo}.restored"

    local sizeLabel
    sizeLabel=$(wc -c < "$inFile" 2>/dev/null || echo "?")
    if [ "$algo" == "lzw" ] && [ "$sizeLabel" -gt 1000000 ] 2>/dev/null; then
        echo "  running $algo on $(basename "$inFile") ($NUM_RUNS runs, LZW is the slow one on large files - this can take a while)..."
    else
        echo "  running $algo on $(basename "$inFile") ($NUM_RUNS runs)..."
    fi

    for i in $(seq 1 "$NUM_RUNS"); do
        output=$("$COMPRESSOR" -a "$algo" -m c -i "$inFile" -o "$compFile")
        t=$(echo "$output" | grep "Execution Time" | awk '{print $3}')
        times+=("$t")
        origSize=$(echo "$output" | grep "Original Size" | awk '{print $3}')
        compSize=$(echo "$output" | grep "Compressed Size" | awk '{print $3}')
        ratio=$(echo "$output" | grep "Compression Ratio" | awk '{print $3}')
        savings=$(echo "$output" | grep "Space Savings" | awk '{print $3}' | tr -d '%')
    done

    avgTime=$(printf '%s\n' "${times[@]}" | awk '{sum+=$1; n++} END {if (n>0) printf "%.4f", sum/n; else print 0}')

    # verify round trip once
    "$COMPRESSOR" -a "$algo" -m d -i "$compFile" -o "$restoredFile" > /dev/null
    if cmp -s "$inFile" "$restoredFile"; then
        verify="PASS"
    else
        verify="FAIL"
    fi

    echo "$scenario,$dataType,$(basename "$inFile"),$origSize,$algo,$avgTime,$compSize,$ratio,$savings,$verify" >> "$RESULTS_CSV"
    echo "  [$scenario] $(basename "$inFile") x $algo -> avg ${avgTime} ms, ratio ${ratio}, verify=$verify"

    rm -f "$compFile" "$restoredFile"
}

# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

echo "scenario,data_type,file,original_size_bytes,algorithm,avg_time_ms,compressed_size_bytes,compression_ratio,space_savings_percent,verify" > "$RESULTS_CSV"

maxSizeKB=${SIZES_KB[-1]}
if [ "$SCENARIO2_SIZE_KB" -gt "$maxSizeKB" ]; then maxSizeKB=$SCENARIO2_SIZE_KB; fi
build_english_source $(( maxSizeKB * 1024 ))

echo ""
echo "=== Scenario 1: Impact of File Size (data type = English text) ==="
for kb in "${SIZES_KB[@]}"; do
    bytes=$((kb * 1024))
    f="$DATA_DIR/english_${kb}kb.txt"
    head -c "$bytes" "$DATA_DIR/english_source.txt" > "$f"
    for algo in "${ALGOS[@]}"; do
        run_case "size" "english" "$f" "$algo"
    done
done

echo ""
echo "=== Scenario 2: Impact of Data Entropy (fixed size = ${SCENARIO2_SIZE_KB}KB) ==="
bytes=$((SCENARIO2_SIZE_KB * 1024))

repFile="$DATA_DIR/repetitive_${SCENARIO2_SIZE_KB}kb.bin"
gen_repetitive "$repFile" "$bytes"

engFile="$DATA_DIR/english_${SCENARIO2_SIZE_KB}kb.txt"
head -c "$bytes" "$DATA_DIR/english_source.txt" > "$engFile"

randFile="$DATA_DIR/random_${SCENARIO2_SIZE_KB}kb.bin"
gen_random "$randFile" "$bytes"

for algo in "${ALGOS[@]}"; do run_case "entropy" "repetitive" "$repFile" "$algo"; done
for algo in "${ALGOS[@]}"; do run_case "entropy" "english" "$engFile" "$algo"; done
for algo in "${ALGOS[@]}"; do run_case "entropy" "random" "$randFile" "$algo"; done

echo ""
echo "Done. Results written to $RESULTS_CSV"
