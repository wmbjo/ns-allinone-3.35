/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


// Class declaration of an application that transmits UDP packets at random times.
// The time between consecutive packets is exponentially distributed.

#ifndef ROUTINGAPP_H
#define ROUTINGAPP_H

#include "simulationConfiguration.h"
#include "car.h"
#include "periodicPacketHeader.h"
#include "dataPacketHeader.h"

namespace ns3 {
  class routingApp : public Application {
  public:
    routingApp();
    virtual ~routingApp();
    void Setup (Ptr<Socket> socket, uint16_t portNum);
    Ptr<Socket> GetSocketPtr(void);
    // Determine next hop in either direction chosen
    int findNextHopDownstream(Ptr<car> receivingNode, dataPacketHeader head);
    int findNextHopUpstream(Ptr<car> receivingNode, dataPacketHeader head);
  private:
    virtual void StartApplication(void);
    virtual void StopApplication(void);
    // helper for findNextHopDownstream
    int findNextHopHighTrafficDownstream(neighbors* allNeighborInfo, Vector center, Vector nodePos, float stopDist);
    int findNextHopMidTrafficDownstream(float neighborHoodSpeed, neighbors* allNeighborInfo, Vector center, Vector nodePos, float stopDist);
    int findNextHopLowTrafficDownstream(float neighborHoodSpeed, neighbors* allNeighborInfo, Vector center, Vector nodePos, float stopDist);
    // helper for findNextHopUpstream
    int findNextHopHighTrafficUpstream(neighbors* allNeighborInfo, Vector center, Vector nodePos, float stopDist);
    int findNextHopMidTrafficUpstream(float neighborHoodSpeed, neighbors* allNeighborInfo, Vector center, Vector nodePos, float stopDist);
    int findNextHopLowTrafficUpstream(float neighborHoodSpeed, neighbors* allNeighborInfo, Vector center, Vector nodePos, float stopDist);    
    Ptr<Socket> m_socketptr; 
    uint16_t m_portNum;
    EventId m_sendEvent;
    bool m_running;
  };

  routingApp::routingApp() : m_sendEvent(), m_running(false) {}

  routingApp::~routingApp() { 
    m_socketptr = NULL; 
  }

  void routingApp::Setup(Ptr<Socket> socket, uint16_t portNum) {
    m_socketptr = socket;
    m_portNum = portNum;
  }

  Ptr<Socket> routingApp::GetSocketPtr(void) {
    return m_socketptr;
  }
  
  
  void routingApp::StartApplication(void) {
    m_running = true;
    // to be able to broadcast
    m_socketptr->Bind(InetSocketAddress(Ipv4Address::GetAny(), m_portNum));
    m_socketptr->SetAllowBroadcast(true);
    m_socketptr->Connect(InetSocketAddress(Ipv4Address(BROADCAST_ADDRESS), m_portNum));
  }

  void routingApp::StopApplication(void) {
    m_running = false;
    if(m_sendEvent.IsRunning())
      Simulator::Cancel(m_sendEvent);
    if(m_socketptr)
      m_socketptr->Close();
  }
  
  int routingApp::findNextHopHighTrafficDownstream(neighbors* allNeighborInfo, Vector center, Vector nodePos, float stopDist) { 
        //used in heavy traffic
        //finds next hop based on vehicle closest to broadcast area      
    std::cout << "Next hop based on high traffic, Min dist to BA" << std::endl;
    uint16_t numNeighbors = allNeighborInfo->Get1HopNumNeighbors(); 
    float currentMin = CalculateDistance(nodePos, center); //if current node closer, hold onto packet
    int furthestIdx = -1;    
    float neighborDist;
    Vector neighborPos;
    for(uint16_t idx = 0; idx < numNeighbors; idx++) {  
        if (allNeighborInfo->Get1HopDirection(idx) == 0) { // car is behind, then skip
            continue;
        }      
        neighborPos = Vector3D(allNeighborInfo->GetNeighborPositionX(idx)
                            , allNeighborInfo->GetNeighborPositionY(idx),0);
        if (CalculateDistance(neighborPos, nodePos) < stopDist) {
            std::cout << "\nToo close, They are  within(m): " << stopDist << std::endl;               
            continue; // if not going to move more than 3 second of driving, hold onto packet
        }
        if (CalculateDistance(neighborPos, nodePos) >= MAX_DISTANCE*TX_CUTOFF_PERCENTAGE) {
            std::cout << "\nToo far, may not make it to them. They are  within(m): " << MAX_DISTANCE*TX_CUTOFF_PERCENTAGE << std::endl;               
            continue; // if not going to move more than 3 second of driving, hold onto packet
        }
        neighborDist = CalculateDistance(neighborPos, center);
        std::cout << "Current min dist is:"<< currentMin << ", best idx is "<< furthestIdx << std::endl;
        std::cout << "Neighbor id is: "<< allNeighborInfo->Get1HopNeighborId(idx) << ", idx is: " << idx << ", Neighbor dist is: " << neighborDist << ", Neighbor dist is: "<< neighborDist<<std::endl;
        if (neighborDist < currentMin) {
            // if closer to broadcast area, change current id  
            currentMin = neighborDist;           
            furthestIdx = idx;                
        }
    }
    return furthestIdx;   
  }
  
