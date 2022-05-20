#ifndef NEIGHBORS_H
#define NEIGHBORS_H

#include <iostream>
#include <vector>
#include "simulationConfiguration.h" 

using namespace ns3;

class neighbors : public Object {
    public:
        neighbors ();
        virtual ~neighbors ();
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
        int GetNeighborFurthestAheadByIndex(Vector reference);  // find vehicle that is furthest ahead within 1 hop
        int GetNeighborFurthestBehindByIndex(Vector reference);
        float GetNeighborAvgSpeedX(uint16_t index);        // get average speed of 1 hop neighbor grabbed by index
        float GetNeighborAvgSpeedY(uint16_t index);
        int Get2HopDistFurthestAheadByIndex(Vector reference);  // discuss working cases, highlight that it handles other cases
        int Get2HopDistFurthestBehindByIndex(Vector reference);
        uint16_t Get2HopCarCount(int twoHopFurthestAheadIndex, int twoHopFurthestBehindIndex, Vector reference);
        uint16_t Get2HopCarCountSelfAhead(int twoHopFurthestAheadIndex, Vector reference);
        uint16_t Get2HopCarCountSelfBehind(int twoHopFurthestBehindIndex, Vector reference);
        float GetLosCalculation(Vector referencePos, Vector referenceVel);
        void PrintNeighborState ();               // will display different info
    private:
        uint16_t m_1HopNumNeighbors;        // total number of neighbors
        uint16_t m_1HopNumNeighborsAhead;   // number of neighbors ahead
        uint16_t m_1HopNumNeighborsBehind;  // number of neighbors behind
        std::vector<uint16_t> m_1HopNeighborIDs;       // store all 1 hop neighborIDs
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

neighbors::neighbors () {
    ScheduleNeighborRemoval ();
}

neighbors::~neighbors () {
}

int neighbors::FindNeighbor (uint16_t nodeId) {
    for(uint16_t idx = 0; idx < m_1HopNumNeighbors; ++idx) {
        if (m_1HopNeighborIDs[idx] == nodeId) {
            // if find value to add is already here, then stop
            return idx; // what index needs to be used for this ip address
        }
    }
    return -1; // if not found
}

void neighbors::AddNeighbor (uint16_t nodeId) {
    m_1HopNeighborIDs.push_back(nodeId); // neighbors identified by nodeId
}

void neighbors::AddDirection (uint16_t direction, uint16_t nodeId) {
    m_1HopNeighborDirection.push_back(direction);
    if (direction) { // true if 1, which is ahead
        m_1HopNumNeighborsAhead++;
        m_1HopNeighborIDAhead.push_back(nodeId);
    } else {
        m_1HopNumNeighborsBehind++;
        m_1HopNeighborIDBehind.push_back(nodeId);
    }
}

void neighbors::AddNumNeighborsAhead (uint16_t numAhead) {
    m_1HopNumNeighborAhead.push_back(numAhead); 
}

void neighbors::AddNumNeighborsBehind (uint16_t numBehind) {
    m_1HopNumNeighborBehind.push_back(numBehind); 
}

void neighbors::AddLocation (float posX, float posY) {
    m_1HopPositionX.push_back(posX);
    m_1HopPositionY.push_back(posY);
}

void neighbors::AddSpeed(float speedX, float speedY) {
    m_1HopNeighborSpeedX.push_back(speedX); 
    m_1HopNeighborSpeedY.push_back(speedY); 
}
void neighbors::AddNeighborFurthestAhead (float neighborFurthestAheadX, float neighborFurthestAheadY) {
    m_neighborFurthestAheadX.push_back(neighborFurthestAheadX);
    m_neighborFurthestAheadY.push_back(neighborFurthestAheadY);
}

void neighbors::AddNeighborFurthestBehind (float neighborFurthestBehindX, float neighborFurthestBehindY) {
    m_neighborFurthestBehindX.push_back(neighborFurthestBehindX);
    m_neighborFurthestBehindY.push_back(neighborFurthestBehindY);
}

void neighbors::AddNeighborAvgSpeed (float neighborAvgSpeedX, float neighborAvgSpeedY) {
    m_neighborAvgSpeedX.push_back(neighborAvgSpeedX);
    m_neighborAvgSpeedY.push_back(neighborAvgSpeedY);
}
        
void neighbors::AddNode (uint16_t nodeId, uint16_t direction, uint16_t neighborsAhead, uint16_t neighborsBehind
                            , float posX, float posY, float speedX, float speedY, float neighborFurthestAheadX
                            , float neighborFurthestAheadY, float neighborFurthestBehindX, float neighborFurthestBehindY
                            , float avgSpeedX, float avgSpeedY) {
   int idx = FindNeighbor(nodeId);
   if (idx < 0) { // if added the neighbor, add to list
        AddNeighbor(nodeId);
        AddDirection(direction, nodeId);
        AddNumNeighborsAhead(neighborsAhead);
        AddNumNeighborsBehind(neighborsBehind);
        AddLocation(posX, posY);
        AddSpeed(speedX, speedY);
        m_1HopNeighborLastTime.push_back(Simulator::Now());
        m_1HopNumNeighbors++; // update number of neighbors by one
        AddNeighborFurthestAhead (neighborFurthestAheadX, neighborFurthestAheadY);  // will add entry to m_neighborFurthestAheadX
        AddNeighborFurthestBehind (neighborFurthestBehindX, neighborFurthestBehindY);  // will add entry to m_neighborFurthestBehindX
        AddNeighborAvgSpeed (avgSpeedX, avgSpeedY);  // will add entry to m_neighborAvgSpeedX
        // start with header first then this part!!!
        //need to add x,y furthest and FurthestAhead (4) and average speed
        // when this is complete, in routing compute LOS, the value used in LOS is not accurate since average of averages
        // average will be skewed to vehicles in the middle
   }
   else {
        // update vectors if not adding a new neighbor
        m_1HopNeighborDirection[idx] = direction;
        UpdateNeighborIDAheadBehind(nodeId, direction);
        m_1HopNumNeighborAhead[idx] = neighborsAhead;
        m_1HopNumNeighborBehind[idx] = neighborsBehind;
        m_1HopPositionX[idx] = posX; 
        m_1HopPositionY[idx] = posY; 
        m_1HopNeighborSpeedX[idx] = speedX;
        m_1HopNeighborSpeedY[idx] = speedY;
        m_1HopNeighborLastTime[idx] = Simulator::Now(); 
        m_neighborFurthestAheadX[idx] = neighborFurthestAheadX;
        m_neighborFurthestAheadY[idx] = neighborFurthestAheadY;
        m_neighborFurthestBehindX[idx] = neighborFurthestBehindX;
        m_neighborFurthestBehindY[idx] = neighborFurthestBehindY;
        m_neighborAvgSpeedX[idx] = avgSpeedX;
        m_neighborAvgSpeedY[idx] = avgSpeedY;
   }
}

void neighbors::CheckForNeighborRemoval () {
    // delete neighbor if no response in TIMEOUT seconds
    if (m_1HopNumNeighbors==0) {
        return; // nothing to remove
    }
    for(uint16_t idx = 0; idx < m_1HopNumNeighbors; ++idx) {    
         if ((Simulator::Now()-m_1HopNeighborLastTime[idx]) >= NEIGHBOR_TIMEOUT) {  //check seconds since last response
            // if find value to delete
            EraseNeighborIDAheadBehind (m_1HopNeighborIDs[idx], m_1HopNeighborDirection[idx]);
            m_1HopNeighborIDs.erase(m_1HopNeighborIDs.begin()+idx);  
            m_1HopNeighborDirection.erase(m_1HopNeighborDirection.begin()+idx);   
            m_1HopNumNeighborAhead.erase(m_1HopNumNeighborAhead.begin()+idx);
            m_1HopNumNeighborBehind.erase(m_1HopNumNeighborBehind.begin()+idx);
            m_1HopPositionX.erase(m_1HopPositionX.begin()+idx);  
            m_1HopPositionY.erase(m_1HopPositionY.begin()+idx);  
            m_1HopNeighborSpeedX.erase(m_1HopNeighborSpeedX.begin()+idx); // can't use same iterator for all since different types
            m_1HopNeighborSpeedY.erase(m_1HopNeighborSpeedY.begin()+idx); 
            m_1HopNeighborLastTime.erase(m_1HopNeighborLastTime.begin()+idx); 
            m_neighborFurthestAheadX.erase(m_neighborFurthestAheadX.begin()+idx);
            m_neighborFurthestAheadY.erase(m_neighborFurthestAheadY.begin()+idx);
            m_neighborFurthestBehindX.erase(m_neighborFurthestBehindX.begin()+idx);
            m_neighborFurthestBehindY.erase(m_neighborFurthestBehindY.begin()+idx);
            m_neighborAvgSpeedX.erase(m_neighborAvgSpeedX.begin()+idx);
            m_neighborAvgSpeedY.erase(m_neighborAvgSpeedY.begin()+idx);
            m_1HopNumNeighbors--; // decrease neighborSize count
        }
    }
}

void neighbors::ScheduleNeighborRemoval () {
    CheckForNeighborRemoval ();
    Simulator::Schedule(Seconds(NEIGHBOR_REMOVAL_PERIOD), &neighbors::ScheduleNeighborRemoval, this);
}

void neighbors::EraseNeighborIDAheadBehind (uint16_t nodeId, uint16_t direction) {
    if (direction) { // means remove from neighbor ahead
        for(uint16_t idx = 0; idx < m_1HopNumNeighborsAhead; ++idx) {
            if (m_1HopNeighborIDAhead[idx] == nodeId) {
                m_1HopNeighborIDAhead.erase(m_1HopNeighborIDAhead.begin()+idx);
            }
        }
        m_1HopNumNeighborsAhead--;
    }
    else { //means remove from neighbor behind
        for(uint16_t idx = 0; idx < m_1HopNumNeighborsBehind; ++idx) {
            if (m_1HopNeighborIDBehind[idx] == nodeId) {
                m_1HopNeighborIDBehind.erase(m_1HopNeighborIDBehind.begin()+idx);
            }
        }
        m_1HopNumNeighborsBehind--;
    }   
}

void neighbors::UpdateNeighborIDAheadBehind (uint16_t nodeId, uint16_t direction) {
    for(uint16_t idx = 0; idx < m_1HopNumNeighborsAhead; ++idx) {
        if (m_1HopNeighborIDAhead[idx] == nodeId) {
            // if find nodeId check if direction changed
            if (direction) {
                return; // direction stayed the same so no changes
            }
            else { //means direction changed
                m_1HopNumNeighborsAhead--;
                m_1HopNumNeighborsBehind++;
                m_1HopNeighborIDAhead.erase(m_1HopNeighborIDAhead.begin()+idx);
                m_1HopNeighborIDBehind.push_back(nodeId);
                return;
            }
        }
    }
    for(uint16_t idx = 0; idx < m_1HopNumNeighborsBehind; ++idx) {
        if (m_1HopNeighborIDBehind[idx] == nodeId) {
            // if find nodeId check if direction changed
            if (!direction) {
                return; // direction stayed the same so no changes
            }
            else { //means direction changed
                m_1HopNumNeighborsAhead++;
                m_1HopNumNeighborsBehind--;
                m_1HopNeighborIDAhead.push_back(nodeId);
                m_1HopNeighborIDBehind.erase(m_1HopNeighborIDBehind.begin()+idx);
                return;
            }
        }
    } 
}

uint16_t neighbors::Get1HopNumNeighbors () {
    return m_1HopNumNeighbors;
}

uint16_t neighbors::Get1HopNumNeighborsAhead () {
    return m_1HopNumNeighborsAhead;
}

uint16_t neighbors::Get1HopNumNeighborsBehind () {
    return m_1HopNumNeighborsBehind;
}

uint16_t neighbors::Get1HopNeighborIDAhead (uint16_t index) {
    return m_1HopNeighborIDAhead[index];
}

uint16_t neighbors::Get1HopNeighborIDBehind (uint16_t index) {
    return m_1HopNeighborIDBehind[index];
}

uint16_t neighbors::Get1HopDirectionById (uint16_t nodeId) {
    int idx = FindNeighbor (nodeId); 
    if (idx >= 0) {
 	return m_1HopNeighborDirection[idx];
    }
    else {
        return 0;
    }
}

uint16_t neighbors::Get1HopDirection (uint16_t index) {
 	return m_1HopNeighborDirection[index];
}

uint16_t neighbors::Get1HopNumberOfNodesAheadOfNeighbor (uint16_t index) {
 	return m_1HopNumNeighborAhead[index];
}

uint16_t neighbors::Get1HopNumberOfNodesBehindOfNeighbor (uint16_t index) {
 	return m_1HopNumNeighborBehind[index];
}

uint16_t neighbors::Get1HopNeighborId (uint16_t index) {
    return m_1HopNeighborIDs[index];
}

float neighbors::GetNeighborPositionX (uint16_t index) {
 	return m_1HopPositionX[index];
}

float neighbors::GetNeighborPositionY (uint16_t index) {
 	return m_1HopPositionY[index];
}

float neighbors::GetNeighborSpeedX (uint16_t index) {
 	return m_1HopNeighborSpeedX[index];
}

float neighbors::GetNeighborSpeedY (uint16_t index) {
 	return m_1HopNeighborSpeedY[index];
}

std::vector<float> neighbors::Get1HopNeighborLocationsX () {
    return m_1HopPositionX;
}

std::vector<float> neighbors::Get1HopNeighborLocationsY () {
    return m_1HopPositionY;
}

std::vector<float> neighbors::Get1HopNeighborSpeedX () {
    return m_1HopNeighborSpeedX;
}

std::vector<float> neighbors::Get1HopNeighborSpeedY () {
    return m_1HopNeighborSpeedY;
}

float neighbors::GetNeighborFurthestAheadX(uint16_t index) {
   return m_neighborFurthestAheadX[index];
}

float neighbors::GetNeighborFurthestAheadY(uint16_t index) {
   return m_neighborFurthestAheadY[index];
}

float neighbors::GetNeighborFurthestBehindX(uint16_t index) {
   return m_neighborFurthestBehindX[index];
}

float neighbors::GetNeighborFurthestBehindY(uint16_t index) {
   return m_neighborFurthestBehindY[index];
}

float neighbors::GetNeighborAvgSpeedX(uint16_t index) {
    return m_neighborAvgSpeedX[index];
}

float neighbors::GetNeighborAvgSpeedY(uint16_t index) {
    return m_neighborAvgSpeedY[index];
}

float neighbors::GetAvgSpeedNeighborX(float speedReferenceX) {
    float avgSpeedX = speedReferenceX;
    if (m_1HopNumNeighbors > 0) {
        for(uint16_t idx = 0; idx < m_1HopNumNeighbors; idx++) { 
            avgSpeedX += m_1HopNeighborSpeedX[idx];
        }
        avgSpeedX = avgSpeedX/(m_1HopNumNeighbors+1);
    }
    return avgSpeedX;
}

float neighbors::GetAvgSpeedNeighborY(float speedReferenceY) {
    float avgSpeedY = speedReferenceY;
    if (m_1HopNumNeighbors > 0) {
        for(uint16_t idx = 0; idx < m_1HopNumNeighbors; idx++) { 
            avgSpeedY += m_1HopNeighborSpeedY[idx];
        }
        avgSpeedY = avgSpeedY/(m_1HopNumNeighbors+1);
    }
    return avgSpeedY;
}

int neighbors::GetNeighborFurthestAheadByIndex(Vector reference) {
    int furthestIdx = -1;
    if (m_1HopNumNeighborsAhead > 0) {
        furthestIdx = FindNeighbor(m_1HopNeighborIDAhead[0]); // in case only has one neighbor ahead
        Vector neighborPos = Vector3D(m_1HopPositionX[furthestIdx], m_1HopPositionY[furthestIdx],0);
        float distFurthestAhead = CalculateDistance(neighborPos, reference); // car more ahead  
        float neighborDist;
        for(uint16_t idx = 0; idx < m_1HopNumNeighbors; idx++) { 
            if (!(m_1HopNeighborDirection[idx])) { // car is behind, then skip
                continue;
            }
            neighborPos = Vector3D(m_1HopPositionX[idx], m_1HopPositionY[idx],0);
            neighborDist = CalculateDistance(neighborPos, reference);
            if (neighborDist > distFurthestAhead) { // car most head 
                distFurthestAhead = neighborDist;
                furthestIdx = idx;
            }                                           
        }                 
    }
    return furthestIdx;
}

int neighbors::GetNeighborFurthestBehindByIndex(Vector reference) {
    int furthestIdx = -1;
    if (m_1HopNumNeighborsBehind > 0) {
        furthestIdx = FindNeighbor(m_1HopNeighborIDBehind[0]); // in case only has one neighbor behind
        Vector neighborPos = Vector3D(m_1HopPositionX[furthestIdx], m_1HopPositionY[furthestIdx],0);
        float distFurthestBehind = CalculateDistance(neighborPos, reference); // car more ahead  
        float neighborDist;
        for(uint16_t idx = 0; idx < m_1HopNumNeighbors; idx++) { 
            if (m_1HopNeighborDirection[idx]) { // car is ahead, then skip
                continue;
            }
            neighborPos = Vector3D(m_1HopPositionX[idx], m_1HopPositionY[idx],0);
            neighborDist = CalculateDistance(neighborPos, reference);
            if (neighborDist > distFurthestBehind) {  //car most behind
                distFurthestBehind = neighborDist;
                furthestIdx = idx;
            }                                           
        }                 
    }
    return furthestIdx;
}

int neighbors::Get2HopDistFurthestAheadByIndex(Vector reference) {
    int furthestIndex = -1;
    float furthestDist = 0;
    if (m_1HopNumNeighborsAhead > 0) {
        uint16_t index = FindNeighbor(m_1HopNeighborIDAhead[0]);
        Vector neighborFurthestAhead = Vector3D(m_neighborFurthestAheadX[index], m_neighborFurthestAheadY[index],0);        
        if (!isnan(neighborFurthestAhead.x)) { //only need to check x because x and y will be nan
            furthestDist = CalculateDistance(reference, neighborFurthestAhead); // Dist ahead
            furthestIndex = index; // in case only has one neighbor
        }
        float neighborDist; 
        for(uint16_t idx = 0; idx < m_1HopNumNeighbors; idx++) { 
            if (!(m_1HopNeighborDirection[idx])) { // car is behind, then skip
                continue;
            }
            neighborFurthestAhead = Vector3D(m_neighborFurthestAheadX[idx], m_neighborFurthestAheadY[idx],0);
            if (isnan(neighborFurthestAhead.x)) { //only need to check x because x and y will be nan
                continue; // go to next entry
            }
            neighborDist = CalculateDistance(reference, neighborFurthestAhead); // Dist ahead
            if (neighborDist > furthestDist) { // car and car furthest head total distance
                furthestDist = neighborDist;
                furthestIndex = idx;
            }   //consider when the vector is not a number, check if nan                                  
        }                 
    } // index of 1 hop neighbor that reaches furthest ahead
    return furthestIndex; // -1 when no 2 hop neighbor ahead or no 1 hop neighbors ahead
}

int neighbors::Get2HopDistFurthestBehindByIndex(Vector reference) {
    int furthestIndex = -1;
    float furthestDist = 0;
    if (m_1HopNumNeighborsBehind > 0) {
        uint16_t index = FindNeighbor(m_1HopNeighborIDBehind[0]);
        Vector neighborFurthestBehind = Vector3D(m_neighborFurthestBehindX[index], m_neighborFurthestBehindY[index],0);
        if (!isnan(neighborFurthestBehind.x)) { //only need to check x because x and y will be nan
            furthestDist = CalculateDistance(reference, neighborFurthestBehind);  // dist behind
            furthestIndex = index; // in case only has one neighbor
        }
        float neighborDist; 
        for(uint16_t idx = 0; idx < m_1HopNumNeighbors; idx++) { 
            if (m_1HopNeighborDirection[idx]) { // car is ahead, then skip
                continue;
            }
            neighborFurthestBehind = Vector3D(m_neighborFurthestBehindX[idx], m_neighborFurthestBehindY[idx],0);
            if (isnan(neighborFurthestBehind.x)) { //only need to check x because x and y will be nan
                continue; // go to next entry
            }
            neighborDist = CalculateDistance(reference, neighborFurthestBehind);  // dist behind
            if (neighborDist > furthestDist) {  // car and car furthest behind total distance
                furthestDist = neighborDist;
                furthestIndex = idx;
            }                                           
        }                 
    } // index of 1 hop neighbor that reaches furthest behind
    return furthestIndex; // -1 when no 2 hop neighbor behind or no 1 hop neighbors behind
}
 
uint16_t neighbors::Get2HopCarCount(int twoHopFurthestAheadIndex, int twoHopFurthestBehindIndex, Vector reference) {
        // 3 regions, cars ahead of 2hopFurthestAheadIdx, cars behind 2hopFurthestBehindIdx, and cars between those 2 Ids
    uint16_t totalCount = m_1HopNumNeighborAhead[twoHopFurthestAheadIndex];
    totalCount += m_1HopNumNeighborBehind[twoHopFurthestBehindIndex]; // first two regions
    Vector furthestAhead = Vector3D(m_1HopPositionX[twoHopFurthestAheadIndex], m_1HopPositionY[twoHopFurthestAheadIndex],0);
    float furthestAheadDist = CalculateDistance(furthestAhead, reference);
    Vector furthestBehind = Vector3D(m_1HopPositionX[twoHopFurthestBehindIndex], m_1HopPositionY[twoHopFurthestBehindIndex],0);
    float furthestBehindDist = CalculateDistance(furthestBehind, reference);
    Vector neighborPos; 
    float neighborDist;
    for(uint16_t idx = 0; idx < m_1HopNumNeighbors; idx++) { 
        neighborPos = Vector3D(m_1HopPositionX[idx], m_1HopPositionY[idx],0);
        neighborDist = CalculateDistance(neighborPos, reference);
        if (m_1HopNeighborDirection[idx]) { // car is ahead
            if (neighborDist <= furthestAheadDist) {
                totalCount++;
            }
        }
        else { // then behind
            if (neighborDist <= furthestBehindDist) {
                totalCount++;
            }
        }                
    }
    totalCount += 1; //need to add one to account current node
    return totalCount; 
}

uint16_t neighbors::Get2HopCarCountSelfAhead(int twoHopFurthestAheadIndex, Vector reference) {
        // 3 regions, vehicles behind reference, vehicles in front furthest ahead, vehicles between reference and furthest ahead
    uint16_t totalCount = m_1HopNumNeighborsBehind; 
    totalCount += m_1HopNumNeighborAhead[twoHopFurthestAheadIndex];
    Vector furthestAhead = Vector3D(m_1HopPositionX[twoHopFurthestAheadIndex], m_1HopPositionY[twoHopFurthestAheadIndex],0);
    float furthestAheadDist = CalculateDistance(furthestAhead, reference);
    Vector neighborPos; 
    float neighborDist;
    for(uint16_t idx = 0; idx < m_1HopNumNeighbors; idx++) {  // also check if 1hopneighbors>0 first
        if (!(m_1HopNeighborDirection[idx])) { // car is behind, then skip
            continue;
        }
        neighborPos = Vector3D(m_1HopPositionX[idx], m_1HopPositionY[idx],0);
        neighborDist  = CalculateDistance(neighborPos, reference);
        if (neighborDist <= furthestAheadDist) {
            totalCount++;
        }               
    }
    totalCount += 1; //need to add one to account current node
    return totalCount; 
}

uint16_t neighbors::Get2HopCarCountSelfBehind(int twoHopFurthestBehindIndex, Vector reference) {
        // 3 regions, vehicles ahead reference, vehicles behind furthest behind, vehicles between reference and furthest behind
    uint16_t totalCount = m_1HopNumNeighborsAhead;
    totalCount += m_1HopNumNeighborBehind[twoHopFurthestBehindIndex]; // first two regions
    Vector furthestBehind = Vector3D(m_1HopPositionX[twoHopFurthestBehindIndex], m_1HopPositionY[twoHopFurthestBehindIndex],0);
    float furthestBehindDist = CalculateDistance(furthestBehind, reference);
    Vector neighborPos; 
    float neighborDist;   
    for(uint16_t idx = 0; idx < m_1HopNumNeighbors; idx++) {  // also check if 1hopneighbors>0 first
        if ((m_1HopNeighborDirection[idx])) { // car is ahead, then skip
            continue;
        }
        neighborPos = Vector3D(m_1HopPositionX[idx], m_1HopPositionY[idx],0);
        neighborDist  = CalculateDistance(neighborPos, reference);
        if (neighborDist <= furthestBehindDist) {
            totalCount++;
        }               
    }
    totalCount += 1; //need to add one to account current node
    return totalCount; 
}

float neighbors::GetLosCalculation(Vector referencePos, Vector referenceVel) {
    Vector furthestAhead = Vector3D(NaN,NaN,0);
    Vector furthestBehind = Vector3D(NaN,NaN,0);
    float avgSpeedX = referenceVel.x; // in case needed
    float avgSpeedY = referenceVel.y;
    int furthestAheadIndex = Get2HopDistFurthestAheadByIndex(referencePos);
    int furthestBehindIndex = Get2HopDistFurthestBehindByIndex(referencePos);
    float dist = std::numeric_limits<float>::infinity(); // to make volume go to zero, if only 1 vehicle
    uint16_t totalCount = 1;  
    int idx;
    Vector temp = referencePos; // will change if needed
    if ( (furthestAheadIndex >= 0) && (furthestBehindIndex >= 0) ) {
        furthestAhead = Vector3D(m_neighborFurthestAheadX[furthestAheadIndex], m_neighborFurthestAheadY[furthestAheadIndex],0);
        furthestBehind = Vector3D(m_neighborFurthestBehindX[furthestBehindIndex], m_neighborFurthestBehindY[furthestBehindIndex],0);
        dist = CalculateDistance(furthestBehind, furthestAhead);
        avgSpeedX = m_neighborAvgSpeedX[furthestAheadIndex] + m_neighborAvgSpeedX[furthestBehindIndex];
        avgSpeedY = m_neighborAvgSpeedY[furthestAheadIndex] + m_neighborAvgSpeedY[furthestBehindIndex];
        avgSpeedX = avgSpeedX/2; // since added 2 values
        avgSpeedY = avgSpeedY/2;
        totalCount = Get2HopCarCount(furthestAheadIndex, furthestBehindIndex, referencePos);
    } 
    else if ( (furthestAheadIndex >= 0) && (furthestBehindIndex < 0) ) {
        furthestAhead = Vector3D(m_neighborFurthestAheadX[furthestAheadIndex], m_neighborFurthestAheadY[furthestAheadIndex],0);
        idx = GetNeighborFurthestBehindByIndex(referencePos);
        if (idx >= 0) {
            temp = Vector3D(m_1HopPositionX[idx], m_1HopPositionY[idx],0);
        }   
        dist = CalculateDistance(temp, furthestAhead);
        avgSpeedX = m_neighborAvgSpeedX[furthestAheadIndex];
        avgSpeedY = m_neighborAvgSpeedY[furthestAheadIndex];     
        totalCount = Get2HopCarCountSelfAhead(furthestAheadIndex, referencePos); // make changes
    }
    else if ( (furthestAheadIndex < 0) && (furthestBehindIndex >= 0) ) {
        furthestBehind = Vector3D(m_neighborFurthestBehindX[furthestBehindIndex], m_neighborFurthestBehindY[furthestBehindIndex],0);
        idx = GetNeighborFurthestAheadByIndex(referencePos);
        if (idx >= 0) {
            temp = Vector3D(m_1HopPositionX[idx], m_1HopPositionY[idx],0);
        } 
        dist = CalculateDistance(temp, furthestBehind);
        avgSpeedX = m_neighborAvgSpeedX[furthestBehindIndex];
        avgSpeedY = m_neighborAvgSpeedY[furthestBehindIndex];    
        totalCount = Get2HopCarCountSelfBehind(furthestBehindIndex, referencePos);
    } // 4th case where both are less than zero will utilize initialized values
    float time = dist/((Vector3D(avgSpeedX, avgSpeedY,0).GetLength()));
    float volume = totalCount/time;
    float LOS = volume/(CAPACITY_PER_LANE*NUM_LANES);
    /*
    std::cout << "\ntotal number of vehicles " << totalCount;
    std::cout << "\ntime " << time;
    std::cout << "\ndist " << dist;
    std::cout << "\nvolume " << volume;
    std::cout << "\ncapacity " << CAPACITY_PER_LANE*NUM_LANES;
    std::cout << "\nfurthestAhead " << furthestAheadIndex;
    std::cout << "\nfurthestBehind " << furthestBehindIndex << std::endl;
    */
    return LOS;
}

void neighbors::PrintNeighbors () {
    std::cout << "My neighbor list is now: ";
    std::cout << '[';
    if (m_1HopNumNeighbors > 0) {
    	for (uint16_t i = 0; i < m_1HopNumNeighbors-1; i++) {
        	std::cout << m_1HopNeighborIDs.at(i) << ", ";
    	}
    	std::cout << m_1HopNeighborIDs.at(m_1HopNumNeighbors-1) << ']';
    }
    else {
        std::cout << ']';
    }
    std::cout << std::endl; // to end line after printing everything out horizontally
}

void neighbors::PrintNeighborsAhead () {
    std::cout << "My neighbor's ahead are: ";
    std::cout << '[';
    if (m_1HopNumNeighborsAhead > 0) {
    	for (uint16_t i = 0; i < m_1HopNumNeighborsAhead-1; i++) {
        	std::cout << m_1HopNeighborIDAhead.at(i) << ", ";
    	}
    	std::cout << m_1HopNeighborIDAhead.at(m_1HopNumNeighborsAhead-1) << ']';
    }
    else {
        std::cout << ']';
    }
    std::cout << std::endl; // to end line after printing everything out horizontally
}

void neighbors::PrintNeighborsBehind () {
    std::cout << "My neighbor's behind are: ";
    std::cout << '[';
    if (m_1HopNumNeighborsBehind > 0) {
    	for (uint16_t i = 0; i < m_1HopNumNeighborsBehind-1; i++) {
        	std::cout << m_1HopNeighborIDBehind.at(i) << ", ";
    	}
    	std::cout << m_1HopNeighborIDBehind.at(m_1HopNumNeighborsBehind-1) << ']';
    }
    else {
        std::cout << ']';
    }
    std::cout << std::endl; // to end line after printing everything out horizontally
}

void neighbors::PrintDirections () {
    std::cout << "My neighbor's directions are (+1=ahead, 0=behind): ";
    std::cout << '[';
    if (m_1HopNumNeighbors > 0) {
    	for (uint16_t i = 0; i < m_1HopNumNeighbors-1; i++) {
        	std::cout << m_1HopNeighborDirection.at(i) << ", ";
    	}
    	std::cout << m_1HopNeighborDirection.at(m_1HopNumNeighbors-1) << ']';
    }
    else {
        std::cout << ']';
    }
    std::cout << std::endl; // to end line after printing everything out horizontally
}

void neighbors::PrintNeighborDirections () {
    std::cout << "My neighbor's have this many cars ahead of them: [";
    if (m_1HopNumNeighbors > 0) {
    	for (uint16_t i = 0; i < m_1HopNumNeighbors-1; i++) {
        	std::cout << m_1HopNumNeighborAhead.at(i) << ", ";
    	}
    	std::cout << m_1HopNumNeighborAhead.at(m_1HopNumNeighbors-1) << ']' << std::endl;
        std::cout << "My neighbor's have this many cars behind them: [";
        for (uint16_t i = 0; i < m_1HopNumNeighbors-1; i++) {
        	std::cout << m_1HopNumNeighborBehind.at(i) << ", ";
    	}
    	std::cout << m_1HopNumNeighborBehind.at(m_1HopNumNeighbors-1) << ']';
    }
    else {
        std::cout << ']'<< std::endl;
        std::cout << "My neighbor's have this many cars behind them: []";
    }
    std::cout << std::endl; // to end line after printing everything out horizontally
}

void neighbors::PrintNumNeighborsAheadBehind () {
    std::cout << "There are: " << Get1HopNumNeighborsAhead () << " ahead and " << Get1HopNumNeighborsBehind () << " behind me" << std::endl;
}

void neighbors::PrintLocations () {
    std::cout << "My neighbor's locations are: ";
    if (m_1HopNumNeighbors > 0) {
	    for (uint16_t i = 0; i < m_1HopNumNeighbors; i++) {
        	std::cout << '[' << m_1HopPositionX.at(i) << ", " << m_1HopPositionY.at(i) << ']' << ' ';
        }
    }
    else {
        std::cout << '[' << ']';
    }
    std::cout << std::endl; // to end line after printing everything out horizontally
}

void neighbors::PrintSpeeds () {
    std::cout << "My neighbor's speeds are: ";
    if (m_1HopNumNeighbors > 0) {
	    for (uint16_t i = 0; i < m_1HopNumNeighbors; i++) {
        	std::cout << '[' << m_1HopNeighborSpeedX.at(i) << ", " << m_1HopNeighborSpeedY.at(i) << ']' << ' ';
        }
    }
    else {
        std::cout << '[' << ']';
    }
    std::cout << std::endl; // to end line after printing everything out horizontally
}

void neighbors::Print1hopFurthestAhead () {
    std::cout << "My neighbor's FurthestAhead Pos are: ";
    if (m_1HopNumNeighbors > 0) {
	    for (uint16_t i = 0; i < m_1HopNumNeighbors; i++) {
        	std::cout << '[' << m_neighborFurthestAheadX.at(i) << ", " << m_neighborFurthestAheadY.at(i) << ']' << ' ';
        }
    }
    else {
        std::cout << '[' << ']';
    }
    std::cout << std::endl; // to end line after printing everything out horizontally
}

void neighbors::Print1hopFurthestBehind () {
    std::cout << "My neighbor's FurthestBehind are: ";
    if (m_1HopNumNeighbors > 0) {
	    for (uint16_t i = 0; i < m_1HopNumNeighbors; i++) {
        	std::cout << '[' << m_neighborFurthestBehindX.at(i) << ", " << m_neighborFurthestBehindY.at(i) << ']' << ' ';
        }
    }
    else {
        std::cout << '[' << ']';
    }
    std::cout << std::endl; // to end line after printing everything out horizontally
}

void neighbors::PrintAvgSpeeds () {
    std::cout << "My neighbor's avg speeds are: ";
    if (m_1HopNumNeighbors > 0) {
	    for (uint16_t i = 0; i < m_1HopNumNeighbors; i++) {
        	std::cout << '[' << m_neighborAvgSpeedX.at(i) << ", " << m_neighborAvgSpeedY.at(i) << ']' << ' ';
        }
    }
    else {
        std::cout << '[' << ']';
    }
    std::cout << std::endl; // to end line after printing everything out horizontally
}

void neighbors::PrintTimes () {
    std::cout << "My neighbor's last updates were at: [";
    if (m_1HopNumNeighbors > 0) {
    	for (uint16_t i = 0; i < m_1HopNumNeighbors-1; i++) {
        	std::cout << m_1HopNeighborLastTime.at(i) << ", ";
    	}
    	std::cout << m_1HopNeighborLastTime.at(m_1HopNumNeighbors-1) << ']';
    }
    else {
        std::cout << ']';
    }
    std::cout << std::endl; // to end line after printing everything out horizontally
}

void neighbors::PrintNeighborState() {
    std::cout << "current time: " << Simulator::Now() << std::endl;
    PrintNeighbors();
    //PrintTimes ();
    PrintDirections();
    PrintNeighborsAhead();
    PrintNeighborsBehind();
    PrintNumNeighborsAheadBehind ();
    PrintNeighborDirections();
    Print1hopFurthestAhead ();
    Print1hopFurthestBehind ();
    PrintAvgSpeeds ();
    std::cout << std::endl;  // put space between this and next printout 
}
#endif
