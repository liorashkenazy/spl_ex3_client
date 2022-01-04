//
// Created by niv on 1/4/22.
//
#include <string>
#include <stdlib.h>
#include <ctime>
#include <sstream>
#include <iomanip>
//#include <messageEncoderDecoder.h>
#include "../include/messageEncoderDecoder.h"


using std::string;

void shortToBytes(short num, char *bytesArr) {
    bytesArr[0] = ((num>>8))&0xFF;
    bytesArr[1] = (num&0xFF);
}

short bytesToShort(char* bytesArr)
{
    short result = (short)((bytesArr[0] & 0xff) << 8);
    result += (short)(bytesArr[1] & 0xff);
    return result;
}

int appendToBytes(char *bytesArr, std::string str) {
    int index = 0;
    for (char c : str) {
        bytesArr[index++] = c;
    }
    return index;
}

int messageEncoderDecoder::encode(const std::string& input, char bytes[], int size) {
    int next_delimiter = input.find(' ');
    std::string action = input.substr(0, next_delimiter);
    next_delimiter++;
    int length = 2;

    if (action.compare("REGISTER") == 0) {
        shortToBytes(1, bytes);
        for (int i = 0; i < 3; i++) {
            length += appendToBytes(&bytes[length],
                                    input.substr(next_delimiter, input.find(' ', next_delimiter) - next_delimiter));
            next_delimiter = input.find(' ', next_delimiter) + 1;
            bytes[length++] = 0;
        }
    }
    else if (action.compare("LOGIN") == 0) {
        shortToBytes(2, bytes);
        for (int i = 0; i < 2; i++) {
            length += appendToBytes(&bytes[length],
                                    input.substr(next_delimiter, input.find(' ', next_delimiter) - next_delimiter));
            next_delimiter = input.find(' ', next_delimiter) + 1;
            bytes[length++] = 0;
        }
        bytes[length++] = static_cast<char>(stoi(input.substr(next_delimiter)));
    }
    else if (action.compare("LOGOUT") == 0) {
        shortToBytes(3, bytes);
    }
    else if (action.compare("FOLLOW") == 0) {
        shortToBytes(4, bytes);
        bytes[length++] = static_cast<char>(stoi(input.substr(next_delimiter)));
        next_delimiter += 2;
        length += appendToBytes(&bytes[length], input.substr(next_delimiter));
        bytes[length++] = 0;
    }
    else if (action.compare("POST") == 0) {
        shortToBytes(5, bytes);
        length += appendToBytes(&bytes[length],
                                input.substr(next_delimiter, input.find(' ', next_delimiter) - next_delimiter));
        bytes[length++] = 0;
    }
    else if (action.compare("PM") == 0) {
        shortToBytes(6, bytes);
        for (int i = 0; i < 2; i++) {
            length += appendToBytes(&bytes[length],
                                    input.substr(next_delimiter, input.find(' ', next_delimiter) - next_delimiter));
            next_delimiter = input.find(' ', next_delimiter) + 1;
            bytes[length++] = 0;
        }
        std::time_t timer = std::time(0);
        std::tm *now = std::localtime(&timer);
        std::stringstream time_str_stream;
        time_str_stream << std::put_time(now, "%d-%m-%Y %H-%M");
        length += appendToBytes(&bytes[length], time_str_stream.str());
        bytes[length++] = 0;
    }
    else if (action.compare("LOGSTAT") == 0) {
        shortToBytes(7, bytes);
    }
    else if (action.compare("STAT") == 0) {
        shortToBytes(8, bytes);
        length += appendToBytes(&bytes[length],
                                input.substr(next_delimiter, input.find(' ', next_delimiter) - next_delimiter));
        bytes[length++] = 0;
    }
    else if (action.compare("BLOCK") == 0) {
        shortToBytes(12, bytes);
        length += appendToBytes(&bytes[length],
                                input.substr(next_delimiter, input.find(' ', next_delimiter) - next_delimiter));
        bytes[length++] = 0;
    }

    bytes[length++] = ';';
    return length;
}

void decodeStatMessage(char bytes[], std::string& output) {
    short data;
    int cur_index = 0;
    for (int i = 0; i < 4; i++) {
        data = bytesToShort(&bytes[cur_index]);
        cur_index += 2;
        output.append(std::to_string(data));
    }
}

void messageEncoderDecoder::decode(char *bytes, string &output, bool *is_logout) {
    short opcode = bytesToShort(bytes);
    int current_index = 2;
    if (opcode == 9) {
        output.append("NOTIFICATION ");
        if (0 == bytes[current_index++]) {
            output.append("PM ");
        }
        else {
            output.append("Public ");
        }
        std::string posting_user(&bytes[current_index]);
        output.append(posting_user + " ");
        current_index += posting_user.length() + 1;
        output.append(&bytes[current_index]);
    }
    else if (opcode == 10) {
        output.append("ACK ");
        short ack_op = bytesToShort(&bytes[current_index]);
        current_index += 2;
        output.append(std::to_string(ack_op));

        if (ack_op == 3) {
            *is_logout = true;
        }
        else if (ack_op == 4) {
            output.append(&bytes[current_index]);
        }
        else if (ack_op == 7 || ack_op == 8) {
            decodeStatMessage(&bytes[current_index], output);
        }

    }
    else if (opcode == 11) {
        output.append("ERROR ");
        short ack_op = bytesToShort(&bytes[current_index]);
        current_index += 2;
        output.append(std::to_string(ack_op));
    }
}
