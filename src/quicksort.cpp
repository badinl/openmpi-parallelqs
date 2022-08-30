#include<iostream>
#include<fstream>
#include<vector>
#include<cstdlib>
#include<ctime>
#include<chrono>

#include"listout.h"
#include"quicksort.h"

int main(int argc, char *argv[]) {
    int size;

    if(argc < 2) {
        std::cerr << "Please provide input file path" << std::endl;
        return 1;
    }

    //open input file
    std::string path = argv[1];
    std::ifstream file;
    file.open(path);
    if(!file) {
        std::cerr << "Could not open input file " << path << std::endl;
        return 2;
    }

    //load size
    file >> size;

    //load input sequence
    int * input = (int*)malloc(sizeof(int) * size);
    for(int i=0; i<size; i++) file >> input[i];
    file.close();

    //run
    srand(time(0));
    uint64_t start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    quicksort(input,size);
    uint64_t end = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    //output
    listout(input,size);
    std::cerr << "Time: " << end - start << std::endl;

    return 0;
}