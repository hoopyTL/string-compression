#include "utils.h"
#include <fstream>

Bytes readFile(std::string path, bool &success) {
    Bytes data;
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file) {
        success = false;
        return data;
    }

    unsigned char byte;
    while (file.read((char*)&byte, 1)) {
        data.push_back(byte);
    }

    success = true;
    return data;
}

bool writeFile(std::string path, Bytes data) {
    std::ofstream file(path.c_str(), std::ios::binary);
    if (!file) return false;

    for (int i = 0; i < (int)data.size(); i++) {
        file.put(data[i]);
    }
    return true;
}

void appendNumber(Bytes &data, long long value, int numBytes) {
    for (int i = 0; i < numBytes; i++) {
        unsigned char byte = (unsigned char)((value >> (8 * i)) & 0xFF);
        data.push_back(byte);
    }
}

long long readNumber(Bytes &data, int &pos, int numBytes) {
    long long value = 0;
    for (int i = 0; i < numBytes; i++) {
        long long b = data[pos + i];
        value = value | (b << (8 * i));
    }
    pos = pos + numBytes;
    return value;
}
