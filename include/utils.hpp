#pragma once

#include <iostream>
#include "structure.hpp"

#define DEBUG 0

#if DEBUG == 1
    #define LOG(x)                 std::cout << x << std::endl;
    #define LOGKV(x)               std::cout << #x << " = " << (x) <<  std::endl;
    #define LOGINL(x)              std::cout << x << " ";
    #define SPACE                  std::cout << " ";
    #define LOOP_START(x)          std::cout << "\n\n========================== " << "Loop " << (x) << " ==========================\n";
    #define DDASH_LINE             std::cout << "==============================================================" << std::endl;
    #define DASH_LINE              std::cout << "--------------------------------------------------------------" << std::endl;
    #define END_LINE               std::cout << std::endl;
    #define AAA                    std::cout << "AAAAAAAAAAAAAAAAAAAA" << std::endl;
    #define BBB                    std::cout << "BBBBBBBBBBBBBBBBBBBB" << std::endl;
#else
    #define LOG(x) 
    #define LOGKV(x) 
    #define LOGINL(x) 
    #define SPACE 
    #define LOOP_START(x) 
    #define DDASH_LINE 
    #define DASH_LINE 
    #define END_LINE 
    #define AAA 
    #define BBB 
#endif

#define MAX(x, y) ((x) > (y))? x : y
#define MIN(x, y) ((x) < (y))? x : y


template <class T>
using MinQueue = std::priority_queue<T, std::vector<T>, std::greater<T>>;

#if DEBUG == 1
template<class T>
void printMinQueue(MinQueue<T>& pq){
    while (!pq.empty()){
        TrunkPtr ptr = pq.top();
        Trunk* t = ptr.ptr;
        std::cout << t->getName() << " : (" << t->start << ", " << t->end << ")";
        pq.pop();
        END_LINE
    }
}
#endif