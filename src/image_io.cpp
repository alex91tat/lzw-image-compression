#include "image_io.h"
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

vector<uint8_t> loadImage(const std::string& path, int& width, int& height) {
    Mat_<uchar> img = imread(path, 0);
    if (img.empty()) {
        cout << "Error: could not load image from: " << path << endl;
        return {};
    }

    width = img.cols;
    height = img.rows;

    //we flatten the matrix into a vector
    vector<uint8_t> pixels;
    pixels.reserve(width * height);

    for (int i = 0; i < img.rows; i++) {
        for (int j = 0; j < img.cols; j++) {
            pixels.push_back(img(i, j));
        }
    }

    return pixels;
}

void saveImage(const string& path, const vector<uint8_t>& pixels, int width, int height) {
    Mat_<uchar> img(height, width);
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            img(i, j) = pixels[i * width + j];
        }
    }

    imwrite(path, img);
}