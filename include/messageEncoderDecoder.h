//
// Created by niv on 1/4/22.
//

#ifndef SPL_EX3_CLIENT_MESSAGEENCODERDECODER_H
#define SPL_EX3_CLIENT_MESSAGEENCODERDECODER_H

#endif //SPL_EX3_CLIENT_MESSAGEENCODERDECODER_H

#include <string>

class messageEncoderDecoder {
public:
    static bool encode(const std::string& input, std::vector<char> &bytes);
    static void decode(char *bytes, size_t len, std::string& output, bool *is_logout);
};