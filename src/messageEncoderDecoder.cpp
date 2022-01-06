//
// Created by niv on 1/4/22.
//
#include <string>
#include <stdlib.h>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <algorithm>
//#include <messageEncoderDecoder.h>
#include "../include/messageEncoderDecoder.h"

using std::string;

struct encoderCallback {
    std::string actionName;
    int (*callback)(const std::string &, char *);
};



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

int encodeRegister(const std::string &input, char *bytes) {
    int length = 2;
    shortToBytes(1, bytes);
    int next_delimiter = 0;
    for (int i = 0; i < 3; i++) {
        length += appendToBytes(&bytes[length],
                                input.substr(next_delimiter, input.find(' ', next_delimiter) - next_delimiter));
        next_delimiter = input.find(' ', next_delimiter) + 1;
        bytes[length++] = 0;
    }
    return length;
}

int encodePM(const std::string &input, char *bytes) {
    int length = 2;

    shortToBytes(6, bytes);
    int next_delimiter = 0;
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

    return length;
}

int encodeLogin(const std::string &input, char *bytes) {
    int length = 2;

    shortToBytes(2, bytes);
    int next_delimiter = 0;
    for (int i = 0; i < 2; i++) {
        length += appendToBytes(&bytes[length],
                                input.substr(next_delimiter, input.find(' ', next_delimiter) - next_delimiter));
        next_delimiter = input.find(' ', next_delimiter) + 1;
        bytes[length++] = 0;
    }
    bytes[length++] = static_cast<char>(stoi(input.substr(next_delimiter)));

    return length;
}

int encodeLogout(const std::string &input, char *bytes) {
    shortToBytes(3, bytes);
    return 2;
}

int encodeFollow(const std::string &input, char *bytes) {
    int length = 2;
    size_t next_delimiter;
    shortToBytes(4, bytes);
    bytes[length++] = static_cast<char>(stoi(input, &next_delimiter));
    next_delimiter += 2;
    length += appendToBytes(&bytes[length], input.substr(next_delimiter));
    bytes[length++] = 0;

    return length;
}

int encodePost(const std::string &input, char *bytes) {
    int length = 2;

    shortToBytes(5, bytes);
    length += appendToBytes(&bytes[length],input);
    bytes[length++] = 0;

    return length;
}

int encodeLogstat(const std::string &input, char *bytes) {
    shortToBytes(7, bytes);
    return 2;
}

int encodeStat(const std::string &input, char *bytes) {
    int length = 2;

    shortToBytes(8, bytes);
    std::string input_copy(input);
    std::replace(input_copy.begin(), input_copy.end(), ' ', '|');
    length += appendToBytes(&bytes[length], input_copy);
    bytes[length++] = 0;

    return length;
}

int encodeBlock(const std::string &input, char *bytes) {
    int length = 2;

    shortToBytes(12, bytes);
    length += appendToBytes(&bytes[length], input);
    bytes[length++] = 0;

    return length;
}

static const struct encoderCallback handlers[] = {  {.actionName = "REGISTER", .callback=encodeRegister},
                                                    {.actionName = "LOGIN", .callback=encodeLogin},
                                                    {.actionName = "LOGOUT", .callback=encodeLogout},
                                                    {.actionName = "FOLLOW", .callback=encodeFollow},
                                                    {.actionName = "POST", .callback=encodePost},
                                                    {.actionName = "PM", .callback=encodePM},
                                                    {.actionName = "LOGTSTAT", .callback=encodeLogstat},
                                                    {.actionName = "STAT", .callback=encodeStat},
                                                    {.actionName = "BLOCK", .callback=encodeBlock}};

int messageEncoderDecoder::encode(const std::string& input, char bytes[], int size) {
    int next_delimiter = input.find(' ');
    std::string action = input.substr(0, next_delimiter);
    // Exclude the line ending and action name from the params
    next_delimiter++;
    std::string params = input.substr(next_delimiter, input.find("\n") - next_delimiter);
    int length = 0;
    for (size_t i = 0; i < sizeof(handlers) / sizeof(handlers[0]); i++) {
        if (handlers[i].actionName.compare(action) == 0) {
            length = handlers[i].callback(params, bytes);
            break;
        }
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
