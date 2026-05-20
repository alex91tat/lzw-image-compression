#include "lzw.h"
#include <unordered_map>
#include <string>
#include <iostream>

using namespace std;

vector<uint16_t> encode(const vector<uint8_t>& pixels) {
    if (pixels.empty()) return {};

    unordered_map<string, uint16_t> dictionary;
    for (int i = 0; i < 256; i++) {
        dictionary[string(1, (char)i)] = i;
    }

    vector<uint16_t> codes;
    codes.push_back(CLEAR_CODE);

    string omega = string(1, (char)pixels[0]);
    uint16_t nextCode = FIRST_CODE;

    for (int idx = 1; idx < pixels.size(); idx++) {
        uint8_t K = pixels[idx];
        string omegaK = omega + (char)K;

        if (dictionary.count(omegaK)) {
            omega = omegaK;
        } else {
            codes.push_back(dictionary[omega]);

            if (nextCode >= 65535) {
                // dictionary full reset
                codes.push_back(CLEAR_CODE);
                dictionary.clear();
                for (int i = 0; i < 256; i++) {
                    dictionary[string(1, (char)i)] = i;
                }
                nextCode = FIRST_CODE;
            } else {
                dictionary[omegaK] = nextCode++;
            }

            omega = string(1, (char)K);
        }
    }

    if (!omega.empty()) {
        codes.push_back(dictionary[omega]);
    }

    codes.push_back(EOI_CODE);
    return codes;
}

vector<uint8_t> decode(const vector<uint16_t>& codes) {
    if (codes.empty()) return {};

    vector<string> dictionary;
    for (int i = 0; i < 256; i++) {
        dictionary.push_back(string(1, (char)i));
    }
    dictionary.push_back(""); // 256 = CLEAR_CODE
    dictionary.push_back(""); // 257 = EOI_CODE

    vector<uint8_t> result;
    string omega = "";
    int i = 0;

    if (codes[0] == CLEAR_CODE) i = 1;

    // read first real code
    uint16_t code = codes[i++];
    omega = dictionary[code];
    for (uint8_t K : omega) result.push_back(K);

    while (i < (int)codes.size()) {
        code = codes[i++];

        if (code == EOI_CODE) break;

        if (code == CLEAR_CODE) {
            dictionary.clear();
            for (int j = 0; j < 256; j++) {
                dictionary.push_back(string(1, (char)j));
            }
            dictionary.push_back(""); // CLEAR_CODE
            dictionary.push_back(""); // EOI_CODE

            // read first code after reset
            if (i >= (int)codes.size()) break;
            code = codes[i++];
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

        if (dictionary.size() < 65535)
            dictionary.push_back(omega + entry[0]);

        omega = entry;
    }

    return result;
}