 /* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef PERIODICPACKETHEADER_H
#define PERIODICPACKETHEADER_H
#include "neighbors.h"

using namespace ns3;
 
class periodicPacketHeader : public Header {
 public:
 
   periodicPacketHeader ();
   virtual ~periodicPacketHeader ();
 
   void SetData (uint8_t packetType, uint16_t id, float posX, float posY, float speedX, float speedY, uint16_t neighborsAhead
                    , uint16_t neighborsBehind, float neighborFurthestAheadX, float neighborFurthestAheadY
                    , float neighborFurthestBehindX, float neighborFurthestBehindY, float avgSpeedX, float avgSpeedY);
            // called when storing information to send in periodic packet
   uint8_t  GetPacketType(void) const;              // periodic packet is type 0
   uint16_t GetNodeId (void) const;                 // id of forwarding node
   uint16_t GetNumNeighborsAhead (void) const;      // number of 1 hop neighbors downstream
   uint16_t GetNumNeighborsBehind (void) const;     // number of 1 hop neighbors upstream
   float GetPositionX  (void) const;                // location of forwarding node
   float GetPositionY  (void) const;
   float GetSpeedX     (void) const;                // current speed of forwarding node
   float GetSpeedY     (void) const;
   float GetNeighborFurthestAheadX (void) const;    // location of node furthest ahead of forwarding node
   float GetNeighborFurthestAheadY (void) const;
   float GetNeighborFurthestBehindX (void) const;   // location of node furthest behind of forwarding node
   float GetNeighborFurthestBehindY (void) const;
   float GetAvgSpeedX (void) const;                 // current average or speed of 1 hop neighbors of forwarding node
   float GetAvgSpeedY (void) const;

   static TypeId GetTypeId (void);
   virtual TypeId GetInstanceTypeId (void) const;
   virtual void Print (std::ostream &os) const;
   virtual void Serialize (Buffer::Iterator start) const;
   virtual uint32_t Deserialize (Buffer::Iterator start);
   virtual uint32_t GetSerializedSize (void) const;
   
 private:
   uint8_t  m_packetType; 
   uint16_t m_nodeId; 
   uint16_t m_neighborsAhead;
   uint16_t m_neighborsBehind;
   uint8_t* m_positionX = new uint8_t[sizeof(float)]; //4, to be able to use float values
   uint8_t* m_positionY = new uint8_t[sizeof(float)];
   uint8_t* m_speedX = new uint8_t[sizeof(float)];
   uint8_t* m_speedY = new uint8_t[sizeof(float)];
   uint8_t* m_neighborFurthestAheadX = new uint8_t[sizeof(float)];
   uint8_t* m_neighborFurthestAheadY = new uint8_t[sizeof(float)];
   uint8_t* m_neighborFurthestBehindX = new uint8_t[sizeof(float)];
   uint8_t* m_neighborFurthestBehindY = new uint8_t[sizeof(float)];
   uint8_t* m_avgSpeedX = new uint8_t[sizeof(float)];
   uint8_t* m_avgSpeedY = new uint8_t[sizeof(float)];
 };
 
 periodicPacketHeader::periodicPacketHeader () {
   // we must provide a public default constructor, 
   // implicit or explicit, but never private.
 }

 periodicPacketHeader::~periodicPacketHeader () {
 }
 
 TypeId periodicPacketHeader::GetTypeId (void) {
   static TypeId tid = TypeId ("ns3::periodicPacketHeader")
     .SetParent<Header> ()
     .AddConstructor<periodicPacketHeader> ();
   return tid;
 }

 TypeId periodicPacketHeader::GetInstanceTypeId (void) const {
   return GetTypeId ();
 }
 
 void periodicPacketHeader::Print (std::ostream &os) const {
   // This method is invoked by the packet printing
   // routines to print the content of my header.
   os << "data=" << m_nodeId << std::endl;
 }

 uint32_t periodicPacketHeader::GetSerializedSize (void) const {
   // we reserve bytes for our header.
   uint32_t totalSize = sizeof(m_packetType) + sizeof(m_nodeId) + sizeof(m_neighborsAhead) + sizeof(m_neighborsBehind)
                            + sizeof(m_positionX) + sizeof(m_positionY) + sizeof(m_speedX) + sizeof(m_speedY)
                            + sizeof(m_neighborFurthestAheadX) + sizeof(m_neighborFurthestAheadY)
                            + sizeof(m_neighborFurthestBehindX) + sizeof(m_neighborFurthestBehindY) + sizeof(m_avgSpeedX) 
                            + sizeof(m_avgSpeedY);  
   //size of header in bytes
   return totalSize;
 }

 void periodicPacketHeader::Serialize (Buffer::Iterator start) const {
   // we write them in network byte order.
   start.WriteU8 (m_packetType); 
   start.WriteHtonU16 (m_nodeId);  
   start.WriteHtonU16 (m_neighborsAhead);
   start.WriteHtonU16 (m_neighborsBehind);
   // use for loop to move 8 bits at a time to store each float
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {  
        start.WriteU8(m_positionX[byteNum]);
   }
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        start.WriteU8(m_positionY[byteNum]);
   }
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        start.WriteU8(m_speedX[byteNum]);
   }
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        start.WriteU8(m_speedY[byteNum]);
   }
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        start.WriteU8(m_neighborFurthestAheadX[byteNum]);
   }
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        start.WriteU8(m_neighborFurthestAheadY[byteNum]);
   }
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        start.WriteU8(m_neighborFurthestBehindX[byteNum]);
   }
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        start.WriteU8(m_neighborFurthestBehindY[byteNum]);
   }
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        start.WriteU8(m_avgSpeedX[byteNum]);
   }
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        start.WriteU8(m_avgSpeedY[byteNum]);
   }
 }

 uint32_t periodicPacketHeader::Deserialize (Buffer::Iterator start) {
   // we read them in network byte order and store them
   m_packetType = start.ReadU8 ();
   m_nodeId = start.ReadNtohU16 ();
   m_neighborsAhead = start.ReadNtohU16 ();
   m_neighborsBehind =start.ReadNtohU16 ();
   // use for loop to move 8 bits at a time to read each float
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        m_positionX[byteNum] = start.ReadU8 ();
   }
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        m_positionY[byteNum] = start.ReadU8 ();
   }
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        m_speedX[byteNum] = start.ReadU8 ();
   }
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        m_speedY[byteNum] = start.ReadU8 ();
   }
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        m_neighborFurthestAheadX[byteNum] = start.ReadU8 ();
   }
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        m_neighborFurthestAheadY[byteNum] = start.ReadU8 ();
   }
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        m_neighborFurthestBehindX[byteNum] = start.ReadU8 ();
   }
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        m_neighborFurthestBehindY[byteNum] = start.ReadU8 ();
   }
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        m_avgSpeedX[byteNum] = start.ReadU8 ();
   }
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        m_avgSpeedY[byteNum] = start.ReadU8 ();
   }
   // we return the number of bytes effectively read.
   return GetSerializedSize();
 }
 
 void periodicPacketHeader::SetData (uint8_t packetType, uint16_t id, float posX, float posY, float speedX, float speedY
                    , uint16_t neighborsAhead, uint16_t neighborsBehind, float neighborFurthestAheadX
                    , float neighborFurthestAheadY, float neighborFurthestBehindX, float neighborFurthestBehindY
                    , float avgSpeedX, float avgSpeedY) {
   m_packetType = packetType;
   m_nodeId = id;
   m_neighborsAhead = neighborsAhead;
   m_neighborsBehind = neighborsBehind;
   // use for loop to move 8 bits at a time to store float into 4 bytes
   uint8_t* temp = (uint8_t*) &posX;
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        m_positionX[byteNum] = temp[byteNum];
   }
   temp = (uint8_t*) &posY;
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        m_positionY[byteNum] = temp[byteNum];
   }
   temp = (uint8_t*) &speedX;
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        m_speedX[byteNum] = temp[byteNum];
   }
   temp = (uint8_t*) &speedY;
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        m_speedY[byteNum] = temp[byteNum];
   }
   temp = (uint8_t*) &neighborFurthestAheadX;
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        m_neighborFurthestAheadX[byteNum] = temp[byteNum];
   }
   temp = (uint8_t*) &neighborFurthestAheadY;
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        m_neighborFurthestAheadY[byteNum] = temp[byteNum];
   }
   temp = (uint8_t*) &neighborFurthestBehindX;
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        m_neighborFurthestBehindX[byteNum] = temp[byteNum];
   }
   temp = (uint8_t*) &neighborFurthestBehindY;
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        m_neighborFurthestBehindY[byteNum] = temp[byteNum];
   }
   temp = (uint8_t*) &avgSpeedX;
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        m_avgSpeedX[byteNum] = temp[byteNum];
   }
   temp = (uint8_t*) &avgSpeedY;
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        m_avgSpeedY[byteNum] = temp[byteNum];
   }
 }

 uint8_t periodicPacketHeader::GetPacketType (void) const {
   return m_packetType;
 }
 
 uint16_t periodicPacketHeader::GetNumNeighborsAhead (void) const {
   return m_neighborsAhead;
 }

uint16_t periodicPacketHeader::GetNumNeighborsBehind (void) const {
   return m_neighborsBehind;
 }

uint16_t periodicPacketHeader::GetNodeId (void) const {
   return m_nodeId;
 }

 float periodicPacketHeader::GetPositionX (void) const {
   // use for loop to move 8 bits at a time to store 4 bytes into a float
   float temp;
   uint8_t* asByte = (uint8_t*) &temp;
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        asByte[byteNum] = m_positionX[byteNum];
   }
   return temp;
 }

 float periodicPacketHeader::GetPositionY (void) const {
   // use for loop to move 8 bits at a time to store 4 bytes into a float
   float temp;
   uint8_t* asByte = (uint8_t*) &temp;
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        asByte[byteNum] = m_positionY[byteNum];
   }
   return temp;
 }

float periodicPacketHeader::GetSpeedX(void) const {
   // use for loop to move 8 bits at a time to store 4 bytes into a float
   float temp;
   uint8_t* asByte = (uint8_t*) &temp;
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        asByte[byteNum] = m_speedX[byteNum];
   }
   return temp;
 }

float periodicPacketHeader::GetSpeedY(void) const {
   // use for loop to move 8 bits at a time to store 4 bytes into a float
   float temp;
   uint8_t* asByte = (uint8_t*) &temp;
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        asByte[byteNum] = m_speedY[byteNum];
   }
   return temp;
}

float periodicPacketHeader::GetNeighborFurthestAheadX(void) const {
   // use for loop to move 8 bits at a time to store 4 bytes into a float
   float temp;
   uint8_t* asByte = (uint8_t*) &temp;
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        asByte[byteNum] = m_neighborFurthestAheadX[byteNum];
   }
   return temp;
}

float periodicPacketHeader::GetNeighborFurthestAheadY(void) const {
   // use for loop to move 8 bits at a time to store 4 bytes into a float
   float temp;
   uint8_t* asByte = (uint8_t*) &temp;
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        asByte[byteNum] = m_neighborFurthestAheadY[byteNum];
   }
   return temp;
}

float periodicPacketHeader::GetNeighborFurthestBehindX(void) const {
   // use for loop to move 8 bits at a time to store 4 bytes into a float
   float temp;
   uint8_t* asByte = (uint8_t*) &temp;
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        asByte[byteNum] = m_neighborFurthestBehindX[byteNum];
   }
   return temp;
}

float periodicPacketHeader::GetNeighborFurthestBehindY(void) const {
   // use for loop to move 8 bits at a time to store 4 bytes into a float
   float temp;
   uint8_t* asByte = (uint8_t*) &temp;
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        asByte[byteNum] = m_neighborFurthestBehindY[byteNum];
   }
   return temp;
}

float periodicPacketHeader::GetAvgSpeedX(void) const {
   // use for loop to move 8 bits at a time to store 4 bytes into a float
   float temp;
   uint8_t* asByte = (uint8_t*) &temp;
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        asByte[byteNum] = m_avgSpeedX[byteNum];
   }
   return temp;
}

float periodicPacketHeader::GetAvgSpeedY(void) const {
   // use for loop to move 8 bits at a time to store 4 bytes into a float
   float temp;
   uint8_t* asByte = (uint8_t*) &temp;
   for (uint32_t byteNum = 0; byteNum < sizeof(float); byteNum++) {
        asByte[byteNum] = m_avgSpeedY[byteNum];
   }
   return temp;
}
 #endif
