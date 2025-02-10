#include <fstream>
#include <utility>
#include "utils.hpp"

namespace vr {
    std::vector<char> readFile(const std::string& path) {
        std::vector<char> result;
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            return result;
        }
        file.seekg(0, std::ios::end);
        std::streampos fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        result.resize(fileSize);
        file.read(result.data(), fileSize);
        if (!file) {
            result.clear();
        }
        file.close();
        return result;
    }
}