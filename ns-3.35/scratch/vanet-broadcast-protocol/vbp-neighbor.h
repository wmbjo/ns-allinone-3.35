





#ifndef VBPNEIGHBOR_H
#define VBPNEIGHBOR_H

#include <vector>
#include "ns3/simulator.h"
#include "ns3/timer.h"
#include "ns3/ipv4-address.h"
#include "ns3/callback.h"
#include "ns3/arp-cache.h"
#include "vanet-broadcast-protocol.h"
#include <iostream> 
#include <algorithm>
#include "ns3/log.h"
#include "ns3/wifi-mac-header.h"

namespace ns3 {
namespace vbp {

class vbpneighbors : public Object {
    public:
        vbpneighbors ();
        virtual ~vbpneighbors ();
        static TypeId GetTypeId (void);
        virtual TypeId GetInstanceTypeId (void) const;
        virtual void Print (std::ostream &os) const;
        void AppendNeighbor(Ipv4Address neighborAddress);
        
        int  FindNeighbor (uint16_t nodeId); // returns index for specified nodeId, returns -1 if new nodeId
        void AddNode (uint16_t nodeId, uint16_t direction, uint16_t neighborsAhead, uint16_t neighborsBehind
                , float posX, float posY, float speedX, float speedY, float neighborFurthestAheadX, float neighborFurthestAheadY
                , float neighborFurthestBehindX, float neighborFurthestBehindY, float avgSpeedX, float avgSpeedY);
        uint16_t Get1HopNumNeighbors ();       // total number of 1 hop neighbors
        uint16_t Get1HopNumNeighborsAhead ();  // number of neighbors ahead of 1 hop neighbor
        uint16_t Get1HopNumNeighborsBehind (); // number of neighbors behind 1 hop neighbor
        uint16_t Get1HopNeighborIDAhead (uint16_t index);  // id of 1 hop neighbor ahead, grabbed by index
        uint16_t Get1HopNeighborIDBehind (uint16_t index); // id of 1 hop neighbor behind, grabbed by index
        uint16_t Get1HopDirectionById (uint16_t nodeId);   // if 1 hop neighbor ahead or behind, grabbed by nodeId
        uint16_t Get1HopDirection (uint16_t index);        // if 1 hop neighbor ahead or behind, grabbed by index
        uint16_t Get1HopNumberOfNodesAheadOfNeighbor (uint16_t index);  // if 1 hop neighbor has neighbor's ahead
        uint16_t Get1HopNumberOfNodesBehindOfNeighbor (uint16_t index); // if 1 hop neighbor has neighbor's behind
        uint16_t Get1HopNeighborId (uint16_t index);  // get id based on index in vector if neighbor Ids, grabbed by index
        float GetNeighborPositionX (uint16_t index);  // get location, grabbed by index
        float GetNeighborPositionY (uint16_t index);
        float GetNeighborSpeedX (uint16_t index);     // get speed, grabbed by index
        float GetNeighborSpeedY (uint16_t index);
        std::vector<float> Get1HopNeighborLocationsX ();   // get locations for all 1 hop neighbors
        std::vector<float> Get1HopNeighborLocationsY ();
        std::vector<float> Get1HopNeighborSpeedX ();       // get speed for all 1 hop neighbors
        std::vector<float> Get1HopNeighborSpeedY ();
        float GetNeighborFurthestAheadX (uint16_t index);  // location for furthest vehicle ahead for a specific node, grabbed by index
        float GetNeighborFurthestAheadY (uint16_t index); 
        float GetNeighborFurthestBehindX (uint16_t index); // location for furthest vehicle behind for a specific node, grabbed by index
        float GetNeighborFurthestBehindY (uint16_t index);
        float GetAvgSpeedNeighborX (float speedReferenceX);// from neighbors, find average speed
        float GetAvgSpeedNeighborY (float speedReferenceY);
        int GetNeighborFurthestAheadByIndex(std::vector<int> reference);  // find vehicle that is furthest ahead within 1 hop
        int GetNeighborFurthestBehindByIndex(std::vector<int> reference);
        float GetNeighborAvgSpeedX(uint16_t index);        // get average speed of 1 hop neighbor grabbed by index
        float GetNeighborAvgSpeedY(uint16_t index);
        int Get2HopDistFurthestAheadByIndex(std::vector<int> reference);  // discuss working cases, highlight that it handles other cases
        int Get2HopDistFurthestBehindByIndex(std::vector<int> reference);
        uint16_t Get2HopCarCount(int twoHopFurthestAheadIndex, int twoHopFurthestBehindIndex, std::vector<uint16_t> reference);
        uint16_t Get2HopCarCountSelfAhead(int twoHopFurthestAheadIndex, std::vector<uint16_t> reference);
        uint16_t Get2HopCarCountSelfBehind(int twoHopFurthestBehindIndex, std::vector<uint16_t> reference);
        float GetLosCalculation(std::vector<float> referencePos, std::vector<float> referenceVel);
        void PrintNeighborState ();               // will display different info
    private:

