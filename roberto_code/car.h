#ifndef CAR_H
#define CAR_H

#include "neighbors.h"
#include "dataPacketHeader.h"
#include "routingApp.h"
#include "collectMetrics.h"

using namespace ns3;
 
class car : public Node {   
    public:
        // neighbors neighbors;
        // neighbors m_neighbors;  //  to store object of type neighbors     
        static TypeId GetTypeId (void);
        virtual TypeId GetInstanceTypeId (void) const;
        void AddToQueue(dataPacketHeader packetHeader); // Queue to store packets when can't be sent
        dataPacketHeader GetFromQueue ();   // Grab packet from Queue, First in is First out
        void CheckForQueueRemoval ();       // check if any packets to route and send them
        void setRoutePacketPointer(void (*routePacketPtr) (Ptr<Socket> socket,Ptr<Packet> packet)); // To create function pointer
        void setSocketPointer(Ptr<Socket> socketPtr);    
        void setCollectMetricsPointer(Ptr<collectMetrics> metricPtr); 
	    void getSpeedValue();  // to sample a new speed and schedule to sample again
	    void addSpeedSample(float speedX, float speedY, float neighborhoodSpeedX, float neighborhoodSpeedY);
       	//float IndividualSpeedMeanX();  // get average from sampled values in m_mostRecentIndividualNSpeedX
	    //float IndividualSpeedMeanY();  // get average from sampled values in m_mostRecentIndividualNSpeedY
	    float NeighborHoodSpeedMeanX(); // get average from sampled values in m_mostRecentNeighborHoodNSpeedX
	    float NeighborHoodSpeedMeanY(); // get average from sampled values in m_mostRecentNeighborHoodNSpeedY
	    void printSpeedSamples();
    private:
        Ptr<Socket> m_socketPtr;
        Ptr<collectMetrics> m_metricPtr;
	    bool m_filledFirstTime = false; // only false until reaches first N_SAMPLES samples
	    int currentIdx = 0;
	    std::vector<float> m_mostRecentIndividualNSpeedX = std::vector<float>(N_SAMPLES,0); // initializes all values to zero
	    std::vector<float> m_mostRecentIndividualNSpeedY = std::vector<float>(N_SAMPLES,0);
	    std::vector<float> m_mostRecentNeighborHoodNSpeedX = std::vector<float>(N_SAMPLES,0); // initializes all values to zero
	    std::vector<float> m_mostRecentNeighborHoodNSpeedY = std::vector<float>(N_SAMPLES,0); 
        void (*m_routePacketPtr) (Ptr<Socket> socket,Ptr<Packet> packet);
        uint16_t m_numPackets; // total number of packets in queue
        std::vector<dataPacketHeader> m_myQ; // to hold queue of packet header
        std::vector<uint64_t> m_packetIds; // packet id for every packet
        void ScheduleQueueRemoval ();
	    void ScheduleSpeedLogUpdate ();      
};

TypeId car::GetTypeId (void) {
   static TypeId tid = TypeId ("ns3::car")
     .SetParent<Node> ();
     //.AddConstructor<car> ();
   return tid;
 }

TypeId car::GetInstanceTypeId (void) const {
   return GetTypeId ();
}

void car::AddToQueue (dataPacketHeader packetHeader) {
    // to store packet in queue to send later
    uint64_t packetId = packetHeader.GetPacketId();
    m_myQ.push_back(packetHeader); // hold contents from packet
    m_packetIds.push_back(packetId); // hold packet id for packet, can delete if do not need above check for getting same packet
    m_numPackets++;
}

dataPacketHeader car::GetFromQueue () {
    return m_myQ.front(); // since first in first out
}

void car::CheckForQueueRemoval () {
    // check if any packets in queue
    if (m_numPackets==0) {
         return; // nothing to remove
    }  
    while(m_numPackets>0){
         // continue sending packets until empty   
         // get packet ready to send
         Ptr<Packet> packetptr = Create<Packet>(); 
         packetptr->AddHeader(m_myQ[0]); // already have header stored
         m_numPackets--; // decrease packet in queue count
         m_myQ.erase(m_myQ.begin()); // delete stored value   
         m_packetIds.erase(m_packetIds.begin()); // can't use same iterator for all since different types
         m_routePacketPtr(m_socketPtr,packetptr); // to send packet
    }
    ScheduleQueueRemoval ();
}

void car::ScheduleQueueRemoval () {
    // to schedule CheckForQueueRemoval every QUEUE_REMOVAL_PERIOD seconds
    Simulator::Schedule(Seconds(QUEUE_REMOVAL_PERIOD), &car::CheckForQueueRemoval, this);  
}

void car::setRoutePacketPointer(void (*routePacketPtr) (Ptr<Socket> socket,Ptr<Packet> packet)) {
    m_routePacketPtr = routePacketPtr;
}

void car::setSocketPointer(Ptr<Socket> socketPtr) {
    m_socketPtr = socketPtr;
} 

void car::setCollectMetricsPointer(Ptr<collectMetrics> metricPtr) {
    m_metricPtr = metricPtr;
}