  int routingApp::findNextHopMidTrafficDownstream(float neighborHoodSpeed, neighbors* allNeighborInfo, Vector center, Vector nodePos, float stopDist) { 
    // used in mid traffic
    //finds next hop based on vehicle with minimum message delivery time and has neighbor ahead
    std::cout << "Next hop based on mid traffic, Min MDT and node must have neighbors ahead" << std::endl;
    uint16_t numNeighbors = allNeighborInfo->Get1HopNumNeighbors(); 
    float currentMin = CalculateDistance(nodePos, center); //if current node closer, hold onto packet
    float currentMDT = currentMin/neighborHoodSpeed; //if current node closer, hold onto packet  
    int bestIdx = -1;    
    float neighborDist;
    float neighborMDT;
    Vector neighborPos;
    for(uint16_t idx = 0; idx < numNeighbors; idx++) {   
        // wil include vehicles behind in case they have min MDT    
        neighborPos = Vector3D(allNeighborInfo->GetNeighborPositionX(idx), allNeighborInfo->GetNeighborPositionY(idx),0);
        if (CalculateDistance(neighborPos, nodePos)  < stopDist) {
            std::cout << "\nToo close, They are  within(m): " << stopDist << std::endl;               
            continue; // if not going to move more than 3 second of driving, hold onto packet
        }
        if (CalculateDistance(neighborPos, nodePos) >= MAX_DISTANCE*TX_CUTOFF_PERCENTAGE) {
            std::cout << "\nToo far, may not make it to them. They are  within(m): " << MAX_DISTANCE*TX_CUTOFF_PERCENTAGE << std::endl;               
            continue; // if not going to move more than 3 second of driving, hold onto packet
        }
        neighborDist = CalculateDistance(neighborPos, center);
        neighborMDT = neighborDist/neighborHoodSpeed; // time = dist/vel
        std::cout << "Current MDT is:" << currentMDT << ", best idx is "<< bestIdx << std::endl;
        std::cout << "Neighbor id is: "<< allNeighborInfo->Get1HopNeighborId(idx) << ", idx is: " << idx << ", Neighbor dist is: " << neighborDist << ", Neighbor MDT is: "<< neighborMDT<<std::endl;
        if (neighborMDT < currentMDT) {
            if (allNeighborInfo->Get1HopNumberOfNodesAheadOfNeighbor(idx) == 0) {
                // if node doesn't have neighbors ahead, move on
                continue;
            } 
            // if will drive faster to broadcast area, change current id  
            currentMDT = neighborMDT;
            currentMin = neighborDist;        
            bestIdx = idx;                
        }                
    }
    return bestIdx;    
  }

