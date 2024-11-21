
#ifndef GIT450_UTILS_H
#define GIT450_UTILS_H

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <map>

namespace utils {
    std::map<std::string, std::string> loadConfig(const std::string& filename = "config.txt") {
        std::ifstream config_file(filename);
        std::map<std::string, std::string> config;
        if (!config_file.is_open()) {
            throw std::runtime_error("Failed to open configuration file.");
        }

        std::string line;
        while (std::getline(config_file, line)) {
            std::istringstream line_stream(line);
            std::string key, value;
            if (std::getline(line_stream, key, '=') && std::getline(line_stream, value)) {
                config[key] = value;
            }
        }

        return config;
    }

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
}

std::map<std::string, std::string> config = utils::loadConfig();

#endif //GIT450_UTILS_H