void car::getSpeedValue () {
    // sample for individual speed to store in m_mostRecentIndividualNSpeedX 
        // and sample of current average for speed of 1 hop neighbors to store in m_mostRecentNeighborHoodNSpeedX
        // done for x and y direction
    Vector speed = Vector3D(GetObject<MobilityModel>()->GetVelocity()); 
    neighbors m_neighbors;
    addSpeedSample(speed.x, speed.y, m_neighbors.GetAvgSpeedNeighborX(speed.x), m_neighbors.GetAvgSpeedNeighborY(speed.y));
    ScheduleSpeedLogUpdate (); // to schedule this method again in SPEED_LOG_PERIOD seconds
}

void car::ScheduleSpeedLogUpdate () { 
    // to schedule getSpeedValue every SPEED_LOG_PERIOD seconds
    Simulator::Schedule(Seconds(SPEED_LOG_PERIOD), &car::getSpeedValue, this); 
}

void car::addSpeedSample(float speedX, float speedY, float neighborhoodSpeedX, float neighborhoodSpeedY) {
    // store new speed sample for individual speed to store in m_mostRecentIndividualNSpeedX 
        // and store sample of current average for speed of 1 hop neighbors to store in m_mostRecentNeighborHoodNSpeedX
        // done for x and y direction
    m_mostRecentIndividualNSpeedX[currentIdx] = speedX;
    m_mostRecentIndividualNSpeedY[currentIdx] = speedY;
    m_mostRecentNeighborHoodNSpeedX[currentIdx] = neighborhoodSpeedX;
    m_mostRecentNeighborHoodNSpeedY[currentIdx] = neighborhoodSpeedY;
    currentIdx++; // increase by one for next time it is used
    if (currentIdx == N_SAMPLES) {
	    currentIdx = 0; // rewrite first index since it is now the oldest
	    m_filledFirstTime = true;
    }
}
/*
float car::IndividualSpeedMeanX() {
    // get average speed from vector of sampled values in m_mostRecentIndividualNSpeedX
    float mean = 0;
    int max = currentIdx;
	if (m_filledFirstTime) { // if reached N_SAMPLES in vector, must change to be the max
		max = N_SAMPLES;
	}
	for(int idx = 0; idx < max; idx++) {
		mean += m_mostRecentIndividualNSpeedX[idx];
	}
	mean = mean/max;
	return mean;
}

float car::IndividualSpeedMeanY() {
    // get average speed from vector of sampled values in m_mostRecentIndividualNSpeedY
	float mean = 0;
	int max = currentIdx;
	if (m_filledFirstTime) { // if reached N_SAMPLES in vector, must change to be the max
		max = N_SAMPLES;
	}
	for(int idx = 0; idx < max; idx++) {
		mean += m_mostRecentIndividualNSpeedY[idx];
	}
	mean = mean/max;
	return mean;
}
*/
float car::NeighborHoodSpeedMeanX() {
    // get average speed from vector of sampled values in m_mostRecentNeighborHoodNSpeedX
    float mean = 0;
	int max = currentIdx;
	if (m_filledFirstTime) { // if reached N_SAMPLES in vector, must change to be the max
		max = N_SAMPLES;
	}
	for(int idx = 0; idx < max; idx++) {
		mean += m_mostRecentNeighborHoodNSpeedX[idx];
	}
	mean = mean/max;
	return mean;
}

float car::NeighborHoodSpeedMeanY() {
    // get average speed from vector of sampled values in m_mostRecentNeighborHoodNSpeedY
	float mean = 0;
	int max = currentIdx;
	if (m_filledFirstTime) { // if reached N_SAMPLES in vector, must change to be the max
		max = N_SAMPLES;
	}
	for(int idx = 0; idx < max; idx++) {
		mean += m_mostRecentNeighborHoodNSpeedY[idx];
	}
	mean = mean/max;
	return mean;
}

void car::printSpeedSamples() {
    /*
    std::cout << "\nMy individual speed samples are ";
    for (uint16_t i = 0; i < N_SAMPLES; i++) {
        	std::cout << '[' << m_mostRecentIndividualNSpeedX.at(i) << ", " << m_mostRecentIndividualNSpeedY.at(i) << ']' << ' ';
    }
	std::cout << "\nThe mean for individual speeds is: [" << IndividualSpeedMeanX() << ", " << IndividualSpeedMeanY() << "]";
    */
	std::cout << "\nMy neighborhood speed samples are ";
    for (uint16_t i = 0; i < N_SAMPLES; i++) {
        	std::cout << '[' << m_mostRecentNeighborHoodNSpeedX.at(i) << ", " << m_mostRecentNeighborHoodNSpeedY.at(i) << ']' << ' ';
    }
	std::cout << "\nThe mean for individual speeds is: [" << NeighborHoodSpeedMeanX() << ", " << NeighborHoodSpeedMeanY() << "]";
    std::cout << std::endl; // to end line after printing everything out horizontally
}
#endif
