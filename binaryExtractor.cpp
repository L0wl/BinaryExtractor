#include "binaryExtractor.h"

BinaryExtractor::BinaryExtractor(const string& filename) : m_filename(filename) {}

void BinaryExtractor::extract() {
    string folder = m_filename.substr(0, m_filename.find_last_of('.'));
    mkDIR(folder);

    ifstream input(m_filename, std::ios::binary);
    if (!input.is_open()) {
        cerr << "Error: unable to open file " << m_filename << std::endl;
        return;
    }

    input.seekg(0, ios::end);
    size_t file_size = input.tellg();
    input.seekg(0, ios::beg);

    while (input.tellg() < file_size) {
        string name = readcstr(input);
        string ftype = readcstr(input);
        uint32_t c = readuint32(input);
        uint32_t d = readuint32(input);
        uint32_t e = readuint32(input);
        vector<unsigned char> data(d);
        input.read(reinterpret_cast<char*>(data.data()), d);
        if (c & 1) {
            data = decompress(data, e);
        }
        string output_filename = folder + "/" + name;
        ofstream output(output_filename, ios::binary);
        if (!output.is_open()) {
            cerr << "Error: unable to create file " << output_filename << endl;
            return;
        }
        output.write(reinterpret_cast<const char*>(data.data()), data.size());
        output.close();
        cout << name << " " << ftype << endl;
    }
    cout << "COMPLETED!!!" << endl;
}

void BinaryExtractor::mkDIR(const string& dir) {
    #ifdef _WIN32
        _mkdir(dir.c_str());
    #else 
        mkdir(dir.c_str(), 0777);
    #endif
}

vector.unsigned char> BinaryExtractor::decompress(const vector<unsigned char>& input, size_t output_size) {
    vector<unsigned char> output(output_size);
    unsigned long output_size_long = static_cast<unsigned long>(output_size);
    unsigned long input_size_long = static_cast<unsigned long>(input.size());
    int zret = uncompress(output.data(), &output_size_long, input.data(), input_size_long);
    if (zret != Z_OK) {
        throw runtime_error("Error: decompression failed");
    }
    return output;
}

uint32_t BinaryExtractor::readuint32(ifstream& input) {
    uint32_t value;
    input.read(reinterpret_cast<char*>(&value), sizeof(uint32_t));
    return value;
}

string BinaryExtractor::readcstr(ifstream& input) {
    vector<char> buffer;
    char c;
    while (input.read(&c, 1) && c != '\0') {
        buffer.push_back(c);
    }
    return string(buffer.begin(), buffer.end());
}
