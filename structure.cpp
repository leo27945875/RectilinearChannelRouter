#include "include/structure.hpp"
#include "include/utils.hpp"


 void Edge::addFeaibleRange(Range newRange){
    
    std::vector<Range> newFeasible;

    bool isNewRangeNotAdded = true;
    for (const Range& nowRange : feasible)
        if (nowRange.min < newRange.min && nowRange.max < newRange.min ||
            nowRange.min > newRange.max && nowRange.max > newRange.max )
            newFeasible.push_back(nowRange);
        else {
            newRange = {
                MIN(nowRange.min, newRange.min),
                MAX(nowRange.max, newRange.max)
            };
            if (isNewRangeNotAdded)
                newFeasible.push_back(newRange);
            else
                newFeasible.back() = newRange;
            
            isNewRangeNotAdded = false;
        }
    if (isNewRangeNotAdded)
        for (int i = 0; i < newFeasible.size(); i++)
            if (newFeasible[i].min > newRange.min){
                newFeasible.insert(newFeasible.begin() + i, newRange);
                isNewRangeNotAdded = false;
                break;
            }
    if (isNewRangeNotAdded)
        newFeasible.push_back(newRange);

    feasible = std::move(newFeasible);
 }

 bool Edge::checkIsInRange(int start, int end) const {
    int i = 1, n = feasible.size();
    for (const Range& range : feasible){
        int min = range.min + 1;
        int max = range.max - 1;
        if (i == 1 && isLeftMost) 
            min--;
        if (i == n && isRightMost)
            max++;

        if (start >= min && end <= max)
            return true;

        i++;
    }
    return false;
 }