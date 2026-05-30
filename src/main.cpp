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

    vector<uint8_t> compressed = encode(pixels);

    ofstream file(outputPath, ios::binary);
    if (!file.is_open()) {
        cout << "Error: could not open output file " << outputPath << endl;
        return;
    }

    // write header
    uint32_t w = (uint32_t)width;
    uint32_t h = (uint32_t)height;
    uint32_t dataSize = (uint32_t)compressed.size();
    file.write((char*)&w,        sizeof(uint32_t));
    file.write((char*)&h,        sizeof(uint32_t));
    file.write((char*)&dataSize, sizeof(uint32_t));

    // write packed bit stream
    file.write((char*)compressed.data(), compressed.size());
    file.close();

    ifstream check(outputPath, ios::binary | ios::ate);
    long compressedSize = check.tellg();
    check.close();

    long originalSize = width * height;

    cout << "File:              " << inputPath << endl;
    cout << "Original size:     " << originalSize   << " bytes" << endl;
    cout << "Compressed size:   " << compressedSize << " bytes" << endl;
    cout << "Compression ratio: " << (float)originalSize / compressedSize << endl;
    cout << "-----------------------------------" << endl;
}

void decompress(const string& inputPath, const string& outputPath) {
    ifstream file(inputPath, ios::binary);
    if (!file.is_open()) {
        cout << "Error: could not open file " << inputPath << endl;
        return;
    }

    uint32_t w, h, dataSize;
    file.read((char*)&w,        sizeof(uint32_t));
    file.read((char*)&h,        sizeof(uint32_t));
    file.read((char*)&dataSize, sizeof(uint32_t));

    int width  = (int)w;
    int height = (int)h;

    // read packed bit stream
    vector<uint8_t> compressed(dataSize);
    file.read((char*)compressed.data(), dataSize);
    file.close();

    vector<uint8_t> pixels = decode(compressed);

    if ((int)pixels.size() != width * height) {
        cout << "Error: pixel count mismatch! got " << pixels.size()
             << " expected " << width * height << endl;
        return;
    }

    saveImage(outputPath, pixels, width, height);
    cout << "Decompressed: " << inputPath << " -> " << outputPath << endl;
}

void benchmark(const vector<string>& imagePaths) {
    cout << "\n=== BENCHMARK RESULTS ===" << endl;
    cout << "Image                  Original        Compressed      Ratio" << endl;
    cout << "----------------------------------------------------------------" << endl;

    for (const string& path : imagePaths) {
        int width, height;
        vector<uint8_t> pixels = loadImage(path, width, height);
        if (pixels.empty()) continue;

        vector<uint8_t> compressed = encode(pixels);

        long originalSize   = width * height;
        long compressedSize = sizeof(uint32_t) * 3 + compressed.size();
        float ratio = (float)originalSize / compressedSize;

        string filename = path.substr(path.find_last_of("/\\") + 1);

        cout << filename     << "\t\t"
             << originalSize << " bytes\t\t"
             << compressedSize << " bytes\t\t"
             << ratio        << endl;
    }
}


void convertToBmp(const string& inputPath, const string& outputPath) {
    Mat_<uchar> img = imread(inputPath, 0);
    if (img.empty()) {
        cout << "Error: could not load image from: " << inputPath << endl;
        return;
    }

    imwrite(outputPath, img);
    cout << "Converted: " << inputPath << " -> " << outputPath << endl;
}

void displayResults() {
    vector<string> images = {
        "camera_man",
        "chess_board",
        "chest_xray",
        "grass_texture",
        "text_page"
    };

    for (const string& name : images) {
        Mat original   = imread("../Images/"  + name + ".bmp", 0);
        Mat decomp     = imread("../output/"  + name + ".bmp", 0);

        if (original.empty() || decomp.empty()) continue;

        Mat combined;
        hconcat(original, decomp, combined);

        namedWindow(name + " (original | decompressed)", WINDOW_FREERATIO);
        imshow(name + " (original | decompressed)", combined);
    }

    waitKey(0);
    destroyAllWindows();
}

int main() {
    vector<string> images = {
        "../Images/camera_man.bmp",
        "../Images/chess_board.bmp",
        "../Images/chest_xray.bmp",
        "../Images/grass_texture.bmp",
        "../Images/text_page.bmp"
    };

    cout << "CONVERTING IMAGES:" << endl;
    convertToBmp("../Images/camera_man.png",    "../Images/camera_man.bmp");
    convertToBmp("../Images/chess_board.png",   "../Images/chess_board.bmp");
    convertToBmp("../Images/chest_xray.png",    "../Images/chest_xray.bmp");
    convertToBmp("../Images/grass_texture.png", "../Images/grass_texture.bmp");
    convertToBmp("../Images/text_page.png",     "../Images/text_page.bmp");

    cout << "COMPRESSING:" << endl;
    for (const string& path : images) {
        string filename = path.substr(path.find_last_of("/\\") + 1);
        filename = filename.substr(0, filename.find_last_of('.'));
        compress(path, "../compressed/" + filename + ".lzw");
    }

    benchmark(images);

    cout << "\nDECOMPRESSING:" << endl;
    decompress("../compressed/camera_man.lzw",    "../output/camera_man.bmp");
    decompress("../compressed/chess_board.lzw",   "../output/chess_board.bmp");
    decompress("../compressed/chest_xray.lzw",    "../output/chest_xray.bmp");
    decompress("../compressed/grass_texture.lzw", "../output/grass_texture.bmp");
    decompress("../compressed/text_page.lzw",     "../output/text_page.bmp");

    displayResults();

    return 0;
}