  int routingApp::findNextHopLowTrafficDownstream(float neighborHoodSpeed, neighbors* allNeighborInfo, Vector center, Vector nodePos, float stopDist) { 
    // used in low traffic
    //finds next hop based on vehicle with minimum message delivery time 
    std::cout << "Next hop based on low traffic, Min MDT" << std::endl;
    uint16_t numNeighbors = allNeighborInfo->Get1HopNumNeighbors(); 
    float currentMDT = CalculateDistance(nodePos, center)/neighborHoodSpeed; //if current node closer, hold onto packet
    int bestIdx = -1;    
    float neighborDist;
    float neighborMDT;
    Vector neighborPos;
    for(uint16_t idx = 0; idx < numNeighbors; idx++) {  
        if (allNeighborInfo->Get1HopDirection(idx) == 0) { // car is behind, then skip
            continue;
        }      
        neighborPos = Vector3D(allNeighborInfo->GetNeighborPositionX(idx)
                                    , allNeighborInfo->GetNeighborPositionY(idx),0);
        if (CalculateDistance(neighborPos, nodePos)  < stopDist) {
            std::cout << "\nToo close, They are  within(m): " << stopDist << std::endl;               
            continue; // if not going to move more than 3 second of driving, hold onto packet
        }
        if (CalculateDistance(neighborPos, nodePos) >= MAX_DISTANCE*TX_CUTOFF_PERCENTAGE) {
            std::cout << "\nToo far, may not make it to them. They are  within(m): " << MAX_DISTANCE*TX_CUTOFF_PERCENTAGE << std::endl;               
            continue; // if not going to move more than 3 second of driving, hold onto packet
        }
        neighborDist = CalculateDistance(neighborPos, center);
        neighborMDT = neighborDist/neighborHoodSpeed;
        std::cout << "Current MDT is:"<< currentMDT << ", best idx is "<< bestIdx << std::endl;
        std::cout << "Neighbor id is: "<< allNeighborInfo->Get1HopNeighborId(idx) << ", idx is: " << idx << ", Neighbor dist is: " << neighborDist << ", Neighbor MDT is: "<< neighborMDT<<std::endl;
        if (neighborMDT < currentMDT) {
            // if will drive faster to broadcast area, change current id  
            currentMDT = neighborMDT;           
            bestIdx = idx;                
        }
    }
    return bestIdx;   
  }
  
  
  int routingApp::findNextHopHighTrafficUpstream(neighbors* allNeighborInfo, Vector center, Vector nodePos, float stopDist) { 
        //used in heavy traffic
        //finds next hop based on vehicle furthest to broadcast area      
    std::cout << "Next hop based on high traffic, Max dist to BA" << std::endl;
    uint16_t numNeighbors = allNeighborInfo->Get1HopNumNeighbors(); 
    float currentMin = CalculateDistance(nodePos, center); //if current node closer, hold onto packet
    int furthestIdx = -1;    
    float neighborDist;
    Vector neighborPos;
    for(uint16_t idx = 0; idx < numNeighbors; idx++) {  
        if (allNeighborInfo->Get1HopDirection(idx) == 1) { // car is ahead, then skip, only change from high traffic
            continue;
        }      
        neighborPos = Vector3D(allNeighborInfo->GetNeighborPositionX(idx)
                            , allNeighborInfo->GetNeighborPositionY(idx),0);
        if (CalculateDistance(neighborPos, nodePos) < stopDist) {
            std::cout << "\nToo close, They are  within(m): " << stopDist << std::endl;               
            continue; // if not going to move more than 3 second of driving, hold onto packet
        }
        if (CalculateDistance(neighborPos, nodePos) >= MAX_DISTANCE*TX_CUTOFF_PERCENTAGE) {
            std::cout << "\nToo far, may not make it to them. They are  within(m): " << MAX_DISTANCE*TX_CUTOFF_PERCENTAGE << std::endl;               
            continue; // if not going to move more than 3 second of driving, hold onto packet
        }
        neighborDist = CalculateDistance(neighborPos, center);
        std::cout << "Current dist is:"<< currentMin << ", best idx is "<< furthestIdx << std::endl;
        std::cout << "Neighbor id is: "<< allNeighborInfo->Get1HopNeighborId(idx) << ", idx is: " << idx << ", Neighbor dist is: " << neighborDist << ", Neighbor dist is: "<< neighborDist<<std::endl;
        if (neighborDist > currentMin) {
            // if closer to broadcast area, change current id  
            currentMin = neighborDist;           
            furthestIdx = idx;                
        }
    }
    return furthestIdx;   
  }