        uint16_t m_1HopNumNeighbors;        // total number of neighbors
        uint16_t m_1HopNumNeighborsAhead;   // number of neighbors ahead
        uint16_t m_1HopNumNeighborsBehind;  // number of neighbors behind
        std::vector<Ipv4Address> m_1HopNeighborAddresses;       // store all 1 hop neighborIDs
        std::vector<uint16_t> m_1HopNeighborDirection; // store if neighbor is ahead=+1 or behind=0
        std::vector<uint16_t> m_1HopNeighborIDAhead;   // store each neighborID of neighbors ahead
        std::vector<uint16_t> m_1HopNeighborIDBehind;  // store each neighborID of neighbors behind
        std::vector<uint16_t> m_1HopNumNeighborAhead;  // store num of neighbors ahead for each neighborID
        std::vector<uint16_t> m_1HopNumNeighborBehind; // store num of neighbors behind for each neighborID
        std::vector<float> m_1HopPositionX;            // store X of all neighbors
        std::vector<float> m_1HopPositionY;            // store Y of all neighbors
        std::vector<float> m_1HopNeighborSpeedX;       // store speed in X direction of all neighbors
        std::vector<float> m_1HopNeighborSpeedY;       // store speed in Y direction of all neighbors
        std::vector<Time> m_1HopNeighborLastTime;      // store last time of communication with all neighbors 
        std::vector<float> m_neighborFurthestAheadX;   // store X of furthest ahead neighbor for each ID
        std::vector<float> m_neighborFurthestAheadY;   // store Y of furthest ahead neighbor for each ID
        std::vector<float> m_neighborFurthestBehindX;  // store X of furthest behind neighbor for each ID
        std::vector<float> m_neighborFurthestBehindY;  // store Y of furthest behind neighbor for each ID
        std::vector<float> m_neighborAvgSpeedX;        // store average speed in X direction for each ID
        std::vector<float> m_neighborAvgSpeedY;        // store average speed in Y direction for each ID
        void AddNeighbor (uint16_t nodeId);            // will add entry to m_1HopNeighborIDs
        void AddDirection (uint16_t direction, uint16_t nodeId);       // will add entry to m_neighborDirection
        void AddNumNeighborsAhead (uint16_t numAhead);   // will add entry to m_1HopNumNeighborAhead
        void AddNumNeighborsBehind (uint16_t numBehind); // will add entry to m_1HopNumNeighborBehind
        void AddLocation (float posX, float posY);    // will add entry to m_positionX and m_positionY
        void AddSpeed (float speedX, float speedY);   // will add entry to m_neighborSpeedX and m_neighborSpeedY
        void AddNeighborFurthestAhead (float neighborFurthestAheadX, float neighborFurthestAheadY);
                  // will add entry to m_neighborFurthestAheadX
        void AddNeighborFurthestBehind (float neighborFurthestBehindX, float neighborFurthestBehindY);
                  // will add entry to m_neighborFurthestBehindX
        void AddNeighborAvgSpeed (float neighborAvgSpeedX, float neighborAvgSpeedY);
                  // will add entry to m_neighborFurthestBehindX
        void CheckForNeighborRemoval (); 
                  // will remove neighbor if no response for certain time, specified by TIMEOUT in simulationConfiguration.h
        void ScheduleNeighborRemoval ();  
        // creates call to CheckForNeighborRemoval periodically, specified by NEIGHBOR_REMOVAL_PERIOD in simulationConfiguration.h
        void EraseNeighborIDAheadBehind (uint16_t nodeId, uint16_t direction);  // erase ID and direction from stored data
        void UpdateNeighborIDAheadBehind (uint16_t nodeId, uint16_t direction); // given ID and direction, check entry is correct
        void PrintNeighbors ();               // will display entries in m_1HopNeighborIDs
        void PrintDirections ();              // will display entries in m_1HopNeighborIDs
        void PrintNeighborsAhead ();          // will display neighbor IDs that are ahead
        void PrintNeighborsBehind ();         // will display neighbor IDs that are behind
        void PrintNumNeighborsAheadBehind (); // will display value in m_myNeighborsAhead and m_myNeighborsBehind
        void PrintNeighborDirections ();      // will display entries in m_neighborAhead and m_neighborBehind
        void PrintLocations ();               // will display entries in m_positionX and m_positionY
        void PrintSpeeds();                   // will display entries in m_neighborSpeedX and m_neighborSpeedY
        void PrintTimes ();                   // will display entries in m_neighborLastTime
        void Print1hopFurthestAhead ();       // will display position of neighbor furthest ahead
        void Print1hopFurthestBehind ();      // will display position of neighbor furthest behind
        void PrintAvgSpeeds ();               // will display average speed of 1 hop neighbors
};

}
}
#endif