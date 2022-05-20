#ifndef SIMULATIONCONFIGURATION_H
#define SIMULATIONCONFIGURATION_H

//For network
#define NET_ADDRESS "192.168.0.0"
#define NET_MASK_ADDRESS "255.255.0.0"
#define BROADCAST_ADDRESS "255.255.255.255"
#define PK_INTERARRIVAL_TIME 1 // seconds
#define FREQ 2.4e9 // Hz
#define SYS_LOSS 1 // unitless
#define MIN_LOSS 0 // dB
#define TX_POWER 110 // dBm
#define TX_GAIN 15 // dB
#define RX_GAIN 15 // dB
#define RX_SENSITIVITY -97 // dBm, from RSU Siemens brochure
#define UNIT_FOR_SECONDS 1e9 // for nanoseconds
#define MAX_DISTANCE 500 // Tx range is 500 meters
#define TX_CUTOFF_PERCENTAGE 1 // 80% of max distance, at 77% only made 2hops, stuck on 3rd, 76% made to 318 onMid, 0.3 on high, 
//For road
#define MAX_SPEED 26.82 // meters per second, 60 mph
#define MAX_STOP_DISTANCE 87 // speed*3
#define MIN_SPEED 2.2  // meters per second, 5 mph
#define MIN_STOP_DISTANCE 6 // speed*3, average size of car is about 4-5 m
#define CAPACITY_PER_LANE 0.388888 // vehicles per second per lane, 2800 vehicles per hour on 2 lane highway
#define VC_HIGH_TRAFFIC 0.8 // volume to capacity ratio, check newer values
#define VC_LOW_TRAFFIC 0.4  
#define NUM_LANES 2
//For periodic
#define PERIODIC_PACKET_TYPE 0
#define UDP_PORT_PERIODIC 8080
#define PERIODIC_START_TIME 1 // seconds
#define PERIODIC_PACKET_RATE 10 // packets per second
#define NEIGHBOR_REMOVAL_PERIOD 0.5 // seconds
#define NEIGHBOR_TIMEOUT 2000000000 // nanoseconds, 2 seconds
//For data/routing
#define DATA_PACKET_TYPE 1
#define UDP_PORT_ROUTING 8081
#define DATA_START_TIME 2  // seconds
#define DATA_PACKET_RATE 0.5 // packets per second
#define QUEUE_REMOVAL_PERIOD 0.5 // seconds, time until check for queue removal
#define SPEED_LOG_PERIOD 1 // seconds, time until sample speed
#define N_SAMPLES 100 // number of samples recorded for speed
//For broadcast area(BA)
// for downstream, places BA on right side
//  /*
#define POINT_ONE_X 4000 // point 1 comes before point 2. Ex. p1(0,0) p2 (1,2)
#define POINT_ONE_Y 0        // in meters
#define POINT_TWO_X 6000 
#define POINT_TWO_Y 4000 // place BA past 41 to test out queue,
//  */
// for upstream, places BA on left side
  /* 
#define POINT_ONE_X -1000 // point 1 comes before point 2. Ex. p1(0,0) p2 (1,2)
#define POINT_ONE_Y 0        // in meters
#define POINT_TWO_X -800 
#define POINT_TWO_Y 200 // place BA past 41 to test out queue,
  */
#define BROADCASTING_TIME 500 // in seconds, simulation time when BA is terminated

// include files
#include "routingApp.h"
#include "periodicApp.h" // will include created header file
#include "dataApp.h" // will include created header file
#include "car.h"  // will include neighbors
#include "collectMetrics.h"
// callbacks used when receive periodic packet and data packet
using namespace ns3;

