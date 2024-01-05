#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <queue>
#include <unordered_map>
#include <algorithm>
#include "include/routing.hpp"
#include "include/utils.hpp"


RectilinearChannelRouter::~RectilinearChannelRouter(){
    for (Trunk* trunk : m_trunks) delete trunk;
    for (Track* track : m_tracks) delete track;
    for (Edge* edge : m_upperEdges) delete edge;
    for (Edge* edge : m_lowerEdges) delete edge;
}

void RectilinearChannelRouter::readFromFile(const std::string& inputFilename){
    std::ifstream file(inputFilename);
    std::string   line;

    // Read edges:
    int maxUpperEdgeIdx = -1, maxLowerEdgeIdx = -1;
    while (true) {
        
        std::getline(file, line, '\n');

        int               start, end, idx;
        Edge*             edge;
        std::string       edgeName;
        std::stringstream ss(line);

        if (line[0] == 'T'){
            ss >> edgeName >> start >> end;
            idx = std::stoi(edgeName.substr(1));
            if (idx > maxUpperEdgeIdx){
                maxUpperEdgeIdx = idx;
                m_upperEdges.resize(idx + 1);
            }
            if (m_upperEdges[idx] == nullptr){
                edge = new Edge(start, end, makeNewTrack(edgeName.substr(0, 1), idx));
                m_orderedUpperEdges.push_back(edge);
                m_upperEdges[idx] = edge;
            }
            else
                m_upperEdges[idx]->addFeaibleRange({start, end});
        }
        else if (line[0] == 'B'){
            ss >> edgeName >> start >> end;
            idx = std::stoi(edgeName.substr(1));
            if (idx > maxLowerEdgeIdx){
                maxLowerEdgeIdx = idx;
                m_lowerEdges.resize(idx + 1);
            }
            if (m_lowerEdges[idx] == nullptr){
                edge = new Edge(start, end, makeNewTrack(edgeName.substr(0, 1), idx));
                m_orderedLowerEdges.push_back(edge);
                m_lowerEdges[idx] = edge;
            }
            else
                m_lowerEdges[idx]->addFeaibleRange({start, end});
        }
        else 
            break;
    }
    m_orderedUpperEdges.front()->isLeftMost = true;
    m_orderedUpperEdges.back()->isRightMost = true;
    m_orderedLowerEdges.front()->isLeftMost = true;
    m_orderedLowerEdges.back()->isRightMost = true;
    
    // Read nets:
    std::unordered_map<int, std::pair<int, Trunk*>> terminalHeadMap;
    std::stringstream ssUpper(line);
    std::stringstream ssLower;
    std::getline(file, line, '\n');
    ssLower << line;

    int terminalUpper, terminalLower, nowPosition = 0;
    auto nowUpperEdge = m_orderedUpperEdges.begin();
    auto nowLowerEdge = m_orderedLowerEdges.begin();
    while (ssUpper >> terminalUpper){
        ssLower >> terminalLower;
        bool isUpperTerminal  = terminalUpper != 0;
        bool isLowerTerminal  = terminalLower != 0;
        Trunk* upperTrunk     = nullptr;
        Trunk* lowerTrunk     = nullptr;
        Trunk* upperTrunkLast = nullptr;
        Trunk* lowerTrunkLast = nullptr;
        if (isUpperTerminal){
            if (terminalHeadMap.count(terminalUpper)){
                upperTrunk = terminalHeadMap[terminalUpper].second;
                upperTrunk->end = nowPosition;
                m_trunks.push_back(upperTrunk);
                upperTrunkLast = upperTrunk;
            }
            terminalHeadMap[terminalUpper] = {
                nowPosition, 
                (upperTrunk = makeNewTrunk(terminalUpper, nowPosition, -1))
            };
            (*nowUpperEdge)->terminalCountMap[upperTrunk]++;
        }
        if (isLowerTerminal){
            if (terminalHeadMap.count(terminalLower)){
                lowerTrunk = terminalHeadMap[terminalLower].second;
                lowerTrunk->end = nowPosition;
                m_trunks.push_back(lowerTrunk);
                lowerTrunkLast = lowerTrunk;
            }
            terminalHeadMap[terminalLower] = {
                nowPosition, 
                (lowerTrunk = makeNewTrunk(terminalLower, nowPosition, -1))
            };
            (*nowLowerEdge)->terminalCountMap[lowerTrunk]++;
        }
        if (isUpperTerminal && isLowerTerminal){
            m_positiveVCG[upperTrunk].insert(lowerTrunk);
            m_negativeVCG[lowerTrunk].insert(upperTrunk);
            if (lowerTrunkLast){
                m_positiveVCG[upperTrunk].insert(lowerTrunkLast);
                m_negativeVCG[lowerTrunkLast].insert(upperTrunk);
            }
            if (upperTrunkLast){
                m_negativeVCG[lowerTrunk].insert(upperTrunkLast);
                m_positiveVCG[upperTrunkLast].insert(lowerTrunk);
            }
            if (upperTrunkLast && lowerTrunkLast){
                m_positiveVCG[upperTrunkLast].insert(lowerTrunkLast);
                m_negativeVCG[lowerTrunkLast].insert(upperTrunkLast);
            }
        }

        nowPosition++;
        if (nowPosition > (*nowUpperEdge)->max) nowUpperEdge++;
        if (nowPosition > (*nowLowerEdge)->max) nowLowerEdge++;
    }

    // Delete unused trunk objects:
    freeUnusedTrunk(terminalHeadMap);

    // Pad graph:
    padGraph(m_positiveVCG);
    padGraph(m_negativeVCG);

    // Find feasible sections of each on-edge track:
    for (int u = m_upperEdges.size() - 2; u >= 0; u--)
        for (const Range& feasible : m_upperEdges[u + 1]->feasible)
            m_upperEdges[u]->addFeaibleRange(feasible);
    for (int l = m_lowerEdges.size() - 2; l >= 0; l--)
        for (const Range& feasible : m_lowerEdges[l + 1]->feasible)
            m_lowerEdges[l]->addFeaibleRange(feasible);

    file.close();
    END_LINE
    LOG("Finish reading.")
}

