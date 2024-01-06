#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <climits>


struct Range {
    int min = INT_MIN;
    int max = INT_MAX;

    bool operator==(const Range& other) const {
        return min == other.min;
    }
    bool operator>=(const Range& other) const {
        return min >= other.min;
    }
    bool operator<=(const Range& other) const {
        return min <= other.min;
    }
    bool operator>(const Range& other) const {
        return min > other.min;
    }
    bool operator<(const Range& other) const {
        return min < other.min;
    }
};


class Track {
public:
    std::string type;
    int         number;

public:
    Track(std::string type, int number)
        : type(type), number(number)
    {}

public:
    std::string getName()          const { return type + std::to_string(number); }
    std::string getName(int total) const { return type + std::to_string(total - number); }

    bool isAddition() const { return type == "C"; }
};


class Trunk {
public:
    int          number;
    int          start;
    int          end;
    const Track* track;

public:
    Trunk(int number, int start, int end)
        : number(number), start(start), end(end)
    {}

public:
    std::string getName() const { 
        std::stringstream ss;
        std::string name;
        ss << number << "(" << this << ")";
        ss >> name;
        return name; 
    }

    void printInfo() const {
        std::cout << "[Trunk " << getName() << "] = (" << start << ", " << end << ") | track: " << (track? track->getName() : "") << std::endl;
    }
};


class Edge {
public:
    const Track*                    track;
    std::vector<Range>              feasible;
    std::unordered_map<Trunk*, int> terminalCountMap;
    bool                            isRightMost = false, isLeftMost = false;

public:
    Edge(int min, int max, const Track* track)
        : track(track)
    { 
        feasible.push_back({min, max}); 
    }

public:
    void addFeaibleRange(Range newRange);
    bool checkIsInRange (int start, int end) const;
};


// For MinQueue:
class TrunkPtr {
public:
    Trunk* ptr;

public:
    TrunkPtr(Trunk* trunkPtr)
        : ptr(trunkPtr)
    {}

public:
    bool operator==(const TrunkPtr& other) const {
        return ptr->start == other.ptr->start;
    }
    bool operator>=(const TrunkPtr& other) const {
        return ptr->start >= other.ptr->start;
    }
    bool operator<=(const TrunkPtr& other) const {
        return ptr->start <= other.ptr->start;
    }
    bool operator>(const TrunkPtr& other) const {
        return ptr->start > other.ptr->start;
    }
    bool operator<(const TrunkPtr& other) const {
        return ptr->start < other.ptr->start;
    }
};
