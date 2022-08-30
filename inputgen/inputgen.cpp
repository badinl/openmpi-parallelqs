#include<iostream>

int main(int argc, char *argv[]) {
    if(argc < 2) {
        std::cerr << "Please provide output count" << std::endl;
        return 1;
    }

    int count = atoi(argv[1]);

    srand(time(0));
    std::cout << count << std::endl;
    for(int i = 0; i < count; i++) std::cout << std::rand() % count << " ";

    return 0;
}