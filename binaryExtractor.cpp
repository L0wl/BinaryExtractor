#include "header.h"

using namespace std;

BinaryExtractor::BinaryExtractor(const std::string& filename) : m_filename(filename) {}

void BinaryExtractor::extract() {
    std::string folder = m_filename.substr(0, m_filename.find_last_of('.'));
    mkDIR(folder);

    std::ifstream input(m_filename, std::ios::binary);
    if (!input.is_open()) {
        std::cerr << "Error: unable to open file " << m_filename << std::endl;
        return;
    }

    input.seekg(0, std::ios::end);
    size_t file_size = input.tellg();
    input.seekg(0, std::ios::beg);

    while (input.tellg() < file_size) {
        std::string name = readcstr(input);
        std::string ftype = readcstr(input);
        uint32_t c = readuint32(input);
        uint32_t d = readuint32(input);
        uint32_t e = readuint32(input);
        std::vector<unsigned char> data(d);
        input.read(reinterpret_cast<char*>(data.data()), d);
        if (c & 1) {
            data = decompress(data, e);
        }
        std::string output_filename = folder + "/" + name;
        std::ofstream output(output_filename, std::ios::binary);
        if (!output.is_open()) {
            std::cerr << "Error: unable to create file " << output_filename << std::endl;
            return;
        }
        output.write(reinterpret_cast<const char*>(data.data()), data.size());
        output.close();
        std::cout << name << " " << ftype << std::endl;
    }
    std::cout << "COMPLETED!!!" << std::endl;
}

void BinaryExtractor::mkDIR(const std::string& dir) {
    #ifdef _WIN32
        _mkdir(dir.c_str());
    #else 
        mkdir(dir.c_str(), 0777);
    #endif
}

std::vector.unsigned char> BinaryExtractor::decompress(const std::vector<unsigned char>& input, size_t output_size) {
    std::vector<unsigned char> output(output_size);
    unsigned long output_size_long = static_cast<unsigned long>(output_size);
    unsigned long input_size_long = static_cast<unsigned long>(input.size());
    int zret = uncompress(output.data(), &output_size_long, input.data(), input_size_long);
    if (zret != Z_OK) {
        throw std::runtime_error("Error: decompression failed");
    }
    return output;
}

uint32_t BinaryExtractor::readuint32(std::ifstream& input) {
    uint32_t value;
    input.read(reinterpret_cast<char*>(&value), sizeof(uint32_t));
    return value;
}

std::string BinaryExtractor::readcstr(std::ifstream& input) {
    std::vector<char> buffer;
    char c;
    while (input.read(&c, 1) && c != '\0') {
        buffer.push_back(c);
    }
    return std::string(buffer.begin(), buffer.end());
}