void RectilinearChannelRouter::outputToFile(const std::string& outputFilename){
    std::ofstream file(outputFilename);
    file << "Channel density: " << m_additionTrack << std::endl;
    for (int net = 1; net <= m_trunkNumberMap.size(); net++){
        file << "Net " << net << std::endl;
        std::vector<int> doglegs;
        for (const Trunk* trunk : m_trunkNumberMap[net]){
            std::string trunkName = (trunk->track->isAddition())? trunk->track->getName(m_additionTrack) : trunk->track->getName();
            file << trunkName << " " << trunk->start << " " << trunk->end << std::endl;
            doglegs.push_back(trunk->end);
        }
        for (int i = 0; i < doglegs.size() - 1; i++)
            file << "Dogleg " << doglegs[i] << std::endl;
    }
    file.close();
    END_LINE
    LOG("Done.")
}

void RectilinearChannelRouter::run(){
    for (int u = m_upperEdges.size() - 1; u > 0; u--)
        routeTrack(m_positiveVCG, m_negativeVCG, m_upperEdges[u], m_upperEdges[u - 1]->track);
    for (int l = m_lowerEdges.size() - 1; l > 0; l--)
        routeTrack(m_negativeVCG, m_positiveVCG, m_lowerEdges[l], m_lowerEdges[l - 1]->track);
    
    while (!m_positiveVCG.empty()){
        routeTrack(m_positiveVCG, m_negativeVCG, makeNewTrack("C", m_additionTrack));
        m_additionTrack++;
    }
}

void RectilinearChannelRouter::routeTrack(DirectedGraph& positiveVCG, DirectedGraph& negativeVCG, const Edge* edge, const Track* track){
    Trunk             *trunk, *prevTrunk;
    MinQueue<TrunkPtr> pq;

    buildMinQueueFromGraph(positiveVCG, pq);

    prevTrunk = nullptr;
    while (!pq.empty()){
        trunk = pq.top().ptr;
        pq.pop();
        if (!(negativeVCG[trunk].empty() &&
              checkIsFeasibleStart(trunk, prevTrunk) &&
              edge->checkIsInRange(trunk->start, trunk->end))) 
            continue;

        trunk->track = track;
        prevTrunk = trunk;
        removeTrunkFromGraphs(positiveVCG, negativeVCG, trunk);
    }
}
void RectilinearChannelRouter::routeTrack(DirectedGraph& positiveVCG, DirectedGraph& negativeVCG, const Track* track){
    Trunk             *trunk, *prevTrunk;
    MinQueue<TrunkPtr> pq;

    buildMinQueueFromGraph(positiveVCG, pq);

    prevTrunk = nullptr;
    while (!pq.empty()){
        trunk = pq.top().ptr;
        pq.pop();
        if (!(negativeVCG[trunk].empty() &&
              checkIsFeasibleStart(trunk, prevTrunk))) 
            continue;

        trunk->track = track;
        prevTrunk = trunk;
        removeTrunkFromGraphs(positiveVCG, negativeVCG, trunk);
    }
}

