#include <stdio.h>
#include "Lock.hpp"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("No path to yaml file was provided!\n");
        return 1;
    }
    else if (argc > 2) {
        printf("Garbage arguments detected!\n");
        return 1;
    }
    Result ret = LOCK::readConfig(argv[1]);
    if (ret) return ret;
    return LOCK::createPatch("test.bin");
}