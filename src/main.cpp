#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>
#include "image_io.h"
#include "lzw.h"

using namespace std;
using namespace cv;

void compress(const string& inputPath, const string& outputPath) {
    int width, height;
    vector<uint8_t> pixels = loadImage(inputPath, width, height);
    if (pixels.empty()) {
        cout << "Error: could not load image from " << inputPath << endl;
        return;
    }

    vector<uint16_t> codes = encode(pixels);

    ofstream file(outputPath, ios::binary);
    if (!file.is_open()) {
        cout << "Error: could not open output file " << outputPath << endl;
        return;
    }

    // write header: width, height, number of codes
    file.write((char*)&width,  sizeof(int));
    file.write((char*)&height, sizeof(int));
    uint32_t numCodes = codes.size();
    file.write((char*)&numCodes, sizeof(uint32_t));

    // write the codes
    file.write((char*)codes.data(), codes.size() * sizeof(uint16_t));
    file.close();

    ifstream check(outputPath, ios::binary | ios::ate);
    long compressedSize = check.tellg();
    check.close();

    long originalSize = width * height;

    cout << "File:              " << inputPath << endl;
    cout << "Original size:     " << originalSize    << " bytes" << endl;
    cout << "Compressed size:   " << compressedSize  << " bytes" << endl;
    cout << "Compression ratio: " << (float)originalSize / compressedSize << endl;
    cout << "-----------------------------------" << endl;
}

void decompress(const string& inputPath, const string& outputPath) {
    // open binary file
    ifstream file(inputPath, ios::binary);
    if (!file.is_open()) {
        cout << "Error: could not open file " << inputPath << endl;
        return;
    }

    // read header
    int width, height;
    file.read((char*)&width,  sizeof(int));
    file.read((char*)&height, sizeof(int));

    // read number of codes
    uint32_t numCodes;
    file.read((char*)&numCodes, sizeof(uint32_t));

    // read the codes
    vector<uint16_t> codes(numCodes);
    file.read((char*)codes.data(), numCodes * sizeof(uint16_t));
    file.close();

    // decode codes back to pixels
    vector<uint8_t> pixels = decode(codes);

    // save reconstructed image
    saveImage(outputPath, pixels, width, height);
    cout << "Decompressed: " << inputPath << " -> " << outputPath << endl;
}

void benchmark(const vector<string>& imagePaths) {
    cout << "\nBENCHMARK RESULTS:" << endl;
    cout << "Image                  Original        Compressed      Ratio" << endl;
    cout << "----------------------------------------------------------------" << endl;

    for (const string& path : imagePaths) {
        int width, height;
        vector<uint8_t> pixels = loadImage(path, width, height);
        if (pixels.empty()) continue;

        vector<uint16_t> codes = encode(pixels);

        long originalSize   = width * height;
        long compressedSize = sizeof(int) * 2 + sizeof(uint32_t) + codes.size() * sizeof(uint16_t);
        float ratio = (float)originalSize / compressedSize;

        string filename = path.substr(path.find_last_of("/\\") + 1);

        cout << filename          << "\t\t"
             << originalSize      << " bytes\t\t"
             << compressedSize    << " bytes\t\t"
             << ratio             << endl;
    }
}

int main() {
    vector<string> images = {
        "../Images/camera_man.bmp",
        "../Images/chess_board.bmp",
        "../Images/chest_xray.bmp",
        "../Images/grass_texture.bmp",
        "../Images/text_page.bmp"
    };

    cout << "COMPRESSING:" << endl;
    for (const string& path : images) {
        string filename = path.substr(path.find_last_of("/\\") + 1);
        filename = filename.substr(0, filename.find_last_of('.'));
        compress(path, "../compressed/" + filename + ".lzw");
    }

    // show benchmark table
    benchmark(images);

    // decompress all images
    cout << "\nDECOMPRESSING:" << endl;
    decompress("../compressed/camera_man.lzw",    "../output/camera_man.bmp");
    decompress("../compressed/chess_board.lzw",   "../output/chess_board.bmp");
    decompress("../compressed/chest_xray.lzw",    "../output/chest_xray.bmp");
    decompress("../compressed/grass_texture.lzw", "../output/grass_texture.bmp");
    decompress("../compressed/text_page.lzw",     "../output/text_page.bmp");


    vector<pair<string,string>> verify = {
        {"../Images/camera_man.bmp",    "../output/camera_man.bmp"},
        {"../Images/chess_board.bmp",   "../output/chess_board.bmp"},
        {"../Images/chest_xray.bmp",    "../output/chest_xray.bmp"},
        {"../Images/grass_texture.bmp", "../output/grass_texture.bmp"},
        {"../Images/text_page.bmp",     "../output/text_page.bmp"}
    };

    cout << "\nLOSSLESS VERIFICATION:" << endl;
    for (auto& p : verify) {
        Mat orig    = imread(p.first,  0);
        Mat restored = imread(p.second, 0);
        bool identical = (countNonZero(orig != restored) == 0);
        string filename = p.first.substr(p.first.find_last_of("/\\") + 1);
        cout << filename << ": " << (identical ? "LOSSLESS OK" : "MISMATCH!") << endl;
    }

    return 0;
}