  int routingApp::findNextHopMidTrafficUpstream(float neighborHoodSpeed, neighbors* allNeighborInfo, Vector center, Vector nodePos, float stopDist) {
    // used in mid traffic
    //finds next hop based on vehicle with maximum message delivery time and has neighbor behind
    std::cout << "Next hop based on mid traffic, Max MDT and has neighbors behind" << std::endl;
    uint16_t numNeighbors = allNeighborInfo->Get1HopNumNeighbors(); 
    float currentMax = CalculateDistance(nodePos, center); //if current node closer, hold onto packet
    float currentMDT = currentMax/neighborHoodSpeed; //if current node closer, hold onto packet  
    int bestIdx = -1;    
    float neighborDist;
    float neighborMDT;
    Vector neighborPos;
    for(uint16_t idx = 0; idx < numNeighbors; idx++) {   
        // wil include vehicles ahead and behind in case they have max MDT    
        neighborPos = Vector3D(allNeighborInfo->GetNeighborPositionX(idx), allNeighborInfo->GetNeighborPositionY(idx),0);
        if (CalculateDistance(neighborPos, nodePos)  < stopDist) {
            std::cout << "\nToo close, They are  within(m): " << stopDist << std::endl;               
            continue; // if not going to move more than 3 second of driving, hold onto packet
        }
        if (CalculateDistance(neighborPos, nodePos) >= MAX_DISTANCE*TX_CUTOFF_PERCENTAGE) {
            std::cout << "\nToo far, may not make it to them. They are  within(m): " << MAX_DISTANCE*TX_CUTOFF_PERCENTAGE << std::endl;               
            continue; // if not going to move more than 3 second of driving, hold onto packet
        }
        neighborDist = CalculateDistance(neighborPos, center);
        neighborMDT = neighborDist/neighborHoodSpeed; // time = dist/vel
        std::cout << "Current MDT is:"<< currentMDT << ", best idx is "<< bestIdx << std::endl;
        std::cout << "Neighbor id is: "<< allNeighborInfo->Get1HopNeighborId(idx) << ", idx is: " << idx;
        std::cout << ", Neighbor dist is: " << neighborDist << ", Neighbor MDT is: "<< neighborMDT<<std::endl;
        if (neighborMDT > currentMDT) { //only change from mid traffic
            if (allNeighborInfo->Get1HopNumberOfNodesBehindOfNeighbor(idx) == 0) {
                // if node doesn't have neighbors ahead, move on
                continue;
            } 
            // if will drive faster to broadcast area, change current id  
            currentMDT = neighborMDT;
            currentMax = neighborDist;        
            bestIdx = idx;                
        }                
    }
    return bestIdx;    
  }

  int routingApp::findNextHopLowTrafficUpstream(float neighborHoodSpeed, neighbors* allNeighborInfo, Vector center, Vector nodePos, float stopDist) { 
    // used in low traffic
    //finds next hop based on vehicle with maximum message delivery time
    std::cout << "Next hop based on low traffic, Max MDT" << std::endl;
    uint16_t numNeighbors = allNeighborInfo->Get1HopNumNeighbors(); 
    float currentMDT = CalculateDistance(nodePos, center)/neighborHoodSpeed; //if current node closer, hold onto packet
    int bestIdx = -1;    
    float neighborDist;
    float neighborMDT;
    Vector neighborPos;
    for(uint16_t idx = 0; idx < numNeighbors; idx++) {  
        // will consider vehicles going behind, change from low traffic 
        if (allNeighborInfo->Get1HopDirection(idx) == 1) { // car is ahead, then skip,only change from high traffic
            continue;
        }     
        neighborPos = Vector3D(allNeighborInfo->GetNeighborPositionX(idx)
                                    , allNeighborInfo->GetNeighborPositionY(idx),0);
        if (CalculateDistance(neighborPos, nodePos)  < stopDist) {
            std::cout << "\nToo close, They are  within(m): " << stopDist << std::endl;               
            continue; // if not going to move more than 3 second of driving, hold onto packet
        }
        if (CalculateDistance(neighborPos, nodePos) >= MAX_DISTANCE*TX_CUTOFF_PERCENTAGE) {
            std::cout << "\nToo far, may not make it to them. They are  within(m): " << MAX_DISTANCE*TX_CUTOFF_PERCENTAGE << std::endl;               
            continue; // if not going to move more than 3 second of driving, hold onto packet
        }
        neighborDist = CalculateDistance(neighborPos, center);
        neighborMDT = neighborDist/neighborHoodSpeed;
        std::cout << "Current MDT is:"<< currentMDT << ", best idx is "<< bestIdx << std::endl;
        std::cout << "Neighbor id is: "<< allNeighborInfo->Get1HopNeighborId(idx) << ", idx is: " << idx;
        std::cout << ", Neighbor dist is: " << neighborDist << ", Neighbor MDT is: "<< neighborMDT<<std::endl;
        if (neighborMDT > currentMDT) { //only change from low traffic
            // if will drive faster to broadcast area, change current id  
            currentMDT = neighborMDT;           
            bestIdx = idx;              
        }
    }
    return bestIdx;   
  }

