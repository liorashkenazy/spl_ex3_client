//
// Created by niv on 1/4/22.
//
#include <string>
#include <vector>
#include <stdlib.h>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <algorithm>
//#include <messageEncoderDecoder.h>
#include "../include/messageEncoderDecoder.h"

struct encoderCallback {
    const std::string actionName;
    void (*callback)(const std::string &, std::vector<char> &);
};

void appendOpcode(short opcode, std::vector<char> &bytes) {
    bytes.push_back(((opcode>>8))&0xFF);
    bytes.push_back((opcode&0xFF));
}

short bytesToShort(char* bytesArr) {
    short result = (short)((bytesArr[0] & 0xff) << 8);
    result += (short)(bytesArr[1] & 0xff);
    return result;
}

size_t splitAndAppend(const std::string& params, char delimiter, size_t count, std::vector<char> &bytes) {
    size_t next_delimiter = 0;
    for (size_t i = 0; i < count; i++) {
        std::string cur_str;
        if (params.find(delimiter, next_delimiter) == std::string::npos) {
            cur_str = params.substr(next_delimiter);
            next_delimiter = params.find(delimiter, next_delimiter);
        }
        else {
            cur_str = params.substr(next_delimiter, params.find(delimiter, next_delimiter) - next_delimiter);
            next_delimiter = params.find(delimiter, next_delimiter) + 1;
        }
        bytes.insert(bytes.end(), cur_str.cbegin(), cur_str.cend());
        bytes.push_back(0);
    }
    return next_delimiter;
}

void encodeRegister(const std::string &input, std::vector<char> &bytes) {
    appendOpcode(1, bytes);
    (void)splitAndAppend(input, ' ', 3, bytes);
}

void encodeLogin(const std::string &input, std::vector<char> &bytes) {
    appendOpcode(2, bytes);
    size_t next_delimiter = splitAndAppend(input, ' ', 2, bytes);

    bytes.push_back(static_cast<char>(stoi(input.substr(next_delimiter))));
}

void encodeLogout(const std::string &input, std::vector<char> &bytes) {
    appendOpcode(3, bytes);
}

void encodeFollow(const std::string &input, std::vector<char> &bytes) {
    appendOpcode(4, bytes);
    bytes.push_back(static_cast<char>(stoi(input)));
    (void)splitAndAppend(input.substr(2), 0, 1, bytes);
}

void encodePost(const std::string &input, std::vector<char> &bytes) {
    appendOpcode(5, bytes);
    (void)splitAndAppend(input, 0, 1, bytes);
}

void encodePM(const std::string &input, std::vector<char> &bytes) {
    appendOpcode(6, bytes);
    size_t next_delim = splitAndAppend(input, ' ', 1, bytes);
    splitAndAppend(input.substr(next_delim), 0, 1, bytes);

    // Add a timestamp
    std::time_t timer = std::time(0);
    std::tm *now = std::localtime(&timer);
    std::stringstream date_str_stream;
    date_str_stream << std::put_time(now, "%d-%m-%Y %H:%M");
    std::string date_str = date_str_stream.str();
    splitAndAppend(date_str, 0, 1, bytes);
}

void encodeLogstat(const std::string &input, std::vector<char> &bytes) {
    appendOpcode(7, bytes);
}

void encodeStat(const std::string &input, std::vector<char> &bytes) {
    appendOpcode(8, bytes);
    std::string input_copy(input);
    std::replace(input_copy.begin(), input_copy.end(), ' ', '|');
    (void) splitAndAppend(input_copy, 0, 1, bytes);
}

void encodeBlock(const std::string &input, std::vector<char> &bytes) {
    appendOpcode(12, bytes);
    (void)splitAndAppend(input, 0, 1, bytes);
}

static const struct encoderCallback handlers[] = {  {.actionName = "REGISTER", .callback=encodeRegister},
                                                    {.actionName = "LOGIN", .callback=encodeLogin},
                                                    {.actionName = "LOGOUT", .callback=encodeLogout},
                                                    {.actionName = "FOLLOW", .callback=encodeFollow},
                                                    {.actionName = "POST", .callback=encodePost},
                                                    {.actionName = "PM", .callback=encodePM},
                                                    {.actionName = "LOGSTAT", .callback=encodeLogstat},
                                                    {.actionName = "STAT", .callback=encodeStat},
                                                    {.actionName = "BLOCK", .callback=encodeBlock}};

bool messageEncoderDecoder::encode(const std::string& input, std::vector<char> &bytes) {
    int next_delimiter = input.find(' ');
    std::string action = input.substr(0, next_delimiter);
    bool valid_action = false;
    // Exclude the line ending and action name from the params
    next_delimiter++;
    std::string params = input.substr(next_delimiter, input.find("\n") - next_delimiter);
    for (size_t i = 0; i < sizeof(handlers) / sizeof(handlers[0]); i++) {
        if (handlers[i].actionName.compare(action) == 0) {
            handlers[i].callback(params, bytes);
            valid_action = true;
            break;
        }
    }

    bytes.push_back(';');
    return valid_action;
}

void decodeStatMessage(char *bytes, size_t len, std::string& output) {
    short data;
    int cur_index = 0;
    for (size_t i = 0; i < len / 8; i++) {
        for (size_t j = 0; j < 4; j++) {
            data = bytesToShort(&bytes[cur_index]);
            cur_index += 2;
            output.append(" " + std::to_string(data));
        }
        output.append("\n");
    }
}

void messageEncoderDecoder::decode(char *bytes, size_t len, std::string &output, bool *is_logout) {
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
            output.append(" ");
            output.append(&bytes[current_index]);
        }
        else if (ack_op == 7 || ack_op == 8) {
            decodeStatMessage(&bytes[current_index], len - 4, output);
        }

    }
    else if (opcode == 11) {
        output.append("ERROR ");
        short err_op = bytesToShort(&bytes[current_index]);
        current_index += 2;
        output.append(std::to_string(err_op));
        if (err_op == 3) {
            *is_logout = true;
        }
    }
}