void ReceivePeriodicPacket(Ptr<Socket> socket) {
  while(Ptr<Packet> packet = socket->Recv()) {  
    // Grab contents of header
    periodicPacketHeader head;  // head will contain data in received packet
    packet->RemoveHeader(head); // to move data to head
    if (head.GetPacketType() != PERIODIC_PACKET_TYPE ) {    
        // check if correct packet type
        int type = head.GetPacketType();
        std::cout << "Within PeriodicPacket, Packet Type: " << type << std::endl;
        return;
    } 
    // determine if forwarding node is ahead=1 or behind=0 by using dot product            
    float dotProduct;
    float dotProductVel;
    Vector receiveNodePos = (socket->GetNode())->GetObject<MobilityModel>()->GetPosition(); 
    Vector diff = Vector3D(head.GetPositionX(),head.GetPositionY(),0) - receiveNodePos; // vector pointing from receiving node to forwarding node
    Vector receiveNodeVelocity = (socket->GetNode())->GetObject<MobilityModel>()->GetVelocity();   
    if (receiveNodeVelocity.GetLength() == 0 ) {
         // if receiving node not moving, don't process anymore
        return; 
    }
    dotProductVel = receiveNodeVelocity.x*head.GetSpeedX() + receiveNodeVelocity.y*head.GetSpeedY();
    if (dotProductVel <=0 ) {
        // if velocity vectors do not align, don't process because neighbor moving in opposite direction
        return;  
    }
    dotProduct = receiveNodeVelocity.x*diff.x + receiveNodeVelocity.y*diff.y;   
    uint16_t direction; // 0 = behind, 1 = ahead
    if (dotProduct >= 0) {  
        // if dot product positive, then ahead
        direction = 1;
    }
    else {  
        // if dot product negative, then behind
        direction = 0;
    }    
    // use received packet to update neighbors information in object neighbors
        // will add node as new neighbor or update information for that neighbor
    neighbors* neighbor1Hop = &((socket->GetNode())->GetObject<car>()->m_neighbors);
    neighbor1Hop->AddNode(head.GetNodeId(), direction, head.GetNumNeighborsAhead()
         , head.GetNumNeighborsBehind(), head.GetPositionX(), head.GetPositionY(), head.GetSpeedX(), head.GetSpeedY()
         , head.GetNeighborFurthestAheadX(), head.GetNeighborFurthestAheadY(), head.GetNeighborFurthestBehindX()
         , head.GetNeighborFurthestBehindY(), head.GetAvgSpeedX(), head.GetAvgSpeedY());
             // order is neighbor's node Id, direction, numAhead, numBehind, X, Y, speedX, speedY, furthestAhead.x, furthestAhead.y, furthestBehind.x
                // , furthestBehind.y, avg SpeedX, avgSpeedY
    /*
    std::cout << "******Periodic Reception: "<<std::endl;
    std::cout << "My id is: " << (socket->GetNode())->GetId() << std::endl;
    std::cout << "Received FurthestAheadX is: " << head.GetNeighborFurthestAheadX() << std::endl;
    std::cout << "Received FurthestAheadY is: " << head.GetNeighborFurthestAheadY() << std::endl;
    std::cout << "Received FurthestBehindX is: " << head.GetNeighborFurthestBehindX() << std::endl;
    std::cout << "Received FurthestBehindY is: " << head.GetNeighborFurthestBehindY() << std::endl;
    std::cout << "Speed is: " << head.GetAvgSpeedX() << std::endl;
    std::cout << "Speed is: " << head.GetAvgSpeedY() << std::endl;
    neighbor1Hop->PrintNeighborState();
    */
  }
}