  int routingApp::findNextHopDownstream(Ptr<car> receivingNode, dataPacketHeader data) {    
    int nextHopIdx;
    neighbors* allNeighborInfo = &(receivingNode->m_neighbors); // turn into a pointer, also methods that use it
    float neighborHoodSpeed = Vector3D(receivingNode->NeighborHoodSpeedMeanX(), receivingNode->NeighborHoodSpeedMeanY(),0).GetLength();
    std::cout << "\n\n\n********For downstream ";
    // check if have neighbors
    uint16_t numNeighbors = allNeighborInfo->Get1HopNumNeighborsAhead(); 
        // remove to consider neighbors ahead and behind?
    if (numNeighbors == 0) {
        return -1; // same as receiving node since no neighbors
    }
    // check traffic conditions of receiving node
    Vector nodePos = receivingNode->GetObject<MobilityModel>()->GetPosition();               
    Vector nodeSpeed = receivingNode->GetObject<MobilityModel>()->GetVelocity(); 
    Vector BA1 = Vector3D(data.GetPosition1X(),data.GetPosition1Y(),0); // for broadcast area point one
    Vector BA2 = Vector3D(data.GetPosition2X(),data.GetPosition2Y(),0); // for broadcast area point one        
    Vector center = Vector3D((BA1.x+BA2.x)/2,(BA1.y+BA2.y)/2,0);
    float LOS = allNeighborInfo->GetLosCalculation(nodePos, nodeSpeed); // need to check if neighbor is moving toward broadcast area, if not moving towards bA then ignore
    // remove use of center since not needed
    uint16_t traffic; // if 3, high traffic. if 2, medium traffic, 1 no traffic
    float stopDist = nodeSpeed.GetLength()*3; // stopping distance according to DMV= speed*3 seconds. Distance needed to stop   
        // include paramter to determine if msg moves ahead or backwards
    if (LOS>VC_HIGH_TRAFFIC) {
        traffic = 3; // high traffic
        nextHopIdx = findNextHopHighTrafficDownstream(allNeighborInfo, center, receivingNode->GetObject<MobilityModel>()->GetPosition(), stopDist);
            //return node that is closest to broadcast area(Dn)
    } else if (LOS<VC_LOW_TRAFFIC) {
        traffic = 1; // no traffic
        nextHopIdx = findNextHopLowTrafficDownstream(neighborHoodSpeed, allNeighborInfo, center
                        , receivingNode->GetObject<MobilityModel>()->GetPosition(), stopDist);
            //return node with MDT
    } else {
        traffic = 2; // medium traffic otherwise
        nextHopIdx = findNextHopMidTrafficDownstream(neighborHoodSpeed, allNeighborInfo, center
                        , receivingNode->GetObject<MobilityModel>()->GetPosition(), stopDist);
            //return node with smallest Dn and MDT
    }
    // check if broadcast area upstream (against direction of traffic) or downstream (with direction of traffic)
    //std::cout << "\ncurrent time: " << Simulator::Now() << std::endl;
    std::cout << "\nMy id is: " << receivingNode->GetId() << std::endl;  
    std::cout << "LOS (Volume to Capacity ratio) is: " << LOS << std::endl;
    std::cout << "traffic is: " << traffic << std::endl;
    std::cout << "My speed is: " << nodeSpeed.x <<", " << nodeSpeed.y<< ", StopDist is: " << stopDist <<std::endl;
    allNeighborInfo->PrintNeighborState();
    std::cout << "In findNextHop, source is: " << data.GetSourceId() << ", prev hop is: "<< data.GetPrevHopId();                
    if (nextHopIdx == -1) {
         std::cout << ", next hop is self: " << nextHopIdx << std::endl;
         return nextHopIdx;
    }
    std::cout << ", next found hop is: " << allNeighborInfo->Get1HopNeighborId(nextHopIdx) << std::endl;
    //data.Print(std::cout);           
    //std::cout << "Next hop speed is: " << allNeighborInfo->GetNeighborSpeedX(nextHopIdx) <<", " << allNeighborInfo->GetNeighborSpeedY(nextHopIdx)<< std::endl;
    //std::cout << "My position is: " << receivingNode->GetObject<MobilityModel>()->GetPosition().x <<", " << receivingNode->GetObject<MobilityModel>()->GetPosition().y<< std::endl;
    //std::cout << "Next hop position is: " << allNeighborInfo->GetNeighborPositionX(nextHopIdx) <<", " << allNeighborInfo->GetNeighborPositionY(nextHopIdx)<< std::endl;
    //std::cout << "Destination is: " << BA1.x <<", " << BA1.y<< std::endl;
    return allNeighborInfo->Get1HopNeighborId(nextHopIdx);
  }

