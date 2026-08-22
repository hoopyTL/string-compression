#include <iostream>
#include <string>
#include <ctime>
#include "utils.h"
#include "rle.h"
#include "huffman.h"
#include "lzw.h"

int main(int argc, char* argv[]) {
    std::string algorithm = "";
    std::string mode = "";
    std::string inputPath = "";
    std::string outputPath = "";

    // read the command line flags: -a -m -i -o
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-a" && i + 1 < argc) {
            algorithm = argv[i + 1];
            i = i + 1;
        } else if (arg == "-m" && i + 1 < argc) {
            mode = argv[i + 1];
            i = i + 1;
        } else if (arg == "-i" && i + 1 < argc) {
            inputPath = argv[i + 1];
            i = i + 1;
        } else if (arg == "-o" && i + 1 < argc) {
            outputPath = argv[i + 1];
            i = i + 1;
        }
    }

    if (algorithm == "" || mode == "" || inputPath == "" || outputPath == "") {
        std::cout << "Usage: compressor -a [rle|huff|lzw] -m [c|d] -i input_file -o output_file" << std::endl;
        return 1;
    }

    bool ok = false;
    Bytes input = readFile(inputPath, ok);
    if (!ok) {
        std::cout << "Error: cannot open input file " << inputPath << std::endl;
        return 1;
    }

    Bytes output;
    clock_t start = clock();

    if (algorithm == "rle" && mode == "c") {
        output = rleCompress(input);
    } else if (algorithm == "rle" && mode == "d") {
        output = rleDecompress(input);
    } else if (algorithm == "huff" && mode == "c") {
        output = huffmanCompress(input);
    } else if (algorithm == "huff" && mode == "d") {
        output = huffmanDecompress(input);
    } else if (algorithm == "lzw" && mode == "c") {
        output = lzwCompress(input);
    } else if (algorithm == "lzw" && mode == "d") {
        output = lzwDecompress(input);
    } else {
        std::cout << "Error: unknown algorithm/mode combination" << std::endl;
        return 1;
    }

    clock_t end = clock();
    double ms = (double)(end - start) / CLOCKS_PER_SEC * 1000.0;

    writeFile(outputPath, output);

    if (mode == "c") {
        double originalSize = (double)input.size();
        double compressedSize = (double)output.size();

        double ratio = 0;
        if (compressedSize > 0) ratio = originalSize / compressedSize;

        double savings = 0;
        if (originalSize > 0) savings = (1.0 - compressedSize / originalSize) * 100.0;

        std::cout << "Compression complete." << std::endl;
        std::cout << "--------------------------------" << std::endl;
        std::cout << "Algorithm: " << algorithm << std::endl;
        std::cout << "Execution Time: " << ms << " ms" << std::endl;
        std::cout << "Original Size: " << input.size() << " bytes" << std::endl;
        std::cout << "Compressed Size: " << output.size() << " bytes" << std::endl;
        std::cout << "Compression Ratio: " << ratio << std::endl;
        std::cout << "Space Savings: " << savings << "%" << std::endl;
    } else {
        std::cout << "Decompression complete." << std::endl;
        std::cout << "--------------------------------" << std::endl;
        std::cout << "Algorithm: " << algorithm << std::endl;
        std::cout << "Execution Time: " << ms << " ms" << std::endl;
        std::cout << "Compressed Size: " << input.size() << " bytes" << std::endl;
        std::cout << "Decompressed Size: " << output.size() << " bytes" << std::endl;
    }

    return 0;
}
