#include "huffman.h"
#include <vector>
#include <string>

// a tree node: either a leaf (holds a byte) or an internal node (holds two children)
struct Node {
    unsigned char byte;
    long long freq;
    Node* left;
    Node* right;
};

Node* newNode(unsigned char byte, long long freq) {
    Node* node = new Node;
    node->byte = byte;
    node->freq = freq;
    node->left = NULL;
    node->right = NULL;
    return node;
}

bool isLeaf(Node* node) {
    return node->left == NULL && node->right == NULL;
}

// finds the index of the node with the smallest frequency in the list
int findSmallest(std::vector<Node*> &list) {
    int smallestIndex = 0;
    for (int i = 1; i < (int)list.size(); i++) {
        if (list[i]->freq < list[smallestIndex]->freq) {
            smallestIndex = i;
        }
    }
    return smallestIndex;
}

// builds the Huffman tree from 256 frequency counts and returns the root node
Node* buildTree(long long freq[256]) {
    std::vector<Node*> list;
    for (int b = 0; b < 256; b++) {
        if (freq[b] > 0) {
            list.push_back(newNode((unsigned char)b, freq[b]));
        }
    }

    if (list.size() == 0) return NULL;

    // repeatedly combine the two smallest nodes until only one is left (the root)
    while (list.size() > 1) {
        int i1 = findSmallest(list);
        Node* first = list[i1];
        list.erase(list.begin() + i1);

        int i2 = findSmallest(list);
        Node* second = list[i2];
        list.erase(list.begin() + i2);

        Node* parent = newNode(0, first->freq + second->freq);
        parent->left = first;
        parent->right = second;
        list.push_back(parent);
    }

    return list[0];
}

// walks the tree and records the bit code ("left" = 0, "right" = 1) for every byte
void generateCodes(Node* node, std::string path, std::string codes[256]) {
    if (isLeaf(node)) {
        if (path == "") path = "0";  // only one distinct byte value in the whole file
        codes[node->byte] = path;
        return;
    }
    generateCodes(node->left, path + "0", codes);
    generateCodes(node->right, path + "1", codes);
}

void freeTree(Node* node) {
    if (node == NULL) return;
    freeTree(node->left);
    freeTree(node->right);
    delete node;
}

// file format:
//   8 bytes  -> original file size
//   256 x 4 bytes -> frequency count of each byte value 0..255
//   rest     -> the encoded bits, packed 8 per byte
Bytes huffmanCompress(Bytes input) {
    Bytes output;
    long long originalSize = (long long)input.size();
    appendNumber(output, originalSize, 8);

    long long freq[256];
    for (int b = 0; b < 256; b++) freq[b] = 0;
    for (int i = 0; i < (int)input.size(); i++) {
        freq[input[i]] = freq[input[i]] + 1;
    }
    for (int b = 0; b < 256; b++) {
        appendNumber(output, freq[b], 4);
    }

    if (input.size() == 0) return output;

    Node* root = buildTree(freq);
    std::string codes[256];
    generateCodes(root, "", codes);

    // build one long string of '0'/'1' characters for the whole input
    std::string bits = "";
    for (int i = 0; i < (int)input.size(); i++) {
        bits += codes[input[i]];
    }

    // pack every 8 characters of bits into one byte (pad the last byte with 0s)
    int i = 0;
    while (i < (int)bits.length()) {
        unsigned char byte = 0;
        for (int j = 0; j < 8; j++) {
            byte = byte << 1;
            if (i + j < (int)bits.length() && bits[i + j] == '1') {
                byte = byte | 1;
            }
        }
        output.push_back(byte);
        i = i + 8;
    }

    freeTree(root);
    return output;
}

Bytes huffmanDecompress(Bytes input) {
    Bytes output;
    int pos = 0;
    long long originalSize = readNumber(input, pos, 8);

    long long freq[256];
    for (int b = 0; b < 256; b++) {
        freq[b] = readNumber(input, pos, 4);
    }

    if (originalSize == 0) return output;

    Node* root = buildTree(freq);

    // turn the remaining bytes back into a string of '0'/'1' characters
    std::string bits = "";
    for (int i = pos; i < (int)input.size(); i++) {
        unsigned char byte = input[i];
        for (int j = 7; j >= 0; j--) {
            if ((byte >> j) & 1) bits += "1";
            else bits += "0";
        }
    }

    // special case: the file only had one distinct byte value
    if (isLeaf(root)) {
        for (int i = 0; i < (int)originalSize; i++) {
            output.push_back(root->byte);
        }
        freeTree(root);
        return output;
    }

    // walk the tree one bit at a time; every time we land on a leaf, that's one output byte
    Node* current = root;
    int i = 0;
    while ((int)output.size() < originalSize && i < (int)bits.length()) {
        if (bits[i] == '0') current = current->left;
        else current = current->right;

        if (isLeaf(current)) {
            output.push_back(current->byte);
            current = root;
        }
        i = i + 1;
    }

    freeTree(root);
    return output;
}
