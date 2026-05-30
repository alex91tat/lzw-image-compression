#pragma once

#include <vector>
#include <cstdint>

const uint16_t CLEAR_CODE = 256;
const uint16_t EOI_CODE   = 257;
const uint16_t FIRST_CODE = 258;

const int MIN_BITS = 9;
const int MAX_BITS = 16;

// takes raw image pixels and returns packed bit stream
std::vector<uint8_t> encode(const std::vector<uint8_t>& pixels);

// takes the codes and reconstructs the original bytes
std::vector<uint8_t> decode(const std::vector<uint8_t>& bitStream);