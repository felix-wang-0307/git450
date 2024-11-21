
#ifndef GIT450_UTILS_H
#define GIT450_UTILS_H

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <map>

namespace utils {
    std::vector<std::string> split(const std::string& str, char delimiter = ' ') {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(str);
        while (std::getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    std::string getOperation(const std::string& data) {
        return split(data)[0];
    }

    std::string getPayload(const std::string& data) {
        return data.substr(data.find(' ') + 1);
    }

    std::string toAstrix(const std::string &input) {
        return std::string(input.size(), '*');
    }
}

#endif //GIT450_UTILS_H
