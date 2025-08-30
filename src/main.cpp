#include "cli.h"

int main(int argc, char* argv[]) {
    sail::CLI cli;
    return cli.run(argc, argv);
}