void RectilinearChannelRouter::removeTrunkFromGraphs(DirectedGraph& positiveVCG, DirectedGraph& negativeVCG, Trunk* trunk){
    for (Trunk* child : positiveVCG[trunk])
        negativeVCG[child].erase(trunk);
        
    positiveVCG.erase(trunk);
    negativeVCG.erase(trunk);
}

void RectilinearChannelRouter::buildMinQueueFromGraph(const DirectedGraph& graph, MinQueue<TrunkPtr>& pq){
    for (auto& kv : graph)
        pq.push(kv.first);
}

bool RectilinearChannelRouter::checkIsFeasibleStart(const Trunk* currTrunk, const Trunk* prevTrunk){
    if (prevTrunk == nullptr) return true;
    if (currTrunk->number == prevTrunk->number && currTrunk->start >= prevTrunk->end) return true;
    if (currTrunk->number != prevTrunk->number && currTrunk->start >  prevTrunk->end) return true;
    return false;
}

Trunk* RectilinearChannelRouter::makeNewTrunk(int number, int start, int end){
    Trunk* trunk = new Trunk(number, start, end);
    m_trunkNumberMap[number].push_back(trunk);
    return trunk;
}

Track* RectilinearChannelRouter::makeNewTrack(const std::string& type, int number){
    Track* track = new Track(type, number);
    m_tracks.push_back(track);
    return track;
}

/************************************************* Some helper methods for reading data *************************************************/
void RectilinearChannelRouter::padGraph(DirectedGraph& graph){
    for (Trunk* trunk : m_trunks)
        graph[trunk];
}

void RectilinearChannelRouter::removeUnusedTrunkFromGraph(DirectedGraph& graph){
    for (auto itg = graph.begin(); itg != graph.end(); )
        if ((*itg).first->end == -1)
            itg = graph.erase(itg);
        else {
            for (auto it = (*itg).second.begin(); it != (*itg).second.end(); )
                if ((*it)->end == -1) it = (*itg).second.erase(it);
                else                  it++;
            itg++;
        }
}

void RectilinearChannelRouter::freeUnusedTrunk(std::unordered_map<int, std::pair<int, Trunk*>>& terminalHeadMap){

    removeUnusedTrunkFromGraph(m_positiveVCG);
    removeUnusedTrunkFromGraph(m_negativeVCG);
    
    for (auto& kv : m_trunkNumberMap)
        for (auto it = kv.second.begin(); it != kv.second.end(); )
            if ((*it)->end == -1)
                it = kv.second.erase(it);
            else
                it++;

    for (auto& kv : terminalHeadMap)
        if (kv.second.second->end == -1)
            delete kv.second.second;
}

/************************************************* Debug tools *************************************************/
#if DEBUG == 1
void RectilinearChannelRouter::printGraphs(){
    END_LINE
    LOG("Positive Graph:")
    for (const auto& kv : m_positiveVCG){
        std::cout << "[Trunk " << kv.first->getName() << "] -> ";
        for (const Trunk* trunk_ : kv.second){
            std::cout << trunk_->getName() << ", ";
        }
        END_LINE
    }
    END_LINE
    LOG("Negative Graph:")
    for (const auto& kv : m_negativeVCG){
        std::cout << "[Trunk " << kv.first->getName() << "] -> ";
        for (const Trunk* trunk_ : kv.second){
            std::cout << trunk_->getName() << ", ";
        }
        END_LINE
    }
}

void RectilinearChannelRouter::printTrunkInfo(){
    END_LINE
    LOG("Trunks:")
    for (const Trunk* trunk : m_trunks)
        trunk->printInfo();
}

void RectilinearChannelRouter::printTrackInfo(){

    int nUpper = m_upperEdges.size();
    int nLower = m_lowerEdges.size();

    END_LINE
    LOG("Tracks:")
    LOGKV(nUpper)
    LOGKV(nLower)
    for (int u = nUpper - 1; u >= 0; u--){
        const Edge* edge = m_upperEdges[u];
        if (edge == nullptr) continue;
        std::cout << "[" << edge->track->getName() << "] : Range = (" << edge->min << ", " << edge->max << "), Feasible = [";
        for (const Range& feasible : edge->feasible)
            std::cout << "(" << feasible.min << ", " << feasible.max << ") ";
        
        std::cout << "\b]";
        END_LINE
    }
    for (int l = nLower - 1; l >= 0; l--){
        const Edge* edge = m_lowerEdges[l];
        if (edge == nullptr) continue;
        std::cout << "[" << edge->track->getName() << "] : Range = (" << edge->min << ", " << edge->max << "), Feasible = [";
        for (const Range& feasible : edge->feasible)
            std::cout << "(" << feasible.min << ", " << feasible.max << ") ";
        
        std::cout << "\b]";
        END_LINE
    }
}
#endif