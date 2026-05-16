#include "lzw.h"
#include <unordered_map>
#include <string>

using namespace std;

vector<uint16_t> encode(const std::vector<uint8_t> &pixels) {
    unordered_map<string, uint16_t> map;
    for (int i = 0; i < 256; i++) {
        map[string(1, (char)i)] = i;
    }

    vector<uint16_t> codes;
    codes.push_back(CLEAR_CODE);

    string omega = "";
    for (uint8_t K : pixels) {
        string omegaK = omega + (char)K;

        if (map.contains(omegaK)) {
            omega = omegaK;
        }
        else {
            codes.push_back(map[omega]);

            if (map.size() < 65535) {
                map[omegaK] = map.size();
            }

            omega = string(1, (char)K);
        }
    }

    if (!omega.empty()) {
        codes.push_back(map[omega]);
    }
    codes.push_back(EOI_CODE);

    return codes;
}

vector<uint8_t> decode(const std::vector<uint16_t>& codes) {
    vector<string> map;
    for (int i = 0; i < 256; i++) {
        map.push_back(string(1, (char)i));
    }

    map.push_back(""); // 256 = CLEAR_CODE
    map.push_back(""); //257 = EOI_CODE

    vector<uint8_t> result;
    string omega = "";

    int i = 0;

    // skip clear code
    if (codes[0] == CLEAR_CODE) {
        i = 1;
    }

    uint16_t code = codes[i++];
    omega = map[code];

    for (uint8_t K : omega) {
        result.push_back(K);
    }

    while (i < codes.size()) {
        code = codes[i++];

        if (code == EOI_CODE) {
            break;
        }

        string entry = "";
        if (code < map.size()) {
            // code already in map
            entry = map[code];
        }
        else {
            // not in the table, the new entry is omega + first character of omega
            entry = omega + omega[0];
        }

        for (uint8_t K : entry) {
            result.push_back(K);
        }

        // add the new entry
        map.push_back(omega + entry[0]);

        omega = entry;
    }

    return result;
}