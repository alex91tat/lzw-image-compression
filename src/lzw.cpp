#include "lzw.h"
#include <unordered_map>
#include <string>
#include <iostream>

using namespace std;

int getBitWidth(int nextCode) {
    if (nextCode < 512)   return 9;
    if (nextCode < 1024)  return 10;
    if (nextCode < 2048)  return 11;
    if (nextCode < 4096)  return 12;
    if (nextCode < 8192)  return 13;
    if (nextCode < 16384) return 14;
    if (nextCode < 32768) return 15;
    return 16;
}

int getBitWidth12(int nextCode) {
    if (nextCode < 512)  return 9;
    if (nextCode < 1024) return 10;
    if (nextCode < 2048) return 11;
    return 12;
}

void writeBits(uint16_t code, int width, uint32_t& buffer, int& bitCount, vector<uint8_t>& output) {
    buffer = (buffer << width) | code;
    bitCount += width;

    while (bitCount >= 8) {
        bitCount -= 8;
        output.push_back((buffer >> bitCount) & 0xFF);
    }
}

uint16_t readBits(int width, uint32_t& buffer, int& bitCount, const vector<uint8_t>& input, int& pos) {
    while (bitCount < width) {
        if (pos >= (int)input.size())
            return EOI_CODE;
        buffer = (buffer << 8) | input[pos++];
        bitCount += 8;
    }

    bitCount -= width;
    return (buffer >> bitCount) & ((1 << width) - 1);
}

vector<uint8_t> encode(const vector<uint8_t>& pixels) {
    if (pixels.empty())
        return {};

    unordered_map<string, uint16_t> dictionary;
    for (int i = 0; i < 256; i++) {
        dictionary[string(1, (char)i)] = i;
    }

    vector<uint8_t> output;
    uint32_t buffer = 0;
    int bitCount = 0;

    uint16_t nextCode = FIRST_CODE;
    int bitWidth = MIN_BITS;

    // write initial clear code
    writeBits(CLEAR_CODE, bitWidth, buffer, bitCount, output);

    string omega = string(1, (char)pixels[0]);

    for (int idx = 1; idx < (int)pixels.size(); idx++) {
        uint8_t K = pixels[idx];
        string omegaK = omega + (char)K;

        if (dictionary.count(omegaK)) {
            omega = omegaK;
        } else {
            // write code for omega with current bit width
            writeBits(dictionary[omega], bitWidth, buffer, bitCount, output);

            if (nextCode >= 65535) {
                writeBits(CLEAR_CODE, bitWidth, buffer, bitCount, output);
                dictionary.clear();
                for (int i = 0; i < 256; i++) {
                    dictionary[string(1, (char)i)] = i;
                }
                nextCode = FIRST_CODE;
                bitWidth = MIN_BITS;
            } else {
                dictionary[omegaK] = nextCode++;
                // update bit width based on new nextCode
                bitWidth = getBitWidth(nextCode);
            }

            omega = string(1, (char)K);
        }
    }

    if (!omega.empty()) {
        writeBits(dictionary[omega], bitWidth, buffer, bitCount, output);
    }

    writeBits(EOI_CODE, bitWidth, buffer, bitCount, output);

    // flush remaining bits
    if (bitCount > 0) {
        output.push_back((buffer << (8 - bitCount)) & 0xFF);
    }

    return output;
}


vector<uint8_t> decode(const vector<uint8_t>& bitStream) {
    if (bitStream.empty())
        return {};

    vector<string> dictionary;
    for (int i = 0; i < 256; i++) {
        dictionary.push_back(string(1, (char)i));
    }
    dictionary.push_back(""); // 256 = CLEAR_CODE
    dictionary.push_back(""); // 257 = EOI_CODE

    vector<uint8_t> result;
    uint32_t buffer = 0;
    int bitCount = 0;
    int pos = 0;
    int bitWidth = MIN_BITS;
    uint16_t nextCode = FIRST_CODE;

    uint16_t code = readBits(bitWidth, buffer, bitCount, bitStream, pos);
    if (code != CLEAR_CODE)
        return {};

    code = readBits(bitWidth, buffer, bitCount, bitStream, pos);
    if (code == EOI_CODE)
        return result;

    string omega = dictionary[code];
    for (uint8_t K : omega) result.push_back(K);

    while (true) {
        // +1: the decoder adds its new entry one step later than the encoder,
        // so it must anticipate that entry to widen at the same code
        bitWidth = getBitWidth(nextCode + 1);

        code = readBits(bitWidth, buffer, bitCount, bitStream, pos);

        if (code == EOI_CODE) break;

        if (code == CLEAR_CODE) {
            dictionary.clear();
            for (int j = 0; j < 256; j++) {
                dictionary.push_back(string(1, (char)j));
            }
            dictionary.push_back(""); // CLEAR_CODE
            dictionary.push_back(""); // EOI_CODE

            bitWidth = MIN_BITS;
            nextCode = FIRST_CODE;

            // read first code after reset
            code = readBits(bitWidth, buffer, bitCount, bitStream, pos);
            if (code == EOI_CODE) break;

            omega = dictionary[code];
            for (uint8_t K : omega) result.push_back(K);
            continue;
        }

        string entry;
        if (code < (uint16_t)dictionary.size()) {
            entry = dictionary[code];
        } else {
            // edge case
            entry = omega + omega[0];
        }

        if (entry.empty()) break;

        for (uint8_t K : entry) result.push_back(K);

        if (nextCode < 65535) {
            dictionary.push_back(omega + entry[0]);
            nextCode++;
        }

        omega = entry;
    }

    return result;
}

