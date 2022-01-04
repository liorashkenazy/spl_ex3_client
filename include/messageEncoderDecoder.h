//
// Created by niv on 1/4/22.
//

#ifndef SPL_EX3_CLIENT_MESSAGEENCODERDECODER_H
#define SPL_EX3_CLIENT_MESSAGEENCODERDECODER_H

#endif //SPL_EX3_CLIENT_MESSAGEENCODERDECODER_H

#include <string>

class messageEncoderDecoder {
public:
    static int encode(const std::string& input, char bytes[], int size);
    static void decode(char bytes[], std::string& output, bool *is_logout);
};