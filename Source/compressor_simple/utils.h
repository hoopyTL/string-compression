#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <string>

// a "Bytes" is just a list of raw bytes (0-255 each)
typedef std::vector<unsigned char> Bytes;

// reads the whole file into a Bytes list
// sets success to false if the file could not be opened
Bytes readFile(std::string path, bool &success);

// writes a Bytes list to a file, returns false if it fails
bool writeFile(std::string path, Bytes data);

// appends "numBytes" bytes of "value" to the end of data (little-endian)
// example: appendNumber(data, 300, 2) appends 2 bytes representing 300
void appendNumber(Bytes &data, long long value, int numBytes);

// reads "numBytes" bytes starting at data[pos] as a number,
// then moves pos forward by numBytes so the next call continues after it
long long readNumber(Bytes &data, int &pos, int numBytes);

#endif
