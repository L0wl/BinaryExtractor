#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <algorithm>
#include <stdexcept>
#include <cstdint>

using namespace std;

class BinaryExtractor {
public:
    BinaryExtractor(const string& filename);
    void extract();

private:
    void mkDIR(const string& dir);
    vector<unsigned char> decompress(const vector<unsigned char>& input, size_t output_size);
    uint32_t readuint32(ifstream& input);
    string readcstr(ifstream& input);
    string m_filename;
};
