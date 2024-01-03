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