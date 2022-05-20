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

#ifndef DATAAPP_H
#define DATAAPP_H

#include "simulationConfiguration.h"
#include "dataPacketHeader.h"
#include "car.h"
#include "routingApp.h"

namespace ns3 {
  class dataApp : public Application {
  public:
    dataApp();
    virtual ~dataApp();
    void Setup (Ptr<Socket> socket, uint16_t portNum, double packetRate, uint64_t prngRunNumber);  // to create socket
    Ptr<Socket> GetSocketPtr(void);
  private:
    virtual void StartApplication(void);
    virtual void StopApplication(void);
    void ScheduleTx(void);
    void SendPacket(void);
    Ptr<Socket> m_socketptr;
    uint16_t m_portNum;
    double m_packetRate; // number of packets to send per second
    EventId m_sendEvent;
    bool m_running;
    Ptr<ExponentialRandomVariable> m_tNextPtr;
    uint64_t m_PRNGRunNumber;
  };

  dataApp::dataApp() : m_socketptr(NULL), m_portNum(0), m_packetRate(0), m_sendEvent(), m_running(false) {}

  dataApp::~dataApp() { 
    m_socketptr = NULL; 
  }

  void dataApp::Setup(Ptr<Socket> socket, uint16_t portNum, double packetRate, uint64_t prngRunNumber) {
    // initialize values
    m_socketptr = socket;
    m_portNum = portNum;
    m_packetRate = packetRate;  
    RngSeedManager::SetRun(prngRunNumber); // to be able to set seed
    m_tNextPtr = CreateObject<ExponentialRandomVariable>();  // needed for scheduling
    m_tNextPtr->SetAttribute("Mean", DoubleValue(1/packetRate));  // packetRate is number of packets to send per second
  }

  Ptr<Socket> dataApp::GetSocketPtr(void) {
    return m_socketptr;
  }

  void dataApp::StartApplication(void) {
    m_running = true;
    // set socket to broadcast to all neighbors
    m_socketptr->Bind(InetSocketAddress(Ipv4Address::GetAny(), m_portNum));
    m_socketptr->SetAllowBroadcast(true);
    m_socketptr->Connect(InetSocketAddress(Ipv4Address(BROADCAST_ADDRESS), m_portNum));
    SendPacket();
  }

  void dataApp::StopApplication(void) {
    m_running = false;
    if(m_sendEvent.IsRunning())
      Simulator::Cancel(m_sendEvent);
    if(m_socketptr)
      m_socketptr->Close();
  }

  void dataApp::SendPacket(void) {
    // create empty packet, all info in header
    Ptr<Packet> packetptr = Create<Packet>(); // may need to specify for what data is being Tx
    // create header object
    dataPacketHeader payload;
    int nexthopIdAhead = -1;
    int nexthopIdBehind = -1;
    uint64_t packetId = packetptr->GetUid();
    // Determine the next hop
    payload.SetData(DATA_PACKET_TYPE, (m_socketptr->GetNode())->GetId(), (m_socketptr->GetNode())->GetId(), nexthopIdAhead, nexthopIdBehind
                , POINT_ONE_X, POINT_ONE_Y, POINT_TWO_X, POINT_TWO_Y, packetId, BROADCASTING_TIME); 
                        // order is packetType, sourceId, prevHopID, nextHopIdAhead, nextHopIdBehind, 1X, 1Y, 2X, 2Y, packetId, time when BA ends       
                                        // source and prev the same in this context     
    Ptr<routingApp> routingAppPtr = (m_socketptr->GetNode())->GetObject<routingApp>();    
    nexthopIdAhead = routingAppPtr->findNextHopDownstream((m_socketptr->GetNode())->GetObject<car>(), payload);
    nexthopIdBehind = routingAppPtr->findNextHopUpstream((m_socketptr->GetNode())->GetObject<car>(), payload);
    // update header with found next hop upstream and downstream
    payload.SetData(DATA_PACKET_TYPE, (m_socketptr->GetNode())->GetId(), (m_socketptr->GetNode())->GetId(), nexthopIdAhead, nexthopIdBehind
                , POINT_ONE_X, POINT_ONE_Y, POINT_TWO_X, POINT_TWO_Y, packetId, BROADCASTING_TIME); 
                        // order is packetType, sourceId, prevHopID, nextHopIdAhead, nextHopIdBehind, 1X, 1Y, 2X, 2Y, packetId, time when BA ends       
                                        // source and prev the same in this context
    packetptr->AddHeader(payload);
    m_socketptr->Send(packetptr);
    ScheduleTx(); // will schedule this packet again based m_packetRate
  }

  void dataApp::ScheduleTx() {
    if(m_running) {
        Time tNext(Seconds(m_tNextPtr->GetValue())); // uses random variable with mean m_packetRate to determine next time to send
        m_sendEvent = Simulator::Schedule(tNext, &dataApp::SendPacket, this);
    }
  }
}
#endif
