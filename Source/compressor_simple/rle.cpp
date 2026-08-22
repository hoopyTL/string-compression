#include "rle.h"

// output format: pairs of (count, value), 1 byte each
// example: "AAAAA" -> (5, 'A')
Bytes rleCompress(Bytes input) {
    Bytes output;
    int n = (int)input.size();
    int i = 0;

    while (i < n) {
        unsigned char value = input[i];
        int count = 1;

        // count how many times "value" repeats in a row (max 255 at once)
        while (i + count < n && input[i + count] == value && count < 255) {
            count = count + 1;
        }

        output.push_back((unsigned char)count);
        output.push_back(value);
        i = i + count;
    }

    return output;
}

Bytes rleDecompress(Bytes input) {
    Bytes output;
    int n = (int)input.size();

    for (int i = 0; i < n; i = i + 2) {
        unsigned char count = input[i];
        unsigned char value = input[i + 1];
        for (int j = 0; j < count; j++) {
            output.push_back(value);
        }
    }

    return output;
}
