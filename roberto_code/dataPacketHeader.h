 /* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef DATAPACKETHEADER_H
#define DATAPACKETHEADER_H
using namespace ns3;
 
class dataPacketHeader : public Header {
 public:
 
   dataPacketHeader ();
   virtual ~dataPacketHeader ();
 
   void SetData (uint8_t packetType, uint16_t sourceId, uint16_t prevHopId, uint16_t nextHopIdAhead, uint16_t nextHopIdBehind, float pos1X
                                , float pos1Y, float pos2X, float pos2Y, uint64_t packetId, float broadcastTime);
               // called when storing information to send in data packet
   uint8_t  GetPacketType (void) const;               // data packet is type 1
   uint16_t GetSourceId (void) const;                 // id of node who created data packet
   uint16_t GetPrevHopId (void) const;                // id of node who held data packet previously
   uint16_t GetNextHopIdAhead (void) const;           // id of chosen node downstream
   uint16_t GetNextHopIdBehind (void) const;          // id of chosen node upstream
   uint64_t GetPacketId (void) const;                 // id to identify each packet
   float GetPosition1X (void) const;                  // Position 1 and 2 create coordinates for broadcasting area (BA)
   float GetPosition1Y (void) const;
   float GetPosition2X (void) const;
   float GetPosition2Y (void) const;
   float GetBroadcastingTime (void) const;            // time in seconds BA will expire

   static TypeId GetTypeId (void);
   virtual TypeId GetInstanceTypeId (void) const;
   virtual void Print (std::ostream &os) const;
   virtual void Serialize (Buffer::Iterator start) const;
   virtual uint32_t Deserialize (Buffer::Iterator start);
   virtual uint32_t GetSerializedSize (void) const;
   
 private:
   uint8_t  m_packetType;
   uint16_t m_sourceId; 
   uint16_t m_prevHopId; 
   uint16_t m_nextHopIdAhead;
   uint16_t m_nextHopIdBehind;
   uint64_t m_packetId;
   uint8_t*  m_position1X = new uint8_t[sizeof(float)]; //4, to be able to use float values
   uint8_t*  m_position1Y = new uint8_t[sizeof(float)];
   uint8_t*  m_Position2X = new uint8_t[sizeof(float)];
   uint8_t*  m_Position2Y = new uint8_t[sizeof(float)];
   uint8_t*  m_BATime = new uint8_t[sizeof(float)];
 };
 
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
   os << "\nAs unsigned integer\nSource is: " << m_sourceId <<  " Prev hop is: " << m_prevHopId << std::endl;
   os << "Next hop ahead is: " << m_nextHopIdAhead << " Next hop behind is: " << m_nextHopIdBehind << std::endl;
   os << "point 1: " << GetPosition1X() << ", " << GetPosition1Y() << std::endl;
   os << "point 2: " << GetPosition2X() << ", " << GetPosition2Y() << std::endl;
   os << "BATime is: " << GetBroadcastingTime() << " seconds" << std::endl;
   os << "packet id is: " << m_packetId << std::endl;
 }

 uint32_t dataPacketHeader::GetSerializedSize (void) const {
   // we reserve bytes for our header.
   uint32_t totalSize = sizeof(m_packetType) + sizeof(m_sourceId) + sizeof(m_prevHopId) + sizeof(m_nextHopIdAhead) + sizeof(m_nextHopIdBehind)
                 + sizeof(m_position1X) + sizeof(m_position1Y) + sizeof(m_Position2X) + sizeof(m_Position2Y) + sizeof(m_packetId) + sizeof(m_BATime);
   //size of header in bytes
   return totalSize;
 }

 void dataPacketHeader::Serialize (Buffer::Iterator start) const {
   // we write them in network byte order.
   start.WriteU8 (m_packetType); 
   start.WriteHtonU16 (m_sourceId);  
   start.WriteHtonU16 (m_prevHopId);
   start.WriteHtonU16 (m_nextHopIdAhead);  
   start.WriteHtonU16 (m_nextHopIdBehind);
   start.WriteHtonU64 (m_packetId);
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
   m_sourceId = start.ReadNtohU16 ();
   m_prevHopId = start.ReadNtohU16 ();
   m_nextHopIdAhead = start.ReadNtohU16 ();
   m_nextHopIdBehind = start.ReadNtohU16 ();
   m_packetId = start.ReadNtohU64 ();
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
 
 void dataPacketHeader::SetData (uint8_t packetType, uint16_t sourceId, uint16_t prevHopId, uint16_t nextHopIdAhead, uint16_t nextHopIdBehind
                                            , float pos1X, float pos1Y, float pos2X, float pos2Y, uint64_t packetId, float broadcastTime) {
   m_packetType = packetType;
   m_sourceId = sourceId;
   m_prevHopId = prevHopId;
   m_nextHopIdAhead = nextHopIdAhead;
   m_nextHopIdBehind = nextHopIdBehind;
   m_packetId = packetId;
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
 
 uint16_t dataPacketHeader::GetSourceId (void) const {
   return m_sourceId;
 }

 uint16_t dataPacketHeader::GetPrevHopId (void) const {
   return m_prevHopId;
 }

 uint16_t dataPacketHeader::GetNextHopIdAhead (void) const {
   return m_nextHopIdAhead;
 }

 uint16_t dataPacketHeader::GetNextHopIdBehind (void) const {
   return m_nextHopIdBehind;
 }
 
 uint64_t dataPacketHeader::GetPacketId (void) const {
   return m_packetId;
 }

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
 #endif
