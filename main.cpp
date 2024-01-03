#include "include/routing.hpp"


int main(int argc, char* args[]){
    RectilinearChannelRouter router;
    router.readFromFile(args[1]);
    router.run();
    router.outputToFile(args[2]);
    return 0;
}