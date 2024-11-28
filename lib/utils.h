
#ifndef GIT450_UTILS_H
#define GIT450_UTILS_H

#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>

namespace utils {
    // -------- BASIC STRING OPERATIONS --------
    std::vector<std::string> split(const std::string &str, char delimiter = ' ') {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(str);
        while (std::getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    std::string join(const std::vector<std::string> &tokens, char delimiter = ' ') {
        std::string result;
        for (const std::string &token: tokens) {
            result.append(token);
            result.push_back(delimiter);
        }
        return result.substr(0, result.size() - 1);
    }

    std::string toUpper(const std::string &str) {
        std::string result;
        for (char c: str) {
            result.push_back(std::toupper(c));
        }
        return result;
    }

    std::string toLower(const std::string &str) {
        std::string result;
        for (char c: str) {
            result.push_back(std::tolower(c));
        }
        return result;
    }


    std::string trim(const std::string &str) {
        size_t first = str.find_first_not_of(' ');
        if (std::string::npos == first) {
            return str;
        }
        size_t last = str.find_last_not_of(' ');
        return str.substr(first, (last - first + 1));
    }

    std::string toAstrix(const std::string &input) {
        return std::string(input.size(), '*');
    }
}

#endif //GIT450_UTILS_H
