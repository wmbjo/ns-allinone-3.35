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

#ifndef PERIODICAPP_H
#define PERIODICAPP_H

#include "periodicPacketHeader.h"
#include "simulationConfiguration.h"
#include "car.h"

namespace ns3 {
  class periodicApp : public Application {
  public:
    periodicApp();
    virtual ~periodicApp();
    void Setup (Ptr<Socket> socket, uint16_t portNum, double packetRate, uint64_t prngRunNumber);  // to create socket
    Ptr<Socket> GetSocketPtr(void);
  private:
    virtual void StartApplication(void);
    virtual void StopApplication(void);
    void ScheduleTx(void);
    void SendPacket(void);
    Ptr<Socket> m_socketptr;
    uint16_t m_portNum;
    double m_packetRate;  // number of packets to send per second
    EventId m_sendEvent;
    bool m_running;
    Ptr<ExponentialRandomVariable> m_tNextPtr;
    uint64_t m_PRNGRunNumber;
  };

  periodicApp::periodicApp() : m_socketptr(NULL), m_portNum(0), m_packetRate(0), m_sendEvent(), m_running(false) {}

  periodicApp::~periodicApp() { 
    m_socketptr = NULL; 
  }

  void periodicApp::Setup(Ptr<Socket> socket, uint16_t portNum, double packetRate, uint64_t prngRunNumber) {
    // initialize values
    m_socketptr = socket;
    m_portNum = portNum;
    m_packetRate = packetRate;  
    RngSeedManager::SetRun(prngRunNumber);  // to be able to set seed
    m_tNextPtr = CreateObject<ExponentialRandomVariable>();  // needed for scheduling
    m_tNextPtr->SetAttribute("Mean", DoubleValue(1/packetRate));  // packetRate is number of packets to send per second
  }

  Ptr<Socket> periodicApp::GetSocketPtr(void) {
    return m_socketptr;
  }

  void periodicApp::StartApplication(void) {
    m_running = true;
    // set socket to broadcast to all neighbors
    m_socketptr->Bind(InetSocketAddress(Ipv4Address::GetAny(), m_portNum));
    m_socketptr->SetAllowBroadcast(true);
    m_socketptr->Connect(InetSocketAddress(Ipv4Address(BROADCAST_ADDRESS), m_portNum));
    SendPacket();
  }

  void periodicApp::StopApplication(void) {
    m_running = false;
    if(m_sendEvent.IsRunning())
      Simulator::Cancel(m_sendEvent);
    if(m_socketptr)
      m_socketptr->Close();
  }

  void periodicApp::SendPacket(void) {
    // get info needed in packet from sockets
    Vector pos = (m_socketptr->GetNode())->GetObject<MobilityModel>()->GetPosition (); // Get position
    Vector vel = (m_socketptr->GetNode())->GetObject<MobilityModel>()->GetVelocity(); // Get velocity
    neighbors* neighbor1Hop = &((m_socketptr->GetNode())->GetObject<car>()->m_neighbors);
        // Determine node furthest ahead downstream and furthest behind upstream
    Vector furthestAhead = Vector3D(NaN,NaN,0);
    int furthestIdxAhead = neighbor1Hop->GetNeighborFurthestAheadByIndex(pos);
    if (furthestIdxAhead >= 0) {
        furthestAhead = Vector3D(neighbor1Hop->GetNeighborPositionX(furthestIdxAhead)
                            , neighbor1Hop->GetNeighborPositionY(furthestIdxAhead),0);
    }
    Vector furthestBehind = Vector3D(NaN,NaN,0); 
    int furthestIdxBehind = neighbor1Hop->GetNeighborFurthestBehindByIndex(pos);
    if (furthestIdxBehind >= 0) {
        furthestBehind = Vector3D(neighbor1Hop->GetNeighborPositionX(furthestIdxBehind)
                            , neighbor1Hop->GetNeighborPositionY(furthestIdxBehind),0);
    }
    // add header with info
    periodicPacketHeader payload;
    payload.SetData(PERIODIC_PACKET_TYPE, (m_socketptr->GetNode())->GetId(), pos.x, pos.y, vel.x, vel.y
            , neighbor1Hop->Get1HopNumNeighborsAhead(), neighbor1Hop->Get1HopNumNeighborsBehind()
            , furthestAhead.x, furthestAhead.y, furthestBehind.x, furthestBehind.y, neighbor1Hop->GetAvgSpeedNeighborX(vel.x)
            , neighbor1Hop->GetAvgSpeedNeighborY(vel.y)); 
            // order is node Id, X, Y, speedX, speedY, numAhead, numBehind, furthestAhead.x, furthestAhead.y, furthestBehind.x, furthestBehind.y
                           // , avg of all 1 hop neighbors' current SpeedX, avg of all 1 hop neighbors' current avgSpeedY
    /*
    std::cout << "******Periodic app Tx" << std::endl;
    std::cout << "My id is: " << (m_socketptr->GetNode())->GetId() << std::endl;
    std::cout << "Received closest is: " << closest.x << std::endl;
    std::cout << "Received closest is: " << closest.y << std::endl;
    std::cout << "Received far is: " << furthest.x << std::endl;
    std::cout << "Received far is: " << furthest.y << std::endl;
    std::cout << "Speed is: " << sumSpeedX/numNeighbors << std::endl;
    std::cout << "Speed is: " << sumSpeedY/numNeighbors << std::endl;
    */
            
    // create empty packet, all info in header
    Ptr<Packet> packetptr = Create<Packet>();
    packetptr->AddHeader(payload);
    m_socketptr->Send(packetptr);
    ScheduleTx();  // will schedule this packet again based m_packetRate
  }

  void periodicApp::ScheduleTx() {
    if(m_running) {
        Time tNext(Seconds(m_tNextPtr->GetValue())); // uses random variable with mean m_packetRate to determine next time to send
        m_sendEvent = Simulator::Schedule(tNext, &periodicApp::SendPacket, this);
    }
  }
}

#endif
