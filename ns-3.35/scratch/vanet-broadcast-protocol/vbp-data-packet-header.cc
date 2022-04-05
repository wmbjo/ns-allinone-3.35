#include "vbp-data-packet-header.h"

namespace ns3
{
     namespace vbp
     {

 dataPacketHeader::dataPacketHeader () {
   // we must provide a public default constructor, 
   // implicit or explicit, but never private.
 }

 dataPacketHeader::~dataPacketHeader () {
 }
 
 TypeId dataPacketHeader::GetTypeId (void) {
   static TypeId tid = TypeId ("ns3::dataPacketHeader")
     .SetParent<Header> ()
     .AddConstructor<dataPacketHeader> ();
   return tid;
 }

 TypeId dataPacketHeader::GetInstanceTypeId (void) const {
   return GetTypeId ();
 }
 
 void dataPacketHeader::Print (std::ostream &os) const {
   // This method is invoked by the packet printing
   // routines to print the content of my header.
   //os << "\nAs unsigned integer\nSource is: " << m_sourceIP <<  " Prev hop is: " << m_prevHopIP << std::endl;
   //os << "Next hop ahead is: " << m_nextHopIdAhead << " Next hop behind is: " << m_nextHopIdBehind << std::endl;
   os << "point 1: " << GetPosition1X() << ", " << GetPosition1Y() << std::endl;
   os << "point 2: " << GetPosition2X() << ", " << GetPosition2Y() << std::endl;
   os << "BATime is: " << GetBroadcastingTime() << " seconds" << std::endl;
 }

 uint32_t dataPacketHeader::GetSerializedSize (void) const {
   // we reserve bytes for our header.
   uint32_t totalSize = sizeof(m_packetType) + sizeof(m_position1X) + sizeof(m_position1Y) + sizeof(m_Position2X) + sizeof(m_Position2Y) + sizeof(m_BATime);
   //uint32_t totalSize = sizeof(m_packetType) + sizeof(m_sourceIP) + sizeof(m_prevHopIP) + sizeof(m_nextHopIdAhead) + sizeof(m_nextHopIdBehind) + sizeof(m_position1X) + sizeof(m_position1Y) + sizeof(m_Position2X) + sizeof(m_Position2Y) + sizeof(m_BATime);
   //size of header in bytes
   return totalSize;
 }

 void dataPacketHeader::Serialize (Buffer::Iterator start) const {
   // we write them in network byte order.
   start.WriteU8 (m_packetType); 
   //start.WriteHtonU32 (m_sourceIP);  
   //start.WriteHtonU16 (m_prevHopIP);
   //start.WriteHtonU16 (m_nextHopIdAhead);  
   //start.WriteHtonU16 (m_nextHopIdBehind);
   // use for loop to move 8 bits at a time to store each float
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        start.WriteU8(m_position1X[byteNum]);
   }
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        start.WriteU8(m_position1Y[byteNum]);
   }
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        start.WriteU8(m_Position2X[byteNum]);
   }
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        start.WriteU8(m_Position2Y[byteNum]);
   }
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        start.WriteU8(m_BATime[byteNum]);
   }
 }

 uint32_t dataPacketHeader::Deserialize (Buffer::Iterator start) {
   // we read them in network byte order and store them
   m_packetType = start.ReadU8 ();
   //m_sourceIP = start.ReadNtohU16 ();
   //m_prevHopIP = start.ReadNtohU16 ();
   //m_nextHopIdAhead = start.ReadNtohU16 ();
   //m_nextHopIdBehind = start.ReadNtohU16 ();
   // use for loop to move 8 bits at a time to read each float
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        m_position1X[byteNum] = start.ReadU8 ();
   }
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        m_position1Y[byteNum] = start.ReadU8 ();
   }
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        m_Position2X[byteNum] = start.ReadU8 ();
   }
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        m_Position2Y[byteNum] = start.ReadU8 ();
   }
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        m_BATime[byteNum] = start.ReadU8 ();
   }
   // we return the number of bytes effectively read.
   return GetSerializedSize();
 }
 
 //void dataPacketHeader::SetData (uint8_t packetType, Ipv4Address sourceIP, Ipv4Address prevHopIP, uint16_t nextHopIdAhead, uint16_t nextHopIdBehind, float pos1X, float pos1Y, float pos2X, float pos2Y, float broadcastTime) {
 void dataPacketHeader::SetData (uint8_t packetType, float pos1X, float pos1Y, float pos2X, float pos2Y, float broadcastTime) {
   m_packetType = packetType;
  // m_sourceIP = sourceIP;
  // m_prevHopIP = prevHopIP;
  // m_nextHopIdAhead = nextHopIdAhead;
  // m_nextHopIdBehind = nextHopIdBehind;
   // use for loop to move 8 bits at a time to store float into 4 bytes
   uint8_t* temp = (uint8_t*) &pos1X;
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        m_position1X[byteNum] = temp[byteNum];
   }
   temp = (uint8_t*) &pos1Y;
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        m_position1Y[byteNum] = temp[byteNum];
   }
   temp = (uint8_t*) &pos2X;
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        m_Position2X[byteNum] = temp[byteNum];
   }
   temp = (uint8_t*) &pos2Y;
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        m_Position2Y[byteNum] = temp[byteNum];
   }
   temp = (uint8_t*) &broadcastTime;
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        m_BATime[byteNum] = temp[byteNum];
   }
 }

 uint8_t dataPacketHeader::GetPacketType (void) const {
   return m_packetType;
 }
 
//  uint16_t dataPacketHeader::GetSourceIP (void) const {
//    return m_sourceIP;
//  }

//  uint16_t dataPacketHeader::GetprevHopIP (void) const {
//    return m_prevHopIP;
//  }

//  uint16_t dataPacketHeader::GetNextHopIdAhead (void) const {
//    return m_nextHopIdAhead;
//  }

//  uint16_t dataPacketHeader::GetNextHopIdBehind (void) const {
//    return m_nextHopIdBehind;
//  }

 float dataPacketHeader::GetPosition1X (void) const {
   // use for loop to move 8 bits at a time to store 4 bytes into a float
   float temp;
   uint8_t* asByte = (uint8_t*) &temp;
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        asByte[byteNum] = m_position1X[byteNum];
   }
   return temp;
 }

 float dataPacketHeader::GetPosition1Y (void) const {
   // use for loop to move 8 bits at a time to store 4 bytes into a float
   float temp;
   uint8_t* asByte = (uint8_t*) &temp;
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        asByte[byteNum] = m_position1Y[byteNum];
   }
   return temp;
 }

float dataPacketHeader::GetPosition2X(void) const {
   // use for loop to move 8 bits at a time to store 4 bytes into a float
   float temp;
   uint8_t* asByte = (uint8_t*) &temp;
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        asByte[byteNum] = m_Position2X[byteNum];
   }
   return temp;
 }

float dataPacketHeader::GetPosition2Y(void) const {
   // use for loop to move 8 bits at a time to store 4 bytes into a float
   float temp;
   uint8_t* asByte = (uint8_t*) &temp;
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        asByte[byteNum] = m_Position2Y[byteNum];
   }
   return temp;
}

float dataPacketHeader::GetBroadcastingTime(void) const {
   // use for loop to move 8 bits at a time to store 4 bytes into a float
   float temp;
   uint8_t* asByte = (uint8_t*) &temp;
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        asByte[byteNum] = m_BATime[byteNum];
   }
   return temp;
}

}

}