// compile command : g++ -o run lodepng.cpp mnist_to_png.cpp
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h> 
#include "lodepng.h"
using namespace std;
typedef unsigned char uchar;
typedef unsigned int uint;

uint convertToInt(uchar* c) {
    return ((uint)c[0] << 24) + ((uint)c[1] << 16) + ((uint)c[2] << 8) + c[3];
}
int main() {
    string data_path = "../data";
    bool debug = false, white_bg = false, print_img = false, write_to_png = true;
    for(auto dataset : vector<string>({"training", "testing"})) {
        cout << "Processing " << dataset << " dataset..." << endl;
        string img_path, lbl_path;
        if(dataset == "training") {
            img_path = data_path + "/train-images.idx3-ubyte";
            lbl_path = data_path + "/train-labels.idx1-ubyte";
        }
        else {
            img_path = data_path + "/t10k-images.idx3-ubyte";
            lbl_path = data_path + "/t10k-labels.idx1-ubyte";
        }
        ifstream img_data(img_path, ios::binary);
        ifstream lbl_data(lbl_path, ios::binary);
        if(lbl_data.is_open() and img_data.is_open()) {
            uchar c[4];
            if(debug)
                printf("%x %x %x %x\n", c[0], c[1], c[2], c[3]);

            // reading labels
            lbl_data.read((char*)c, sizeof(int));
            uint magic_number = convertToInt(c);
            if(magic_number != 2049)
                throw runtime_error("Invalid MNIST lable data file!");
            lbl_data.read((char*)c, sizeof(int));
            uint n_lables = convertToInt(c);
            if(debug)
                cout << n_lables << endl;
            uchar* lbl = new uchar[n_lables];
            lbl_data.read((char*)lbl, n_lables);

            // reading images
            img_data.read((char*)c, sizeof(int));
            magic_number = convertToInt(c);
            if(magic_number != 2051)
                throw runtime_error("Invalid MNIST image data file!");
            img_data.read((char*)c, sizeof(int));
            uint n_images = convertToInt(c);
            img_data.read((char*)c, sizeof(int));
            uint n_rows = convertToInt(c);
            img_data.read((char*)c, sizeof(int));
            uint n_cols = convertToInt(c);
            if(debug)
                cout << n_images << " " << n_rows << " " << n_cols << endl;
            if(write_to_png) {
                mkdir(dataset.c_str());
                for(int i = 0; i < 10; i++)
                    mkdir((dataset + "/" + to_string(i)).c_str());
            }
            uint image_size = n_rows * n_cols;
            uchar** img = new uchar*[n_images];
            for(int i = 0; i < n_images; i++) {
                img[i] = new uchar[image_size];
                img_data.read((char*)img[i], image_size);

                // negating pixels
                if(white_bg)
                    for(int j = 0; j < image_size; j++)
                        img[i][j] = ~img[i][j];

                // printing image matrix
                if(print_img) {
                    for(int j = 0; j < n_rows; j++) {
                        for(int k = 0; k < n_cols; k++)
                            cout << (img[i][n_rows * j + k] > 100 ? '#' : '.') << " ";
                        cout << endl;
                    }
                    cout << endl;
                    usleep(500000);
                }

                // writing png
                if(write_to_png)
                    if(lodepng_encode_file((dataset + "/" + to_string(lbl[i]) + "/" + to_string(i) + ".png").c_str(), img[i], n_cols, n_rows, LCT_GREY, 8))
                        throw runtime_error("unable to write '" + dataset + "/" + to_string(lbl[i]) + "/" + to_string(i) + ".png" + "'!");

                // progress bar
                if(!print_img) {
                    uint bar_width = 100;
                    float progress = (float)i / (n_images - 1);
                    uint position = bar_width * progress;
                    cout << "#[";
                    for (int i = 0; i < bar_width; ++i) {
                        if(i < position)
                            cout << "=";
                        else if(i == position)
                            cout << ">";
                        else
                            cout << " ";
                    }
                    cout << "] " << int(progress * 100.0) << " %\r";
                    cout.flush();
                }
            }
            if(!print_img)
                cout << endl;
        } else {
            throw runtime_error("Cannot open dataset at path'" + data_path + "'!");
        }
    }
}