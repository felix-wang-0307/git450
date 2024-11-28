#include <string>
#include <unordered_map>
#define debug(x) {std::cerr << "DEBUG: " << (#x) << " = " << (x) << std::endl;}

using std::string;
using std::unordered_map;

namespace config {
    int SERVER_A_PORT = 21012;
    int SERVER_R_PORT = 22012;
    int SERVER_D_PORT = 23012;
    int SERVER_M_UDP_PORT = 24012;
    int SERVER_M_TCP_PORT = 25012;
    string CLIENT_IP = "127.0.0.1";
    string SERVER_IP = "127.0.0.1";
}

enum class ClientType {
    NOT_EXIST = -1,
    INVALID = 0,
    GUEST,
    MEMBER
};

const unordered_map<ClientType, string> ClientTypeToString = {
    {ClientType::NOT_EXIST, "NOT_EXIST"},
    {ClientType::INVALID, "INVALID"},
    {ClientType::GUEST, "GUEST"},
    {ClientType::MEMBER, "MEMBER"}
};

const unordered_map<string, ClientType> StringToClientType = {
    {"NOT_EXIST", ClientType::NOT_EXIST},
    {"INVALID", ClientType::INVALID},
    {"GUEST", ClientType::GUEST},
    {"MEMBER", ClientType::MEMBER}
};