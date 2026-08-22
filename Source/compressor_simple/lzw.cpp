#include "lzw.h"
#include <map>
#include <vector>
#include <string>

// codes are stored as 2 bytes each, so the dictionary can hold at most 65536 entries
const int MAX_DICT_SIZE = 65536;

Bytes lzwCompress(Bytes input) {
    Bytes output;
    if (input.size() == 0) return output;

    // start the dictionary with every single byte value (0..255) mapped to itself
    std::map<std::string, int> dictionary;
    for (int i = 0; i < 256; i++) {
        std::string s = "";
        s = s + (char)i;
        dictionary[s] = i;
    }
    int nextCode = 256;

    std::string current = "";
    current = current + (char)input[0];

    for (int i = 1; i < (int)input.size(); i++) {
        std::string next = current + (char)input[i];

        if (dictionary.count(next) > 0) {
            // "next" is already known, keep growing it
            current = next;
        } else {
            // output the code for what we had, then add the new string to the dictionary
            appendNumber(output, dictionary[current], 2);
            if (nextCode < MAX_DICT_SIZE) {
                dictionary[next] = nextCode;
                nextCode = nextCode + 1;
            }
            current = "";
            current = current + (char)input[i];
        }
    }
    appendNumber(output, dictionary[current], 2);

    return output;
}

Bytes lzwDecompress(Bytes input) {
    Bytes output;
    if (input.size() == 0) return output;

    // start the dictionary the same way as compression: entry i is just byte value i
    std::vector<std::string> dictionary;
    for (int i = 0; i < 256; i++) {
        std::string s = "";
        s = s + (char)i;
        dictionary.push_back(s);
    }
    int nextCode = 256;

    int pos = 0;
    int code = (int)readNumber(input, pos, 2);
    std::string previous = dictionary[code];
    for (int i = 0; i < (int)previous.length(); i++) {
        output.push_back((unsigned char)previous[i]);
    }

    while (pos < (int)input.size()) {
        code = (int)readNumber(input, pos, 2);

        std::string entry;
        if (code < (int)dictionary.size()) {
            entry = dictionary[code];
        } else {
            // code not in dictionary yet: this only happens for the special
            // "previous + first char of previous" pattern
            entry = previous + previous[0];
        }

        for (int i = 0; i < (int)entry.length(); i++) {
            output.push_back((unsigned char)entry[i]);
        }

        if (nextCode < MAX_DICT_SIZE) {
            dictionary.push_back(previous + entry[0]);
            nextCode = nextCode + 1;
        }
        previous = entry;
    }

    return output;
}
