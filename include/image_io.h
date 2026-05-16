#pragma once

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

// loads the grayscale image and returns its pixels as vector of bytes
// sets the width and height
std::vector<uint8_t> loadImage(const std::string& path, int& width, int& height);

// takes the vector of grayscale image and saves it
void saveImage(const std::string& path, const std::vector<uint8_t>& pixels, int width, int height);