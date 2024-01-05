#pragma once

#include <vector>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "structure.hpp"
#include "utils.hpp"


class RectilinearChannelRouter {
public:
    typedef std::unordered_map<Trunk*, std::unordered_set<Trunk*>> DirectedGraph;
    typedef std::unordered_map<int, std::vector<Trunk*>>           TrunkNumberMap;

public:
    int                 m_additionTrack = 0;
    std::vector<Trunk*> m_trunks;
    std::vector<Track*> m_tracks;
    std::vector<Edge*>  m_upperEdges;
    std::vector<Edge*>  m_lowerEdges;

    std::vector<Edge*>  m_orderedUpperEdges;
    std::vector<Edge*>  m_orderedLowerEdges;

    DirectedGraph       m_positiveVCG;
    DirectedGraph       m_negativeVCG;

    TrunkNumberMap      m_trunkNumberMap;

public:
    RectilinearChannelRouter() = default;
    ~RectilinearChannelRouter();

public:
    void readFromFile(const std::string& inputFilename);
    void outputToFile(const std::string& outputFilename);

    void run();

#if DEBUG == 1
    void printGraphs   ();
    void printTrunkInfo();
    void printTrackInfo();
#endif

private:
    void routeTrack(DirectedGraph& positiveVCG, DirectedGraph& negativeVCG, const Track* track);
    void routeTrack(DirectedGraph& positiveVCG, DirectedGraph& negativeVCG, const Edge* edge, const Track* track);

    void removeTrunkFromGraphs (DirectedGraph& positiveVCG, DirectedGraph& negativeVCG, Trunk* trunk);
    void buildMinQueueFromGraph(const DirectedGraph& graph, MinQueue<TrunkPtr>& pq);
    bool checkIsFeasibleStart  (const Trunk* currTrunk, const Trunk* prevTrunk);
    
    Trunk* makeNewTrunk              (int number, int start, int end = -1, bool isAddToTrunkVector = true);
    Track* makeNewTrack              (const std::string& type, int number);
    void   removeUnusedTrunkFromGraph(DirectedGraph& graph);
    void   padGraph                  (DirectedGraph& graph);
    void   freeUnusedTrunk           (std::unordered_map<int, std::pair<int, Trunk*>>& terminalHeadMap);
};