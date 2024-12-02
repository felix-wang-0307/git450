#ifndef GIT450_LOGGER_H
#define GIT450_LOGGER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include "utils.h"
#include "git450protocol.h"

using std::string;
using std::vector;
using std::map;

class Logger {
public:
    // Member variables
    map<string, vector<string> > logs;
    string log_file_path;

    // Constructor that initializes the log file
    Logger(const string &log_file_path = "./data/logs.txt") : log_file_path(log_file_path) {
        std::ifstream log_file(log_file_path);

        if (log_file) {
            string line;
            std::getline(log_file, line);  // Skip the header
            if (line != "UserName Logs") {
                std::cerr << "Invalid log file header" << std::endl;
                return;
            }

            while (std::getline(log_file, line)) {
                // Process each line and update the `logs` map
                parseLogLine(line);
            }
        } else {
            std::ofstream file(log_file_path);  // Create an empty file
        }
    }

    // Destructor that saves the logs to the file
    ~Logger() {
        writeFile();
    }

    // Converts logs for a specific user to a string
    string getLogString(const string &username) const {
        auto it = logs.find(username);
        if (it == logs.end()) {
            return "No logs found for " + username;
        }

        std::ostringstream oss;
        oss << "Logs for " << username << ":\n";
        int log_count = 1;
        for (const string &log: it->second) {
            oss << log_count++ << ". " << log << "\n";
        }
        return oss.str();
    }

    // Appends a log for a specific user
    void appendLog(const string &username, const string &log) {
        logs[username].push_back(log);
    }

    void appendLog(const Git450Message& message) {
        logs[message.username].push_back(utils::join({message.operation, message.payload}, ' '));
    }

    // Writes all logs to the file
    void writeFile() {
        std::ofstream file(log_file_path, std::ios::trunc);
        if (!file) {
            std::cerr << "Error opening log file for writing: " << log_file_path << std::endl;
            return;
        }

        file << "UserName Logs\n"; // Add header to the log file
        for (const auto &log: logs) {
            file << log.first << " ";
            for (const string &log_str: log.second) {
                file << "[" << log_str << "]";
            }
            file << std::endl;
        }
    }

private:
    // Helper function to parse individual log lines from the file
    void parseLogLine(const string &line) {
        string username = utils::split(line, ' ')[0];
        string log_string = line.substr(username.size() + 1);

        vector<string> log_list = utils::split(log_string, '[');
        for (string &log: log_list) {
            if (log.empty()) continue;  // Skip the first empty string
            log.pop_back();  // Remove the closing ']'
            logs[username].push_back(log);
        }
    }
};

/*
 * Example Usage:
 * Logger *logger = new Logger();
 * logger->appendLog("HannahWilliams598", "push a.txt");
 * logger->appendLog("HannahWilliams598", "lookup CharlieMartinez274");
 * logger->printLog("HannahWilliams598");
 * delete logger;  // Ensure the logs are saved to the file
 */

#endif //GIT450_LOGGER_H
