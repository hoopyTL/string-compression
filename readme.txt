========================================================================
STRING COMPRESSION TOOL - USER GUIDE & DOCUMENTATION
Course: CSC10004 - Data Structures & Algorithms
Department of Knowledge Engineering - FIT, HCMUS
========================================================================

1. PROJECT OVERVIEW
------------------------------------------------------------------------
This project provides a robust, cross-platform command-line utility for 
lossless string and file compression. It implements three fundamental 
compression algorithms:
  - Run-Length Encoding (RLE)
  - Huffman Coding (Greedy Min-Heap Tree Construction)
  - Lempel-Ziv-Welch (LZW) Dictionary-based Compression

All source files are strictly written in standard C++17 without any 
external/third-party dependencies.

------------------------------------------------------------------------
2. DIRECTORY STRUCTURE
------------------------------------------------------------------------
.
├── Source/
│   └── compressor_simple/
│       ├── main.cpp              : CLI entry point and performance metrics
│       ├── utils.h / utils.cpp   : Binary I/O utilities and helper functions
│       ├── rle.h / rle.cpp       : Run-Length Encoding implementation
│       ├── huffman.h / huffman.cpp: Huffman Coding implementation
│       ├── lzw.h / lzw.cpp       : LZW Compression implementation
│       ├── bench_data/           : Benchmark dataset for experimental scenarios
│       ├── run_experiments.sh    : Automated benchmarking & verification script
│       ├── results.csv           : Benchmark measurement records
│       └── readme.txt            : User guide
├── docs/                         : Project documentation & reports
├── README.md                     : Repository overview
└── readme.txt                    : User guide and documentation

------------------------------------------------------------------------
3. COMPILATION INSTRUCTIONS
------------------------------------------------------------------------
The project must be compiled using a GNU g++ (or Clang) compiler supporting 
the C++17 standard.

Navigate into `Source/compressor_simple/`:
    cd Source/compressor_simple

[Linux / macOS / Git Bash]:
    g++ *.cpp -o compressor -std=c++17

[Windows (MinGW / MSYS2 / Command Prompt)]:
    g++ *.cpp -o compressor.exe -std=c++17

------------------------------------------------------------------------
4. USAGE & COMMAND-LINE SYNTAX
------------------------------------------------------------------------
Syntax:
    ./compressor -a [algorithm] -m [mode] -i [input_file] -o [output_file]

Arguments:
    -a [algorithm] : Choose compression algorithm:
                     * rle  : Run-Length Encoding
                     * huff : Huffman Coding
                     * lzw  : Lempel-Ziv-Welch
    -m [mode]      : Choose operational mode:
                     * c    : Compression mode
                     * d    : Decompression mode
    -i [input_file]: Path to the source/input file.
    -o [output_file]: Path to write the output result.

------------------------------------------------------------------------
5. EXAMPLES
------------------------------------------------------------------------
[Example 1: Compressing with RLE]
    ./compressor -a rle -m c -i bench_data/english_10kb.txt -o eng.rle

[Example 2: Decompressing with RLE]
    ./compressor -a rle -m d -i eng.rle -o eng_decomp.txt

[Example 3: Compressing with Huffman Coding]
    ./compressor -a huff -m c -i bench_data/english_1024kb.txt -o english.huff

[Example 4: Decompressing with Huffman Coding]
    ./compressor -a huff -m d -i english.huff -o english_restored.txt

[Example 5: Compressing with LZW]
    ./compressor -a lzw -m c -i bench_data/repetitive_1024kb.bin -o rep.lzw

[Example 6: Decompressing with LZW]
    ./compressor -a lzw -m d -i rep.lzw -o rep_restored.bin

------------------------------------------------------------------------
6. RUNNING AUTOMATED EXPERIMENTS & VERIFICATION
------------------------------------------------------------------------
To run the automated benchmark script across all test datasets:
    cd Source/compressor_simple
    chmod +x run_experiments.sh
    ./run_experiments.sh

The script benchmarks all algorithms and verifies 100% lossless output.
========================================================================