vector<uint8_t> encode12(const vector<uint8_t>& pixels) {
    if (pixels.empty())
        return {};

    unordered_map<string, uint16_t> dictionary;
    for (int i = 0; i < 256; i++) {
        dictionary[string(1, (char)i)] = i;
    }

    vector<uint8_t> output;
    uint32_t buffer = 0;
    int bitCount = 0;
    uint16_t nextCode = FIRST_CODE;
    int bitWidth = MIN_BITS;

    writeBits(CLEAR_CODE, bitWidth, buffer, bitCount, output);

    string omega = string(1, (char)pixels[0]);

    for (int idx = 1; idx < (int)pixels.size(); idx++) {
        uint8_t K = pixels[idx];
        string omegaK = omega + (char)K;

        if (dictionary.count(omegaK)) {
            omega = omegaK;
        } else {
            writeBits(dictionary[omega], bitWidth, buffer, bitCount, output);

            if (nextCode >= 4096) {
                // reset at 4096 for 12 bit max
                writeBits(CLEAR_CODE, bitWidth, buffer, bitCount, output);
                dictionary.clear();
                for (int i = 0; i < 256; i++) {
                    dictionary[string(1, (char)i)] = i;
                }
                nextCode = FIRST_CODE;
                bitWidth = MIN_BITS;
            } else {
                dictionary[omegaK] = nextCode++;
                bitWidth = getBitWidth12(nextCode);
            }

            omega = string(1, (char)K);
        }
    }

    if (!omega.empty()) {
        writeBits(dictionary[omega], bitWidth, buffer, bitCount, output);
    }

    writeBits(EOI_CODE, bitWidth, buffer, bitCount, output);

    if (bitCount > 0) {
        output.push_back((buffer << (8 - bitCount)) & 0xFF);
    }

    return output;
}

vector<uint8_t> decode12(const vector<uint8_t>& bitStream) {
    if (bitStream.empty())
        return {};

    vector<string> dictionary;
    for (int i = 0; i < 256; i++) {
        dictionary.push_back(string(1, (char)i));
    }
    dictionary.push_back(""); // 256 = CLEAR_CODE
    dictionary.push_back(""); // 257 = EOI_CODE

    vector<uint8_t> result;
    uint32_t buffer = 0;
    int bitCount = 0;
    int pos = 0;
    int bitWidth = MIN_BITS;
    uint16_t nextCode = FIRST_CODE;

    uint16_t code = readBits(bitWidth, buffer, bitCount, bitStream, pos);
    if (code != CLEAR_CODE)
        return {};

    code = readBits(bitWidth, buffer, bitCount, bitStream, pos);
    if (code == EOI_CODE)
        return result;

    string omega = dictionary[code];
    for (uint8_t K : omega) result.push_back(K);

    while (true) {
        bitWidth = getBitWidth12(nextCode + 1);

        code = readBits(bitWidth, buffer, bitCount, bitStream, pos);

        if (code == EOI_CODE) break;

        if (code == CLEAR_CODE) {
            dictionary.clear();
            for (int j = 0; j < 256; j++) {
                dictionary.push_back(string(1, (char)j));
            }
            dictionary.push_back(""); // CLEAR_CODE
            dictionary.push_back(""); // EOI_CODE

            bitWidth = MIN_BITS;
            nextCode = FIRST_CODE;

            code = readBits(bitWidth, buffer, bitCount, bitStream, pos);
            if (code == EOI_CODE) break;

            omega = dictionary[code];
            for (uint8_t K : omega) result.push_back(K);
            continue;
        }

        string entry;
        if (code < (uint16_t)dictionary.size()) {
            entry = dictionary[code];
        } else {
            entry = omega + omega[0];
        }

        if (entry.empty()) break;

        for (uint8_t K : entry) result.push_back(K);

        if (nextCode < 4096) {
            dictionary.push_back(omega + entry[0]);
            nextCode++;
        }

        omega = entry;
    }

    return result;
}