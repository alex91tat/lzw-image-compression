#pragma once

#include <vector>
#include <cstdint>

const uint16_t CLEAR_CODE = 256;
const uint16_t EOI_CODE   = 257;
const uint16_t FIRST_CODE = 258;

// takes the raw image pixels (one byte per pixel) and compress them
std::vector<uint16_t> encode(const std::vector<uint8_t>& pixels);

// takes the codes and reconstructs the original bytes
std::vector<uint8_t> decode(const std::vector<uint16_t>& codes);