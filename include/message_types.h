//
// Created by Waterdog on 2024/12/2.
//

#ifndef GIT450_MESSAGE_TYPES_H
#define GIT450_MESSAGE_TYPES_H

#include <string>
#include <vector>
#include <iostream>
#include "utils.h"
#include "git450protocol.h"

using std::string;
using std::vector;

class Git450AuthRequest: public Git450Message {
public:
    Git450AuthRequest(const string &username, const string &password) {
        this->username = username;
        this->operation = "auth";
        this->payload = password;
    }

    Git450AuthRequest(const string &data) {
        fromString(data);
    }

    string getPassword() const {
        return payload;
    }

    string getMaskedPassword() const {
        return utils::toAstrix(payload);
    }
};

class Git450AuthResponse: public Git450Message {
public:
    Git450AuthResponse(const string &username, const string &result) {
        this->username = username;
        this->operation = "auth_result";
        this->payload = result;
    }

    Git450AuthResponse(const string &data) {
        fromString(data);
    }

    ClientType getResult() const {
        // payload is either "guest" or "member"
        return StringToClientType.at(payload);
    }
};

class Git450LookupRequest: public Git450Message {
public:
    Git450LookupRequest(const string &username, const string &target_username) {
        this->username = username;
        this->operation = "lookup";
        this->payload = target_username;
    }

    Git450LookupRequest(const string &data) {
        fromString(data);
    }

    string getTargetUsername() const {
        return payload;
    }
};

class Git450LookupResponse: public  Git450Message {
public:
    Git450LookupResponse(const string &username, const string &status, const vector<string> &files) {
        this->username = username;
        this->operation = "lookup";
        this->payload = status + " " + utils::join(files);
    }

    Git450LookupResponse(const string &data) {
        fromString(data);
    }

    string getStatus() const {
        // One of: "user_not_exist", "no_files", "ok"
        return utils::split(payload)[0];
    }

    vector<string> getFiles() const {
        return utils::split(payload);
    }
};

class Git450PushRequest: public Git450Message {
public:
    Git450PushRequest(const string &username, const string &filename) {
        this->username = username;
        this->operation = "push";
        this->payload = filename;
    }

    Git450PushRequest(const string &data) {
        fromString(data);
    }

    string getFilename() const {
        return payload;
    }
};

class Git450PushResponse: public Git450Message {
public:
    Git450PushResponse(const string &username, const string &filename, const string &result) {
        this->username = username;
        this->operation = "push";
        this->payload = filename + " " + result;
    }

    Git450PushResponse(const string &data) {
        fromString(data);
    }

    string getFilename() const {
        return utils::split(payload)[0];
    }

    string getResult() const {
        // One of: "ok", "already_exist"
        return utils::split(payload)[1];
    }
};

class Git450RemoveRequest: public Git450Message {
public:
    Git450RemoveRequest(const string &username, const string &filename) {
        this->username = username;
        this->operation = "remove";
        this->payload = filename;
    }

    Git450RemoveRequest(const string &data) {
        fromString(data);
    }

    string getFilename() const {
        return payload;
    }
};

class Git450RemoveResponse: public Git450Message {
public:
    Git450RemoveResponse(const string &username, const string &result) {
        this->username = username;
        this->operation = "remove";
        this->payload = result;
    }

    Git450RemoveResponse(const string &data) {
        fromString(data);
    }

    string getResult() const {
        // One of: "ok", "not_exist"
        return payload;
    }
};

class Git450DeployRequest: public Git450Message {
public:
    Git450DeployRequest(const string &username) {
        this->username = username;
        this->operation = "deploy";
        this->payload = "";
    }

    string getUsername() const {
        return username;
    }
};

// Server-side messages: ServerM -> ServerR
class Git450ServerDeployRequest: public Git450Message {
public:
    Git450ServerDeployRequest(const string &username, const vector<string> &files) {
        this->username = username;
        this->operation = "deploy";
        this->payload = utils::join(files);
    }

    Git450ServerDeployRequest(const string &data) {
        fromString(data);
    }

    vector<string> getFiles() const {
        return utils::split(payload);
    }
};

class Git450DeployResponse: public Git450Message {
public:
    Git450DeployResponse(const string &username, const string &result) {
        this->username = username;
        this->operation = "deploy";
        this->payload = result;
    }

    Git450DeployResponse(const string &data) {
        fromString(data);
    }

    string getResult() const {
        // One of: "ok", "not_exist"
        return payload;
    }
};

class Git450LogRequest: public Git450Message {
public:
    Git450LogRequest(const string &username) {
        this->username = username;
        this->operation = "log";
        this->payload = "";
    }
};

class Git450LogResponse: public Git450Message {
public:
    Git450LogResponse(const string &username, const string &log) {
        this->username = username;
        this->operation = "log";
        this->payload = log;
    }

    Git450LogResponse(const string &data) {
        fromString(data);
    }

    string getLog() const {
        return payload;
    }
};



#endif //GIT450_MESSAGE_TYPES_H
