//
// Created by Waterdog on 2024/11/27.
//

#ifndef GIT450_GIT450PROTOCOL_H
#define GIT450_GIT450PROTOCOL_H

#include <string>
#include "utils.h"

struct Git450Message {
    // Git450 Message Protocol: <OPERATION> <USERNAME> <PAYLOAD>
    std::string operation;
    std::string username;
    std::string payload;

    std::string toString() const {
        return utils::join({operation, username, payload});
    }

    void fromString(const std::string &data) {
        auto tokens = utils::split(data);
        if (tokens.size() >= 1) {
            operation = tokens[0];
        }
        if (tokens.size() >= 2) {
            username = tokens[1];
        }
        if (tokens.size() >= 3) {
            payload = utils::join(std::vector<std::string>(tokens.begin() + 2, tokens.end()));
        }
    }

    std::ostream &operator<<(std::ostream &os) const {
        os << toString();
        return os;
    }
};

namespace protocol {
    // -------- PROTOCOL OPERATIONS --------
    // Git450 Message Protocol: <OPERATION> <USERNAME> <PAYLOAD>
    std::string getOperation(const std::string &data) {
        // Example: "push user1 file1.txt file2.txt file3.txt" -> "push"
        if (data.empty()) {
            return "";
        }
        return utils::toLower(utils::split(data)[0]);  // The operation is always lowercase
    }

    std::string getUsername(const std::string &data) {
        // Example: "push user1 file1.txt file2.txt file3.txt" -> "user1"
        auto tokens = utils::split(data);
        if (tokens.size() <= 1) {
            return "";  // No username
        }
        return tokens[1];
    }

    std::string getPayload(const std::string &data) {
        // Example: "push user1 file1.txt file2.txt file3.txt" -> "file1.txt file2.txt file3.txt"
        auto tokens = utils::split(data);
        if (tokens.size() <= 2) {
            return "";
        }
        return utils::join(std::vector<std::string>(tokens.begin() + 2, tokens.end()));
    }


    Git450Message parseMessage(const std::string &data) {
        return {
                getOperation(data),
                getUsername(data),
                getPayload(data)
        };
    }

    std::string stringifyMessage(const Git450Message &message) {
        return utils::join({message.operation, message.username, message.payload});
    }
}




#endif //GIT450_GIT450PROTOCOL_H
