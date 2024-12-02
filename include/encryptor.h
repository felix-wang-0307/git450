
#ifndef GIT450_ENCRYPTOR_H
#define GIT450_ENCRYPTOR_H

#include <string>
#include <cctype>

using std::string;

class Encryptor {
public:
    static string encrypt(const string &input) {
        string result;
        for (char c: input) {
            if (isalpha(c)) {
                if (isupper(c)) {
                    result += (c - 'A' + 3) % 26 + 'A';
                } else {
                    result += (c - 'a' + 3) % 26 + 'a';
                }
            } else if (isdigit(c)) {
                result += (c - '0' + 3) % 10 + '0';
            } else {
                result += c;
            }
        }
        return result;
    }

    static string decrypt(const string &input) {
        string result;
        for (char c: input) {
            if (isalpha(c)) {
                if (isupper(c)) {
                    result += (c - 'A' + 23) % 26 + 'A';
                } else {
                    result += (c - 'a' + 23) % 26 + 'a';
                }
            } else if (isdigit(c)) {
                result += (c - '0' + 7) % 10 + '0';
            } else {
                result += c;
            }
        }
        return result;
    }
};

#endif //GIT450_ENCRYPTOR_H