void RoutePacket(Ptr<Socket> socket, dataPacketHeader head) {
    // Called by RoutePacketFromQueue or RoutePacketFromSocket
    // Determine if in broadcast area (BA)
    Vector currentPos = (socket->GetNode())->GetObject<MobilityModel>()->GetPosition();
    // assumes point 1 comes before point 2. Ex. BA is: p1(0,0) p2 (1,2)
    // will make rectangle for area, Ex p3(0.5,0.6) is within broadcast area
    if ((head.GetPosition1X() <= currentPos.x) && (currentPos.x <= head.GetPosition2X())) {
        // means receiving node is within boundary of x
        if ((head.GetPosition1Y() <= currentPos.y) && (currentPos.y <= head.GetPosition2Y())) {
        // means receiving node is within boundary of y
            std::cout << "\ncurrent time: " << Simulator::Now() << std::endl;
            std::cout << "My id is: " << (socket->GetNode())->GetId() << std::endl;
            std::cout << "I am in broadcast area" << std::endl;  
            ((socket->GetNode())->GetObject<car>())->m_neighbors.PrintNeighborState();
            // broadcast to all vehicles for specified time
            // remove routingHeader and LCB.
            return; // to end early
        }
    }
    // Determine if will reach BA before it expires
    Vector BA1 = Vector3D(head.GetPosition1X(), head.GetPosition1Y(),0); // for broadcast area point one
    Vector BA2 = Vector3D(head.GetPosition2X(), head.GetPosition2Y(),0); // for broadcast area point two        
    Vector center = Vector3D((BA1.x+BA2.x)/2,(BA1.y+BA2.y)/2,0);			
    float neighborHoodSpeed = Vector3D((socket->GetNode())->GetObject<car>()->NeighborHoodSpeedMeanX()
					, (socket->GetNode())->GetObject<car>()->NeighborHoodSpeedMeanY(),0).GetLength();
    float currentMDT = CalculateDistance(currentPos, center)/neighborHoodSpeed;  
    bool closeToBA = false;
    if (Simulator::Now()/UNIT_FOR_SECONDS + currentMDT <= head.GetBroadcastingTime()) { 
        std::cout << "\n 50% chance to be in broadcast area" << std::endl;  
        std::cout << "current time: " << Simulator::Now()/UNIT_FOR_SECONDS << std::endl;
        std::cout << "current MDT: " << currentMDT << std::endl;
        std::cout << "Time will reach BA: " << Simulator::Now()/UNIT_FOR_SECONDS + currentMDT << std::endl;
        std::cout << "BA expiration time: " << head.GetBroadcastingTime() << std::endl;
        std::cout << "My id is: " << (socket->GetNode())->GetId() << std::endl;
        //duplicate packet, remove routingHeader from duplicated packet, and LCB the duplicated packet
        closeToBA = true;
    }
    // Determine if destination node
    if ( ((socket->GetNode())->GetId() != head.GetNextHopIdAhead()) && ((socket->GetNode())->GetId() != head.GetNextHopIdBehind()) ) {
        return; // to end early
    }
    Ptr<collectMetrics> metric = ((socket->GetNode())->GetObject<car>())->GetObject<collectMetrics>();
    metric->addDataCollection(head.GetPacketId(), currentPos.x, currentPos.y, Simulator::Now().GetSeconds());
    // Determine the next hop
    Ptr<routingApp> routingAppPtr = (socket->GetNode())->GetObject<routingApp>(); // to be able to use findNextHop methods
    int nextHopIdAhead = -1;
    int nextHopIdBehind = -1;
    if ( ((socket->GetNode())->GetObject<car>())->m_neighbors.Get1HopDirectionById(head.GetPrevHopId()) == 0) {  //IP of vehicle that forwarded the packet. If returns 0, vehicle behind
        // means sent from behind, so send forward
    	nextHopIdAhead = routingAppPtr->findNextHopDownstream((socket->GetNode())->GetObject<car>(), head);
    	if (nextHopIdAhead < 0) {
            (socket->GetNode())->GetObject<car>()->AddToQueue(head); // add to packet queue if reaches here
	        std::cout << "added to queue "<< std::endl;
            return; // means will hold onto packet, will need to Tx later
    	}
    }
    else {
        if (closeToBA) { // if will get close to BA, still send
            nextHopIdBehind = routingAppPtr->findNextHopUpstream((socket->GetNode())->GetObject<car>(), head);
        }
        else {
            std::cout << "Will stop sending upstream" << std::endl; 
            return; // to stop sending upstream
        }
    	if (nextHopIdBehind < 0) {
            (socket->GetNode())->GetObject<car>()->AddToQueue(head); // add to packet queue if reaches here
            std::cout << "added to queue "<< std::endl;
            return; // means will hold onto packet, will need to Tx later
    	}
    }
    // create header to store data that will be sent
    head.SetData(DATA_PACKET_TYPE, head.GetSourceId(), (socket->GetNode())->GetId(), nextHopIdAhead, nextHopIdBehind
                , head.GetPosition1X(), head.GetPosition1Y(), head.GetPosition2X(), head.GetPosition2Y()
                , head.GetPacketId(), head.GetBroadcastingTime());
        // order is packetType, sourceId, prevHopID, nextHopIdAhead, nextHopIdBehind, 1X, 1Y, 2X, 2Y, packetId, BA expiration time
    // broadcast packet to all neighbors
    Ptr<Packet> packet = Create<Packet>(); // create new packet and add header
    packet->AddHeader(head);
    socket->Send(packet);
}

void RoutePacketFromSocket(Ptr<Socket> socket) { 
    // called when receiving data packet
    while(Ptr<Packet> packet = socket->Recv()) {        
    // Grab contents of header
    dataPacketHeader head;  // head will contain nodeId of received packet
    packet->RemoveHeader(head); // to assign value to head, peekHeader will return size of packet in bytes
    if (head.GetPacketType() != DATA_PACKET_TYPE ) {  // check if correct packet type
        int type = head.GetPacketType();
        std::cout << "Within routePacket, Packet Type: " << type << std::endl;
        return;
    }
    RoutePacket(socket,head); // to process data packet
    return;
  }
}

void RoutePacketFromQueue(Ptr<Socket> socket,Ptr<Packet> packet) { 
  // called by Car class when removing packet from queue
  // Grab contents of header
  dataPacketHeader head;  // head will contain nodeId of received packet
  packet->RemoveHeader(head); // to assign value to head, peekHeader will return size of packet in bytes
  if (head.GetPacketType() != DATA_PACKET_TYPE ) { // check if correct packet type
      int type = head.GetPacketType();
      std::cout << "Within routePacket, Packet Type: " << type << std::endl;
      return;
  }
  RoutePacket(socket,head); // to process data packet
  return;
}
#endif 
