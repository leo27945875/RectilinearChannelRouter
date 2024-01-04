#include "include/structure.hpp"
#include "include/utils.hpp"


 void Edge::addFeaibleRange(const Range& newRange){
    std::vector<Range> newFeasible;
        for (const Range& nowRange : feasible)
            if (nowRange.min < newRange.min && nowRange.max < newRange.min ||
                nowRange.min > newRange.max && nowRange.max > newRange.max)
                newFeasible.push_back(nowRange);
            else
                newFeasible.push_back({
                    MIN(nowRange.min, newRange.min),
                    MAX(nowRange.max, newRange.max)
                });

        feasible = std::move(newFeasible);
 }

 bool Edge::checkIsInRange(int start, int end) const {
    int i = 1, n = feasible.size();
    for (const Range& range : feasible){
        int min = range.min;
        int max = range.max;
        if (i == 1 && !isLeftMost) 
            min++;
        if (i == n && !isRightMost)
            max--;

        if (start < min || end > max)
            return false;

        i++;
    }
    return true;
 }