  int routingApp::findNextHopUpstream(Ptr<car> receivingNode, dataPacketHeader data) {    
    // Determine traffic level then find next hop upstream based on traffic level
    int nextHopIdx;
    neighbors* allNeighborInfo = &(receivingNode->m_neighbors); // turn into a pointer, also methods that use it
    float neighborHoodSpeed = Vector3D(receivingNode->NeighborHoodSpeedMeanX(), receivingNode->NeighborHoodSpeedMeanY(),0).GetLength();
    // check if have neighbors
    uint16_t numNeighbors = allNeighborInfo->Get1HopNumNeighborsBehind(); 
    std::cout << "\n\n\n********For upstream ";
        // remove to consider neighbors ahead and behind?
    if (numNeighbors == 0) {
        return -1; // same as receiving node since no neighbors
    }
    // check traffic conditions of receiving node
    Vector nodePos = receivingNode->GetObject<MobilityModel>()->GetPosition();               
    Vector nodeSpeed = receivingNode->GetObject<MobilityModel>()->GetVelocity(); 
    Vector BA1 = Vector3D(data.GetPosition1X(),data.GetPosition1Y(),0); // for broadcast area point one
    Vector BA2 = Vector3D(data.GetPosition2X(),data.GetPosition2Y(),0); // for broadcast area point one        
    Vector center = Vector3D((BA1.x+BA2.x)/2,(BA1.y+BA2.y)/2,0);
    float LOS = allNeighborInfo->GetLosCalculation(nodePos, nodeSpeed); // need to check if neighbor is moving toward broadcast area, if not moving towards bA then ignore
    // remove use of center since not needed
    uint16_t traffic; // if 3, high traffic. if 2, medium traffic, 1 no traffic
    float stopDist = nodeSpeed.GetLength()*3; // stopping distance according to DMV= speed*3 seconds. Distance needed to stop   
        // include paramter to determine if msg moves ahead or backwards
    if (LOS>VC_HIGH_TRAFFIC) {
        traffic = 3; // high traffic
        nextHopIdx = findNextHopHighTrafficUpstream(allNeighborInfo, center, receivingNode->GetObject<MobilityModel>()->GetPosition(), stopDist);
            //return node that is closest to broadcast area(Dn)
    } else if (LOS<VC_LOW_TRAFFIC) {
        traffic = 1; // no traffic
        nextHopIdx = findNextHopLowTrafficUpstream(neighborHoodSpeed, allNeighborInfo, center
                        , receivingNode->GetObject<MobilityModel>()->GetPosition(), stopDist);
            //return node with MDT
    } else {
        traffic = 2; // medium traffic otherwise
        nextHopIdx = findNextHopMidTrafficUpstream(neighborHoodSpeed, allNeighborInfo, center
                        , receivingNode->GetObject<MobilityModel>()->GetPosition(), stopDist);
            //return node with smallest Dn and MDT
    }
    // check if broadcast area upstream (against direction of traffic) or downstream (with direction of traffic)
    //std::cout << "\ncurrent time: " << Simulator::Now() << std::endl;
    std::cout << "My id is: " << receivingNode->GetId() << std::endl;  
    std::cout << "LOS (Volume to Capacity ratio) is: " << LOS << std::endl;
    std::cout << "traffic is: " << traffic << std::endl;
    std::cout << "My speed is: " << nodeSpeed.x <<", " << nodeSpeed.y<< ", StopDist is: " << stopDist <<std::endl;
    allNeighborInfo->PrintNeighborState();
    std::cout << "In findNextHop, source is: " << data.GetSourceId() << ", prev hop is: "<< data.GetPrevHopId();                
    if (nextHopIdx == -1) {
         std::cout << ", next hop is self: " << nextHopIdx << std::endl;
         return nextHopIdx;
    }
    std::cout << ", next found hop is: " << allNeighborInfo->Get1HopNeighborId(nextHopIdx) << std::endl;
    //data.Print(std::cout);         
    //std::cout << "Next hop speed is: " << allNeighborInfo->GetNeighborSpeedX(nextHopIdx) <<", " << allNeighborInfo->GetNeighborSpeedY(nextHopIdx)<< std::endl;
    //std::cout << "My position is: " << receivingNode->GetObject<MobilityModel>()->GetPosition().x <<", " << receivingNode->GetObject<MobilityModel>()->GetPosition().y<< std::endl;
    //std::cout << "Next hop position is: " << allNeighborInfo->GetNeighborPositionX(nextHopIdx) <<", " << allNeighborInfo->GetNeighborPositionY(nextHopIdx)<< std::endl;
    //std::cout << "Destination is: " << BA1.x <<", " << BA1.y<< std::endl;
    return allNeighborInfo->Get1HopNeighborId(nextHopIdx);
  }
}

