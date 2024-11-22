
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

    std::string getUsername(const std::string& data) {
        return split(data)[1];
    }

    std::string getPayload(const std::string& data) {
        return data.substr(data.find(' ') + 1);
    }

    std::string toAstrix(const std::string &input) {
        return std::string(input.size(), '*');
    }

    std::string trim(const std::string &str) {
        size_t first = str.find_first_not_of(' ');
        if (std::string::npos == first) {
            return str;
        }
        size_t last = str.find_last_not_of(' ');
        return str.substr(first, (last - first + 1));
    }

    std::string toUpper(const std::string &str) {
        std::string result;
        for (char c : str) {
            result.push_back(c);
        }
        return result;
    }
}

#endif //GIT450_UTILS_H