// work looks good, find the pdf by
   // find bins and create as vector. For now, hardcode to 10 bins
   // count how many values fall inside those values and divide by num of samples to get prob
   // if minimum= max?
   // check for special cases
   // also need stopping condition for going backwards. is When prob drops, ex froom 90% making it to 30% chance of making it

// ********* Assignments for next time
// double check that all routing is correct (high,mid, low)
           // then try on sumo trafic scenario
// clean up code and make sure to include comments, very detailed
// Look for papers related to VANET, routing and broadcasting. That address routing In VANET whether unicast, multi or broadcast
// *******



// Tests Passed:
    // If all nodes same speed, each traffic case picks furthest vehicle
       // tested by changing node distance to cause LOS to be high (200), medium (300), low (400)  
       //sudo ./waf --run "scratch/constantMovement --NumNodes=14 --SimulationTime=10 --nodeSpeed=124.4 --nodeDist=300 --twoLane=true --RVForAllSpeed=3"
    // For low traffic-1
        // Will hold onto packet for having lower MDT than neighbors ahead, singleSpeed=50
        // passes packet from one node to the next for lower, single speed=5       
        //sudo ./waf --run "scratch/caravanMovement --NumNodes=14 --SimulationTime=10 --nodeSpeed=24.4 --nodeDist=400 --twoLane=true --RVForAllSpeed=3 --singleSpeed=5"
    // For med traffic-2
        // Will hold onto packet beascause neighbor ahead has no nodes
        //sudo ./waf --run "scratch/constantMovement --NumNodes=14 --SimulationTime=10 --nodeSpeed=124.4 --nodeDist=300 --twoLane=true --RVForAllSpeed=3"
    // For high traffic-3
        // passes packet based on distance
        //sudo ./waf --run "scratch/caravanMovement --NumNodes=14 --SimulationTime=10 --nodeSpeed=124.4 --nodeDist=200 --twoLane=true --RVForAllSpeed=3 --singleSpeed=124.4"  make smaller for speed and distance


                // scenario: traffic factor=5, road length = 18km, simTime = 100 with 40 nodes, 400 sec with 100 nodes
                        // high traffic,  count=78, Los 1 or above
                        // mid traffic,   count=39, Los around 0.5-0.8
                        // low traffic,  count=19, Los around 0.3

//Finally the broadcast
        // when reaches broadcast? How do you send them to everyone
     //idea: if not destination node, determine probability itself will be in BA
                // determining probability of being in BA
                        // Assumptions: Speed, Locations, LOS, BA
                        // Tests: MDT with average speed of neighbors, assumes straight path
                                // could also keep history of info to determine if will change
                        // Things we know: 100%, vehicle is in BA
                                 // uncertian MDT=broadcasting time
        // 2nd step: if above a threshold, will send a packet to those neighbors
        // learn about how distribution of speed of vehicles
     // idea: add field to data Packet to know if packet should be broadcasted
          // will be checked before using RoutePacket function in simulationConfiguration
                // When should be brodcasted
                    // idea 1: Install Data App on node since data app broadcasts all packets
                        // Use Data App to continously broadcast packet
                    // idea 2: For loop to send packet to all neighbors
                // When not broadcasted
                    //use Route packet like normal
        
#endif
