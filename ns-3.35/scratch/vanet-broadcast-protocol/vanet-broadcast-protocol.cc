#include "vanet-broadcast-protocol.h"
#include "ns3/udp-l4-protocol.h"

namespace ns3
{
  NS_LOG_COMPONENT_DEFINE("VanetBroadcastProtocol");

  namespace vbp
  {
    NS_OBJECT_ENSURE_REGISTERED(RoutingProtocol);
    const uint8_t RoutingProtocol::PROT_NUMBER = 253;
    const uint32_t RoutingProtocol::VBP_HELLO_PORT = 655;
    uint64_t m_uniformRandomVariable;
    const uint32_t Period_HelloTx = 95;
    const uint32_t Jitter_HelloTx = 10;

    /// Tag used by vbp implementation
    struct DeferredRouteOutputTag : public Tag
    {
      /// Positive if output device is fixed in RouteOutput
      int32_t oif;

      /**
       * Constructor
       *
       * \param o outgoing interface (OIF)
       */
      DeferredRouteOutputTag(int32_t o = -1)
          : Tag(),
            oif(o)
      {
      }

      /**
       * \brief Get the type ID.
       * \return the object TypeId
       */
      static TypeId
      GetTypeId()
      {
        static TypeId tid = TypeId("ns3::vbp::DeferredRouteOutputTag")
                                .SetParent<Tag>()
                                .SetGroupName("Vbp")
                                .AddConstructor<DeferredRouteOutputTag>();
        return tid;
      }

      TypeId
      GetInstanceTypeId() const
      {
        return GetTypeId();
      }

      uint32_t
      GetSerializedSize() const
      {
        return sizeof(int32_t);
      }

      void
      Serialize(TagBuffer i) const
      {
        i.WriteU32(oif);
      }

      void
      Deserialize(TagBuffer i)
      {
        oif = i.ReadU32();
      }

      void
      Print(std::ostream &os) const
      {
        os << "DeferredRouteOutputTag: output interface = " << oif;
      }
    };

    TypeId
    RoutingProtocol::GetTypeId(void)
    {
      static TypeId tid = TypeId("ns3::vbp::RoutingProtocol")
                              .SetParent<Ipv4RoutingProtocol>()
                              .SetGroupName("Vbp")
                              .AddConstructor<RoutingProtocol>();
      return tid;
    }

    RoutingProtocol::RoutingProtocol()
        : m_maxDistance(500),
          m_txCutoffPercentage(1),
          m_vcHighTraffic(0.8),
          m_vcLowTraffic(0.4),
          m_emptyQueuePeriod(0.5),
          m_BroadcastTime(1000000),
          m_routingTable(Time(5)),
          m_helloPacketType('h'),
          m_dataPacketType('d')
    {
      Ptr<VbpNeighbors> m_neighborsListPointer2 = CreateObject<VbpNeighbors>();
      m_neighborsListPointer->AggregateObject(m_neighborsListPointer2);
      m_broadcastArea[0] = NAN;
      m_broadcastArea[1] = NAN;
      m_broadcastArea[2] = NAN;
      m_broadcastArea[3] = NAN;
      Ptr<VbpQueue> m_queuePointer2 = CreateObject<VbpQueue>();
      m_queuePointer->AggregateObject(m_queuePointer2);
      //VbpNextHopFinder m_nextHopFinder;
      // Ptr<VbpNextHopFinder> m_nextHopPointer2 = CreateObject<VbpNextHopFinder>();
      //m_nextHopPointer->AggregateObject(m_nextHopPointer2);
      //m_nextHopPointer = &nextHopFinder;
      ScheduleEmptyQueue();
    }

    RoutingProtocol::~RoutingProtocol()
    {
    }

    Ptr<Ipv4Route>
    RoutingProtocol::RouteOutput(Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
    {
      std::cout << "Route Output" << std::endl;
      Ptr<Ipv4Route> route;
      NS_LOG_FUNCTION (this << header << (oif ? oif->GetIfIndex () : 0));
      if (m_socketAddresses.empty ())
        {
          sockerr = Socket::ERROR_NOROUTETOHOST;
          NS_LOG_LOGIC ("No vbp interfaces");
          return route;
        }
      sockerr = Socket::ERROR_NOTERROR;
      VbpRoutingHeader routingHeader;
      RoutingTableEntry rt;
      Ipv4Address dst = header.GetDestination();
      Ipv4Address src = header.GetSource();
      Ipv4InterfaceAddress iface = m_socketAddresses.begin()->second;
      Ipv4Address origin = iface.GetAddress();
      Ptr<NetDevice> dev = m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (iface.GetLocal ()));
      routingHeader.SetData(m_dataPacketType, origin, m_broadcastArea[0], m_broadcastArea[1], m_broadcastArea[2], m_broadcastArea[3], m_BroadcastTime);
      p->AddHeader(routingHeader);
      //dev,iface
      Ipv4Address nextHopAhead;
      Ipv4Address nextHopBehind;
      std::cout << "Route Output Packet Type: " << routingHeader.GetPacketType() << std::endl;
      if (FindFirstHop(&nextHopAhead, &nextHopBehind)) //find next hop
      {
         rt.SetOutputDevice(dev);
         rt.SetInterface(iface);
         Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol>();
         std::cout << "Find First Hop: " << std::endl;
         if (nextHopAhead != Ipv4Address("102.102.102.102") && nextHopBehind != Ipv4Address("102.102.102.102")) //case: hops both ahead and behind
         {
          std::cout << "Next Hop Ahead and Behind" << std::endl;
          // next hop ahead
          Ptr<Ipv4Route> routeDownstream;
          rt.SetNextHop(nextHopAhead);
          routeDownstream = rt.GetRoute();
          l3->Send(p, src, dst, PROT_NUMBER, routeDownstream);
          // next hop behind
          Ptr<Packet> q = p->Copy();
          Ptr<Ipv4Route> routeUpstream;
          rt.SetNextHop(nextHopBehind);
          routeUpstream = rt.GetRoute();
          l3->Send(q, src, dst, PROT_NUMBER, routeUpstream);
         }
         else if (nextHopAhead != Ipv4Address("102.102.102.102"))
         {
          std::cout << "Next Hop Ahead Only" << std::endl;
          Ptr<Ipv4Route> routeDownstream;
          rt.SetNextHop(nextHopAhead);
          routeDownstream = rt.GetRoute();
          l3->Send(p, src, dst, PROT_NUMBER, routeDownstream);
         } 
         else
         {
          std::cout << "Next Hop Behind Only" << std::endl;
          Ptr<Ipv4Route> routeUpstream;
          rt.SetNextHop(nextHopBehind);
          routeUpstream = rt.GetRoute();
          l3->Send(p, src, dst, PROT_NUMBER, routeUpstream);
         }
      } 
      else
      {
        std::cout << "Valid Route Not Found" << std::endl;
        // Valid route not found, return loopback
        uint32_t iif = (oif ? m_ipv4->GetInterfaceForDevice (oif) : -1);
        DeferredRouteOutputTag tag (iif);
        NS_LOG_DEBUG ("Valid Route not found");
        if (!p->PeekPacketTag (tag))
        {
          p->AddPacketTag (tag);
        }
        route = LoopbackRoute(header, oif);
        //  std::cout << "Socket Error " << std::endl; 
        //  sockerr = Socket::ERROR_NOROUTETOHOST;
      }
      return route;
    }
    
    bool
    RoutingProtocol::RouteInput(Ptr<const Packet> p, const Ipv4Header &header,
                                Ptr<const NetDevice> idev, UnicastForwardCallback ucb,
                                MulticastForwardCallback mcb, LocalDeliverCallback lcb, ErrorCallback ecb)
    {
      NS_LOG_FUNCTION (this << p->GetUid () << header.GetDestination () << idev->GetAddress ());
      if (m_socketAddresses.empty())
      {
        NS_LOG_LOGIC("No vbp interfaces");
        return false;
      }
      NS_ASSERT (m_ipv4 != 0);
      NS_ASSERT (p != 0);
      // Check if input device supports IP
      NS_ASSERT (m_ipv4->GetInterfaceForDevice (idev) >= 0);
      int32_t iif = m_ipv4->GetInterfaceForDevice(idev);
      Ipv4Address dst = header.GetDestination();
      Ipv4Address src = header.GetSource();
      // Deferred route request
      if (idev == m_lo)
      {
        //std::cout << "Deferred Route Request" << std::endl;
        DeferredRouteOutputTag tag;
        if (p->PeekPacketTag (tag))
        {
          //std::cout << "Deferred Route Request 2" << std::endl;
          DeferredRouteOutput (p, header, ucb, ecb);
          return false;
          //return true;
        }        
      }
      // VBP is not a multicast routing protocol
      if (dst.IsMulticast())
      {
        NS_LOG_LOGIC("Multicast Return False");
        return false;
      }
      Ipv4InterfaceAddress iface = m_socketAddresses.begin()->second;
      // Unicast local delivery
      if (m_ipv4->IsDestinationAddress(dst, iif))
      {
        if (lcb.IsNull() == false)
        {
          NS_LOG_LOGIC ("Unicast local delivery to " << dst);
          uint8_t protocol_numb = header.GetProtocol();
          if (protocol_numb == PROT_NUMBER)
          {
            std::cout << "Protocol Number = 253, VBP Data Packet" << std::endl;
            bool packetSentIndicator = false;
            bool lcbIndicator = RoutePacket(p, dst, src, &packetSentIndicator); //true lcb. false no lcb
            if (lcbIndicator)
            {
              VbpRoutingHeader routingHeader;
              p->RemoveHeader(routingHeader);
              lcb(p,header,iif);
            }
            return true;
          }
          // else //PROT_NUMB != 253 (hello packet)
          // {
          // VbpHelloHeader helloHeader;
          // p->PeekHeader(helloHeader);
          // std::cout << "Packet Type LCB Hello " << helloHeader.GetPacketType() << std::endl;
          // }
          // if (header.GetProtocol () == 17) // add #include "udp-l4-protocol.h" to make this work (udp protocol number = 17). != does not print anything
          // {
          //   std::cout << "Detected UDP Protocol 2 " << std::endl;
          // }
          // else
          // {
          //   std::cout << "Never prints out else" << std::endl;
          // }
          lcb(p, header, iif); //non-vbp-data-packet local callback
        }
        else
        {
          NS_LOG_ERROR ("Unable to deliver packet locally due to null callback " << p->GetUid () << " from " << src);
          ecb (p, header, Socket::ERROR_NOROUTETOHOST);
        }
        return true;
      }
      std::cout << "Forwarding Error in RouteInput " << std::endl;
      NS_LOG_ERROR ("Unable to forward packet due to not being a VANET Broadcast Protocol data packet " << p->GetUid () << " from " << src);
      ecb (p, header, Socket::ERROR_NOROUTETOHOST);

      // // Forward packet
      // // not a loopback, local delivery or multicast
      // VbpRoutingHeader routingHeader;
      // //Ipv4InterfaceAddress iface = m_socketAddresses.begin()->second;
      // p->PeekHeader(routingHeader);
      // routingHeader.SetPrevHopIP(iface.GetAddress()); 
      // routingHeader.Print(std::cout);
      // Ipv4Address nextHop;
      // std::cout << "IS BROADCAST " << nextHop.IsBroadcast() << std::endl;
      // if (FindNextHop(&nextHop)) //find next hop
      // {
      //    //std::cout << "RI Find Next Hop TRUE: " << nextHop << std::endl;
      //    RoutingTableEntry rt;
      //    Ptr<NetDevice> dev = m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (iface.GetLocal ()));
      //    rt.SetNextHop(nextHop); //set route
      //    rt.SetOutputDevice(dev);
      //    rt.SetInterface(iface);
      //    ucb(rt.GetRoute(),p,header);
      //    return true;
      // }
      return false;

    }

    void
    RoutingProtocol::NotifyInterfaceUp(uint32_t interface)
    {
      if (interface > 1)
      {
        NS_LOG_WARN("VBP does not work with more then one interface.");
      }
      NS_LOG_FUNCTION(this << m_ipv4->GetAddress(interface, 0).GetLocal());
      Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol>();
      if (l3->GetNAddresses(interface) > 1)
      {
        NS_LOG_WARN("VBP does not work with more then one address per each interface.");
      }
      Ipv4InterfaceAddress iface = l3->GetAddress(interface, 0);
      if (iface.GetLocal() == Ipv4Address("127.0.0.1"))
      {
        return;
      }
      //include check that m_socketaddresses is empty and m_socketSubnetBroadcastAddresses is empty. Print out message only one interface is allowed if check fails
      // Create a socket to listen only on this interface
      Ptr<Socket> socket = Socket::CreateSocket(GetObject<Node>(), UdpSocketFactory::GetTypeId());
      NS_ASSERT(socket != 0);
      socket->SetRecvCallback(MakeCallback(&RoutingProtocol::RecvVbp, this));
      socket->BindToNetDevice(l3->GetNetDevice(interface));
      socket->Bind(InetSocketAddress(iface.GetLocal(), VBP_HELLO_PORT));
      socket->SetAllowBroadcast(true);
      socket->SetIpRecvTtl(true);
      m_socketAddresses.insert(std::make_pair(socket, iface));

      // create also a subnet broadcast socket
      socket = Socket::CreateSocket(GetObject<Node>(), UdpSocketFactory::GetTypeId());
      NS_ASSERT(socket != 0);
      socket->SetRecvCallback(MakeCallback(&RoutingProtocol::RecvVbp, this));
      socket->BindToNetDevice(l3->GetNetDevice(interface));
      socket->Bind(InetSocketAddress(iface.GetBroadcast(), VBP_HELLO_PORT));
      socket->SetAllowBroadcast(true);
      socket->SetIpRecvTtl(true);
      m_socketSubnetBroadcastAddresses.insert(std::make_pair(socket, iface));

      NS_LOG_FUNCTION(Simulator::Now().GetSeconds() << " Seconds --- " << "NotifyInterfaceUp " << "--- " << m_ipv4->GetNInterfaces() << " Interfaces");

      m_thisNode = socket->GetNode();
      NS_LOG_FUNCTION("This Node: " << m_thisNode->GetObject<MobilityModel>()->GetPosition());
      m_neighborsListPointer->GetObject<VbpNeighbors>()->SetThisNode(m_thisNode);
    }

    void
    RoutingProtocol::NotifyInterfaceDown(uint32_t interface)
    {
      NS_LOG_FUNCTION (this << m_ipv4->GetAddress (interface, 0).GetLocal ());
    }

    void
    RoutingProtocol::NotifyAddAddress(uint32_t interface, Ipv4InterfaceAddress address)
    {
      NS_LOG_FUNCTION (this << " interface " << interface << " address " << address);
      Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol>();

      if (!l3->IsUp(interface))
      {
        return;
      }
    }

    void
    RoutingProtocol::NotifyRemoveAddress(uint32_t interface, Ipv4InterfaceAddress address)
    {
      NS_LOG_FUNCTION (this);
    }

    void
    RoutingProtocol::RecvVbp(Ptr<Socket> socket)
    {
      NS_LOG_FUNCTION(this << socket);
      Address sourceAddress;
      Ptr<Packet> packet = socket->RecvFrom(sourceAddress);
      InetSocketAddress inetSourceAddr = InetSocketAddress::ConvertFrom(sourceAddress);
      std::cout << "PORT " << inetSourceAddr.GetPort() << std::endl;
      Ipv4Address sender = inetSourceAddr.GetIpv4();
      Ipv4Address receiver;

      if (m_socketAddresses.find(socket) != m_socketAddresses.end())
      {
        receiver = m_socketAddresses[socket].GetLocal();
      }
      else if (m_socketSubnetBroadcastAddresses.find(socket) != m_socketSubnetBroadcastAddresses.end())
      {
        receiver = m_socketSubnetBroadcastAddresses[socket].GetLocal();
      }
      else
      {
        NS_ASSERT_MSG(false, "Received a packet from an unknown socket");
      }
      // remove the header from the packet:
      VbpHelloHeader destinationHeader;
      packet->PeekHeader(destinationHeader);
      NS_LOG_FUNCTION("---Tx From --- " << sender);
      NS_LOG_FUNCTION("---Tx To --- " << receiver);
      NS_LOG_FUNCTION( "---Begin Header Information --- ");
      NS_LOG_FUNCTION("Packet Type: " << destinationHeader.GetPacketType());
      NS_LOG_FUNCTION("Position X: " << destinationHeader.GetPositionX());
      NS_LOG_FUNCTION("Position Y: " << destinationHeader.GetPositionY());
      NS_LOG_FUNCTION("Speed X: " << destinationHeader.GetSpeedX());
      NS_LOG_FUNCTION("Speed Y: " << destinationHeader.GetSpeedY());
      NS_LOG_FUNCTION("---End Header Information --- ");
      if (destinationHeader.GetPacketType() == m_helloPacketType)
      {
        RecvHello(packet, receiver, sender);
        std::cout << "Neighbors List: " << "Receiver " << receiver << " Sender " << sender << std::endl;
        m_neighborsListPointer->GetObject<VbpNeighbors>()->PrintNeighbors2();
      }
    }

    void
    RoutingProtocol::RecvHello(Ptr<Packet> p, Ipv4Address receiver, Ipv4Address sender)
    {
      VbpHelloHeader helloHeader;
      p->PeekHeader(helloHeader);

      // determine if forwarding node is ahead=1 or behind=0 by using dot product
      float dotProduct;
      float dotProductVel;

      Vector receiveNodePos = m_thisNode->GetObject<MobilityModel>()->GetPosition();
      Vector diff = Vector3D(helloHeader.GetPositionX(), helloHeader.GetPositionY(), 0) - receiveNodePos; // vector pointing from receiving node to forwarding node
      Vector receiveNodeVelocity = m_thisNode->GetObject<MobilityModel>()->GetVelocity();

      if (receiveNodeVelocity.GetLength() == 0)
      {
        // if receiving node not moving, don't process anymore
        return;
      }

      dotProductVel = receiveNodeVelocity.x * helloHeader.GetSpeedX() + receiveNodeVelocity.y * helloHeader.GetSpeedY();
      if (dotProductVel <= 0)
      {
        // if velocity vectors do not align, don't process because neighbor moving in opposite direction
        return;
      }

      dotProduct = receiveNodeVelocity.x * diff.x + receiveNodeVelocity.y * diff.y;
      uint16_t direction; // 0 = behind, 1 = ahead
      if (dotProduct >= 0)
      {
        // if dot product positive, then ahead
        direction = 1;
      }
      else
      {
        // if dot product negative, then behind
        direction = 0;
      }

      // use received packet to update neighbors information in object neighbors
      // will add node as new neighbor or update information for that neighbor
      m_neighborsListPointer->GetObject<VbpNeighbors>()->AddNode(sender,
                                                                 direction,
                                                                 helloHeader.GetNumNeighborsAhead(),
                                                                 helloHeader.GetNumNeighborsBehind(),
                                                                 helloHeader.GetPositionX(),
                                                                 helloHeader.GetPositionY(),
                                                                 helloHeader.GetSpeedX(),
                                                                 helloHeader.GetSpeedY(),
                                                                 helloHeader.GetNeighborFurthestAheadX(),
                                                                 helloHeader.GetNeighborFurthestAheadY(),
                                                                 helloHeader.GetNeighborFurthestBehindX(),
                                                                 helloHeader.GetNeighborFurthestBehindY(),
                                                                 helloHeader.GetAvgSpeedX(),
                                                                 helloHeader.GetAvgSpeedY());
      // order is neighbor's node Id, direction, numAhead, numBehind, X, Y, speedX, speedY, furthestAhead.x, furthestAhead.y, furthestBehind.x
      // , furthestBehind.y, avg SpeedX, avgSpeedY
    }

    void
    RoutingProtocol::SetIpv4(Ptr<Ipv4> ipv4)
    {
      NS_ASSERT(ipv4 != 0);

      m_ipv4 = ipv4; // m_ipv4 set here

      // Create lo route. It is asserted that the only one interface up for now is loopback
      NS_ASSERT(m_ipv4->GetNInterfaces() == 1 && m_ipv4->GetAddress(0, 0).GetLocal() == Ipv4Address("127.0.0.1"));
      m_lo = m_ipv4->GetNetDevice(0);
      NS_ASSERT(m_lo != 0);
      return;
    }

    void
    RoutingProtocol::PrintRoutingTable(Ptr<OutputStreamWrapper> stream, Time::Unit unit) const
    {
      *stream->GetStream() << "Node: " << m_ipv4->GetObject<Node>()->GetId()
                           << "; Time: " << Now().As(unit)
                           << ", Local time: " << m_ipv4->GetObject<Node>()->GetLocalTime().As(unit)
                           << ", VBP Routing table" << std::endl;

      m_routingTable.Print(stream, unit);
      *stream->GetStream() << std::endl;
    }

    void
    RoutingProtocol::SendHello()
    {
      // In this version of SendHello I fill SetData with real values. This is a work in progress
      for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin(); j != m_socketAddresses.end(); ++j)
      {

        Ptr<Socket> socket = j->first;
        Ipv4InterfaceAddress iface = j->second;
        Ptr<Packet> packet = Create<Packet>();
        // create header here
        VbpHelloHeader HelloHeader;

        // get info needed in packet from sockets
        Vector pos = m_thisNode->GetObject<MobilityModel>()->GetPosition();
        Vector vel = m_thisNode->GetObject<MobilityModel>()->GetVelocity(); 
        // set dummy values to header setData (pass hardcoded values)
        Vector furthestAhead = Vector3D(NAN, NAN, 0);
        int furthestIdxAhead = m_neighborsListPointer->GetObject<VbpNeighbors>()->GetNeighborFurthestAheadByIndex(pos);
        if (furthestIdxAhead >= 0)
        {
          furthestAhead = Vector3D(m_neighborsListPointer->GetObject<VbpNeighbors>()->GetNeighborPositionX(furthestIdxAhead), m_neighborsListPointer->GetObject<VbpNeighbors>()->GetNeighborPositionY(furthestIdxAhead), 0);
        }
        Vector furthestBehind = Vector3D(NAN, NAN, 0);
        int furthestIdxBehind = m_neighborsListPointer->GetObject<VbpNeighbors>()->GetNeighborFurthestBehindByIndex(pos);
        if (furthestIdxBehind >= 0)
        {
          furthestBehind = Vector3D(m_neighborsListPointer->GetObject<VbpNeighbors>()->GetNeighborPositionX(furthestIdxBehind), m_neighborsListPointer->GetObject<VbpNeighbors>()->GetNeighborPositionY(furthestIdxBehind), 0);
        }
        HelloHeader.SetData(m_helloPacketType,
                            pos.x,
                            pos.y,
                            vel.x,
                            vel.y,
                            m_neighborsListPointer->GetObject<VbpNeighbors>()->Get1HopNumNeighborsAhead(),
                            m_neighborsListPointer->GetObject<VbpNeighbors>()->Get1HopNumNeighborsBehind(),
                            furthestAhead.x,
                            furthestAhead.y,
                            furthestBehind.x,
                            furthestBehind.y,
                            m_neighborsListPointer->GetObject<VbpNeighbors>()->GetAvgSpeedNeighborX(vel.x),
                            m_neighborsListPointer->GetObject<VbpNeighbors>()->GetAvgSpeedNeighborY(vel.y));

        // add header to packet
        packet->AddHeader(HelloHeader);
        // print the content of my packet on the standard output.
        //packet->Print(std::cout);

        // Send to all-hosts broadcast if on /32 addr, subnet-directed otherwise
        Ipv4Address destination;
        if (iface.GetMask() == Ipv4Mask::GetOnes())
        {
          destination = Ipv4Address("255.255.255.255");
        }
        else
        {
          destination = iface.GetBroadcast();
        }
        Time jitter = Time(MilliSeconds(Period_HelloTx + m_uniformRandomVariable->GetInteger(0, Jitter_HelloTx)));
        Simulator::Schedule(jitter, &RoutingProtocol::SendHello, this);
        SendTo(socket, packet, destination);
      }
      m_neighborsListPointer->GetObject<VbpNeighbors>()->PrintNeighborState();
    }

    void
    RoutingProtocol::SendTo(Ptr<Socket> socket, Ptr<Packet> packet, Ipv4Address destination)
    {
      socket->SendTo(packet, 0, InetSocketAddress(destination, VBP_HELLO_PORT));
    }

    void 
    RoutingProtocol::StartHelloTx()
    {
      m_uniformRandomVariable = CreateObject<UniformRandomVariable>();
      Time jitter = Time(MilliSeconds(Period_HelloTx + m_uniformRandomVariable->GetInteger(0, Jitter_HelloTx)));
      Simulator::Schedule(jitter, &RoutingProtocol::SendHello, this);
    }

    std::vector<float> 
    RoutingProtocol::GetBroadcastArea()
    {
        return m_broadcastArea;
    }

    void
    RoutingProtocol::SetBroadcastArea(std::vector<float> broadcastArea)
    {
        m_broadcastArea[0] = broadcastArea[0];
        m_broadcastArea[1] = broadcastArea[1];
        m_broadcastArea[2] = broadcastArea[2];
        m_broadcastArea[3] = broadcastArea[3];
    }

   void
   RoutingProtocol::EmptyQueue()
   {
     Ipv4Address nextHopAhead;
     Ipv4Address nextHopBehind;
     if (!FindFirstHop(&nextHopAhead, &nextHopBehind))
     {
       return;
     }
     uint16_t queueSize =  m_queuePointer->GetObject<VbpQueue>()->GetQueueSize();
     if (queueSize == 0)
     {
        return;
     }
     Ipv4InterfaceAddress iface = m_socketAddresses.begin()->second;
     Ptr<NetDevice> dev = m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (iface.GetLocal ()));
     //RoutingTableEntry rt;
     while(m_queuePointer->GetObject<VbpQueue>()->GetQueueSize() > 0)
     {
      //test to determine findfirsthop or findnexthop
      Ipv4Address thisVehicleIP = iface.GetAddress();
      Ptr<const Packet> pkt = m_queuePointer->GetObject<VbpQueue>()->PeekPacket();
      VbpRoutingHeader routingHeader;
      pkt->PeekHeader(routingHeader);
      if (thisVehicleIP == routingHeader.GetPrevHopIP()) //check if first hop
      {
        if (FindFirstHop(&nextHopAhead, &nextHopBehind)) //find next hop
        {
          Ptr<const Packet> p = m_queuePointer->GetObject<VbpQueue>()->GetPacket();
          Ipv4Header header = m_queuePointer->GetObject<VbpQueue>()->GetHeader();
          //Ipv4RoutingProtocol::UnicastForwardCallback ucb = m_queuePointer->GetObject<VbpQueue>()->GetUcb();
          Ipv4Address dst = header.GetDestination();
          Ipv4Address src = header.GetSource();
          RoutingTableEntry rt;
          rt.SetOutputDevice(dev);
          rt.SetInterface(iface);
          Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol>();
          std::cout << "Find First Hop: " << std::endl;
          if (nextHopAhead != Ipv4Address("102.102.102.102") && nextHopBehind != Ipv4Address("102.102.102.102")) //case: hops both ahead and behind
          {
            std::cout << "Next Hop Ahead and Behind" << std::endl;
            // next hop ahead
            Ptr<Ipv4Route> routeDownstream;
            rt.SetNextHop(nextHopAhead);
            routeDownstream = rt.GetRoute();
            l3->Send(p, src, dst, PROT_NUMBER, routeDownstream);
            // next hop behind
            Ptr<Packet> q = p->Copy();
            Ptr<Ipv4Route> routeUpstream;
            rt.SetNextHop(nextHopBehind);
            routeUpstream = rt.GetRoute();
            l3->Send(q, src, dst, PROT_NUMBER, routeUpstream);
          }
          else if (nextHopAhead != Ipv4Address("102.102.102.102"))
          {
            std::cout << "Next Hop Ahead Only" << std::endl;
            Ptr<Ipv4Route> routeDownstream;
            rt.SetNextHop(nextHopAhead);
            routeDownstream = rt.GetRoute();
            l3->Send(p, src, dst, PROT_NUMBER, routeDownstream);
          } 
          else
          {
            std::cout << "Next Hop Behind Only" << std::endl;
            Ptr<Ipv4Route> routeUpstream;
            rt.SetNextHop(nextHopBehind);
            routeUpstream = rt.GetRoute();
            l3->Send(p, src, dst, PROT_NUMBER, routeUpstream);
          }
          if (m_queuePointer->GetObject<VbpQueue>()->GetQueueSize() == 0)
          {
            return;
          }
        }
        else
        {
          return;
        }
      }
      else // this is a forwarding vehicle
      {
        Ptr<const Packet> p = m_queuePointer->GetObject<VbpQueue>()->GetPacket();
        Ipv4Header header = m_queuePointer->GetObject<VbpQueue>()->GetHeader();
        Ipv4Address dst = header.GetDestination();
        Ipv4Address src = header.GetSource();
        bool packetSentIndicator = false;
        RoutePacket(p,dst,src, &packetSentIndicator);
        if (!packetSentIndicator || (m_queuePointer->GetObject<VbpQueue>()->GetQueueSize() == 0))
        {
          return;
        }
       
      }
        // //std::cout << "Empty Queue NextHop: " << nextHop << std::endl;
        // Ptr<const Packet> p = m_queuePointer->GetObject<VbpQueue>()->GetPacket();
        // Ipv4Header header = m_queuePointer->GetObject<VbpQueue>()->GetHeader();
        // Ipv4RoutingProtocol::UnicastForwardCallback ucb = m_queuePointer->GetObject<VbpQueue>()->GetUcb();
        // Ipv4InterfaceAddress iface = m_socketAddresses.begin()->second;
        // Ptr<NetDevice> dev = m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (iface.GetLocal ()));
        // // std::cout << "Empty Queue Local Address: " << iface.GetLocal () << std::endl;
        // RoutingTableEntry rt;
        // rt.SetNextHop(nextHopAhead); //set route
        // rt.SetOutputDevice(dev);
        // rt.SetInterface(iface);
        // Ptr<Ipv4Route> route = rt.GetRoute();
        // ucb(route, p, header);
        // // Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol>();
        // // l3->Send(p, source, destination, protocol, route);
        //queueSize--;
     }
   }

    void 
    RoutingProtocol::ScheduleEmptyQueue()
    {
        EmptyQueue();
        Simulator::Schedule(Seconds(m_emptyQueuePeriod), &RoutingProtocol::ScheduleEmptyQueue, this);  
    }

    bool
    RoutingProtocol::FindFirstHop(Ipv4Address* nextHopAheadPtr,Ipv4Address* nextHopBehindPtr)
    {
      Vector vehiclePos = m_thisNode->GetObject<MobilityModel>()->GetPosition();
      Vector vehicleVel = m_thisNode->GetObject<MobilityModel>()->GetVelocity();
      float upperLeftBA_x = m_broadcastArea[0];
      float upperLeftBA_y = m_broadcastArea[1];
      float lowerRightBA_x = m_broadcastArea[2];
      float lowerRightBA_y = m_broadcastArea[3];
      Vector centerBA = Vector3D((upperLeftBA_x+lowerRightBA_x)/2,(upperLeftBA_y+lowerRightBA_y)/2,0);
      Vector vehicleToBA = centerBA - vehiclePos;
      bool movingToBA = (vehicleVel.x*vehicleToBA.x + vehicleVel.y*vehicleToBA.y) > 0; // true if moving towards BA
      if (movingToBA) 
      {
        nextHopAheadPtr->Set(FindNextHopDownstream(centerBA, movingToBA).Get());
      }
      nextHopBehindPtr->Set(FindNextHopUpstream(centerBA, movingToBA).Get());
      if (*nextHopAheadPtr == Ipv4Address("102.102.102.102") && *nextHopBehindPtr == Ipv4Address("102.102.102.102"))
      {
        std::cout << "Find First Hop Returns False" << std::endl;
        return false;
      }
      std::cout << "Find First Hop Returns True" << std::endl;
      return true;
    }
    //original find first hop, void
    // bool
    // RoutingProtocol::FindNextHop(Ipv4Address* nextHopAheadPtr,Ipv4Address* nextHopBehindPtr)
    // {
    //   if (m_neighborsListPointer->GetObject<VbpNeighbors>()->Get1HopNumNeighborsAhead() == 0)
    //   {
    //     return false;
    //   }
    //   Ipv4Address nextHop = m_neighborsListPointer->GetObject<VbpNeighbors>()->Get1HopNeighborIPAhead(0);
    //   nextHopPtr->Set(nextHop.Get());
    //   return true;

    // }


    Ipv4Address
    RoutingProtocol::FindNextHopDownstream(Vector centerBA, bool movingToBA)
    {
      std::cout << "Find Next Hop Downstream " << std::endl;
      if (!movingToBA)
      {
        return Ipv4Address("102.102.102.102"); // BA in upstream, not downstream
      }
      Ptr<VbpNeighbors> neighborInfo = m_neighborsListPointer->GetObject<VbpNeighbors>();
      uint16_t numNeighbors = neighborInfo->Get1HopNumNeighborsAhead();
      if (numNeighbors == 0) 
      {
        return Ipv4Address("102.102.102.102"); // same as receiving node since no neighbors
      }
      Vector vehiclePos = m_thisNode->GetObject<MobilityModel>()->GetPosition();
      Vector vehicleVel = m_thisNode->GetObject<MobilityModel>()->GetVelocity();
      float LOS = neighborInfo->GetLosCalculation(vehiclePos, vehicleVel); // need to check if neighbor is moving toward broadcast area, if not moving towards bA then ignore
      float stopDist = vehicleVel.GetLength()*3; // stopping distance according to DMV= speed*3 seconds. Distance needed to stop   
          // include paramter to determine if msg moves ahead or backwards
      int nextHopIdx;
      float neighborhoodSpeed = Vector3D(neighborInfo->GetNeighborHoodSpeedMeanX(), neighborInfo->GetNeighborHoodSpeedMeanY(),0).GetLength();
      if (LOS>m_vcHighTraffic) //high traffic
      {
        nextHopIdx = FindNextHopHighTrafficDownstream(centerBA, vehiclePos, stopDist);
        std::cout << "Next Hop IDX A " << nextHopIdx << std::endl; 
            //return node that is closest to broadcast area(Dn)
      } 
      else if (LOS<m_vcLowTraffic) //no traffic
      {
        nextHopIdx = FindNextHopLowTrafficDownstream(neighborhoodSpeed, centerBA, vehiclePos, stopDist);
        std::cout << "Next Hop IDX B " << nextHopIdx << std::endl; 
            //return node with MDT
      }
      else //medium traffic
      {
        nextHopIdx = FindNextHopMidTrafficDownstream(neighborhoodSpeed, centerBA, vehiclePos, stopDist);
        std::cout << "Next Hop IDX C " << nextHopIdx << std::endl; 
            //return node with smallest Dn and MDT
      }
      if (nextHopIdx >= 0)
      {
        return neighborInfo->Get1HopNeighborIP(nextHopIdx);
      }
      return Ipv4Address("102.102.102.102");
    }

    Ipv4Address
    RoutingProtocol::FindNextHopUpstream(Vector centerBA, bool movingToBA)
    {
      std::cout << "Find Next Hop Upstream " << std::endl;
      Ptr<VbpNeighbors> neighborInfo = m_neighborsListPointer->GetObject<VbpNeighbors>();
      uint16_t numNeighbors = neighborInfo->Get1HopNumNeighborsAhead();
      if (numNeighbors == 0) 
      {
        return Ipv4Address("102.102.102.102"); // same as receiving node since no neighbors
      }
      Vector vehiclePos = m_thisNode->GetObject<MobilityModel>()->GetPosition();
      Vector vehicleVel = m_thisNode->GetObject<MobilityModel>()->GetVelocity();
      float LOS = neighborInfo->GetLosCalculation(vehiclePos, vehicleVel);
      float stopDist = vehicleVel.GetLength()*3;
      int nextHopIdx;
      float neighborhoodSpeed = Vector3D(neighborInfo->GetNeighborHoodSpeedMeanX(), neighborInfo->GetNeighborHoodSpeedMeanY(),0).GetLength();
      if (!movingToBA)
      {
        if (LOS>m_vcHighTraffic)
        {
          nextHopIdx = FindNextHopHighTrafficUpstreamAwayBA(centerBA, vehiclePos, stopDist);
          std::cout << "Next Hop IDX D " << nextHopIdx << std::endl; 
        } 
        else if (LOS<m_vcLowTraffic) 
        {
          nextHopIdx = FindNextHopLowTrafficUpstreamAwayBA(neighborhoodSpeed, centerBA, vehiclePos, stopDist);
          std::cout << "Next Hop IDX E " << nextHopIdx << std::endl; 
        } 
        else
        {
          nextHopIdx = FindNextHopMidTrafficUpstreamAwayBA(neighborhoodSpeed, centerBA, vehiclePos, stopDist);
          std::cout << "Next Hop IDX F " << nextHopIdx << std::endl; 
        }
      }
      else
      {
        // Determine if will reach BA before it expires, return -1 if not closeToBA 	
        float currentMDT = CalculateDistance(vehiclePos, centerBA)/neighborhoodSpeed;  
        //bool closeToBA = false;
        if ((Simulator::Now()/1e9 + Seconds(currentMDT)) <= Seconds(m_BroadcastTime))
        { 
          if (LOS>m_vcHighTraffic)
          {
            nextHopIdx = FindNextHopHighTrafficUpstreamToBA(centerBA, vehiclePos, stopDist);
            std::cout << "Next Hop IDX G " << nextHopIdx << std::endl; 
          } 
          else if (LOS<m_vcLowTraffic)
          {
            nextHopIdx = FindNextHopLowTrafficUpstreamToBA(neighborhoodSpeed, centerBA, vehiclePos, stopDist);
            std::cout << "Next Hop IDX H " << nextHopIdx << std::endl; 
          } 
          else 
          {
           nextHopIdx = FindNextHopMidTrafficUpstreamToBA(neighborhoodSpeed, centerBA, vehiclePos, stopDist);
           std::cout << "Next Hop IDX I " << nextHopIdx << std::endl; 
          }
        }
        else
        {
          std::cout << "Will not reach broadcast area" << std::endl;
          return Ipv4Address("102.102.102.102");
        }

      }

      if (nextHopIdx >= 0)
      {
        neighborInfo->Get1HopNeighborIP(nextHopIdx);
      }

      return Ipv4Address("102.102.102.102");
    }

int
RoutingProtocol::FindNextHopHighTrafficDownstream(Vector centerBA, Vector vehiclePos, float stopDist)
{
  //used in heavy traffic
  //finds next hop based on vehicle closest to broadcast area      
  // high traffic downstream: 
  // movement towards BA: consider neighbors ahead, closest to BA
  // movement away BA: wont receive -- handled by Rx callback
  std::cout << "Next hop based on high traffic, consider neighbors ahead, Min dist to BA" << std::endl;
  Ptr<VbpNeighbors> neighborInfo = m_neighborsListPointer->GetObject<VbpNeighbors>();
  uint16_t numNeighbors = neighborInfo->Get1HopNumNeighbors();
  float currentMin = std::numeric_limits<float>::max();
  int furthestIdx = -1;
  float neighborDist;
  Vector neighborPos;
  for(uint16_t idx = 0; idx < numNeighbors; idx++)
  {  
    if (neighborInfo->Get1HopDirection(idx) == 0) { // car is behind, then skip
        continue;
    }      
    neighborPos = Vector3D(neighborInfo->GetNeighborPositionX(idx), neighborInfo->GetNeighborPositionY(idx),0);
    if (CalculateDistance(neighborPos, vehiclePos) < stopDist) {
        std::cout << "\nToo close, They are  within(m): " << stopDist << std::endl;               
        continue; // if not going to move more than 3 second of driving, hold onto packet
    }
    if (CalculateDistance(neighborPos, vehiclePos) >= m_maxDistance*m_txCutoffPercentage) {
        std::cout << "\nToo far, may not make it to them. They are  within(m): " << m_maxDistance*m_txCutoffPercentage << std::endl;               
        continue; // if not going to move more than 3 second of driving, hold onto packet
    }
    neighborDist = CalculateDistance(neighborPos, centerBA);
    std::cout << "Current min dist is:"<< currentMin << ", best idx is "<< furthestIdx << std::endl;
    std::cout << "Neighbor id is: "<< neighborInfo->Get1HopNeighborIP(idx) << ", idx is: " << idx << ", Neighbor dist is: " << neighborDist << ", Neighbor dist is: "<< neighborDist<<std::endl;
    if (neighborDist < currentMin) {
        // if closer to broadcast area, change current id  
        currentMin = neighborDist;           
        furthestIdx = idx;                
    }
  }
return furthestIdx;  
}
      
int 
RoutingProtocol::FindNextHopMidTrafficDownstream(float neighborHoodSpeed, Vector centerBA, Vector vehiclePos, float stopDist)
{
  // used in mid traffic
  //finds next hop based on vehicle with Max of sqrt(speed^2 + distToNeighbor^2)
  // Mid traffic downstream: 
  // movement towards BA: consider neighbors ahead, Max of sqrt(speed^2 + distToNeighbor^2)
  // movement away BA: wont receive  -- handled by Rx callback
  std::cout << "Next hop based on mid traffic, consider neighbors ahead, Max of sqrt(speed^2 + distToNeighbor^2)" << std::endl;
  Ptr<VbpNeighbors> neighborInfo = m_neighborsListPointer->GetObject<VbpNeighbors>();
  uint16_t numNeighbors = neighborInfo->Get1HopNumNeighbors(); 
  float currentMax = -1; 
  int bestIdx = -1;    
  float distToNeighbor;
  float neighborMax;
  float neighborVel;
  Vector neighborPos;
  for(uint16_t idx = 0; idx < numNeighbors; idx++)
  {   
    if (neighborInfo->Get1HopDirection(idx) == 0)
    { // car is behind, then skip
      continue;
    }    
    neighborPos = Vector3D(neighborInfo->GetNeighborPositionX(idx), neighborInfo->GetNeighborPositionY(idx),0);
    distToNeighbor = CalculateDistance(neighborPos, vehiclePos);
    if (distToNeighbor < stopDist) 
    {
        std::cout << "\nToo close, They are  within(m): " << stopDist << std::endl;               
        continue; // if not going to move more than 3 second of driving, hold onto packet
    }
    if (distToNeighbor >= m_maxDistance*m_txCutoffPercentage) 
    {
      std::cout << "\nToo far, may not make it to them. They are  within(m): " << m_maxDistance*m_txCutoffPercentage << std::endl;               
      continue; // if not going to move more than 3 second of driving, hold onto packet
    }
    // use most recent measurement speed of individual node
    neighborVel = Vector3D(neighborInfo->GetNeighborSpeedX(idx), neighborInfo->GetNeighborSpeedY(idx),0).GetLength();
    neighborMax = std::sqrt(neighborVel*neighborVel + distToNeighbor*distToNeighbor);
    if (neighborMax > currentMax) 
    {
      currentMax = neighborMax;       
      bestIdx = idx;                
    }                
  }
  return bestIdx;  
}
      
int
RoutingProtocol::FindNextHopLowTrafficDownstream(float neighborHoodSpeed, Vector centerBA, Vector vehiclePos, float stopDist)
{
  // used in low traffic
  //finds next hop based on vehicle with minimum message delivery time 
  // Low traffic downstream:
  // movement towards BA : consider neighbors ahead, min MDT
  // movement away BA : wont receive -- handled by Rx callback
  std::cout << "Next hop based on low traffic, Min MDT" << std::endl;
  Ptr<VbpNeighbors> neighborInfo = m_neighborsListPointer->GetObject<VbpNeighbors>();
  uint16_t numNeighbors = neighborInfo->Get1HopNumNeighbors(); 
  float currentMDT = std::numeric_limits<float>::max();
  int bestIdx = -1;    
  float neighborDist;
  float neighborVel;
  float neighborMDT;
  Vector neighborPos;
  for(uint16_t idx = 0; idx < numNeighbors; idx++)
  {  
    if (neighborInfo->Get1HopDirection(idx) == 0) 
    { // car is behind, then skip
        continue;
    }      
    neighborPos = Vector3D(neighborInfo->GetNeighborPositionX(idx), neighborInfo->GetNeighborPositionY(idx),0);
    if (CalculateDistance(neighborPos, vehiclePos)  < stopDist) 
    {
      std::cout << "\nToo close, They are  within(m): " << stopDist << std::endl;               
      continue; // if not going to move more than 3 second of driving, hold onto packet
    }
    if (CalculateDistance(neighborPos, vehiclePos) >= m_maxDistance*m_txCutoffPercentage) 
    {
      std::cout << "\nToo far, may not make it to them. They are  within(m): " << m_maxDistance*m_txCutoffPercentage << std::endl;               
      continue; // if not going to move more than 3 second of driving, hold onto packet
    }
    neighborDist = CalculateDistance(neighborPos, centerBA);
    neighborVel = Vector3D(neighborInfo->GetNeighborSpeedX(idx), neighborInfo->GetNeighborSpeedY(idx),0).GetLength();
    neighborMDT = neighborDist/neighborVel;
    std::cout << "Current MDT is:"<< currentMDT << ", best idx is "<< bestIdx << std::endl;
    std::cout << "Neighbor id is: "<< neighborInfo->Get1HopNeighborIP(idx) << ", idx is: " << idx << ", Neighbor dist is: " << neighborDist << ", Neighbor MDT is: "<< neighborMDT<<std::endl;
    if (neighborMDT < currentMDT) {
        // if will drive faster to broadcast area, change current id  
        currentMDT = neighborMDT;           
        bestIdx = idx;                
    }
  }
  return bestIdx;   
}

int 
RoutingProtocol::FindNextHopHighTrafficUpstreamToBA(Vector centerBA, Vector vehiclePos, float stopDist)
{
  //used in heavy traffic
  //finds next hop based on vehicle furthest to broadcast area    
  // High traffic upstream: 
  // movement towards BA : consider neighbors behind, farthest from BA
  // movement away BA : consider neighbors behind, min Dist to BA
  std::cout << "Next hop based on high traffic, Max dist to BA" << std::endl;
  Ptr<VbpNeighbors> neighborInfo = m_neighborsListPointer->GetObject<VbpNeighbors>();
  uint16_t numNeighbors = neighborInfo->Get1HopNumNeighbors(); 
  float currentMax = -1; //if current node closer, hold onto packet
  int furthestIdx = -1;    
  float neighborDist;
  Vector neighborPos;
  for(uint16_t idx = 0; idx < numNeighbors; idx++)
  {  
    if (neighborInfo->Get1HopDirection(idx) == 1)
    { // car is ahead, then skip
      continue;
    }      
    neighborPos = Vector3D(neighborInfo->GetNeighborPositionX(idx), neighborInfo->GetNeighborPositionY(idx),0);
    if (CalculateDistance(neighborPos, vehiclePos) < stopDist)
    {
      std::cout << "\nToo close, They are  within(m): " << stopDist << std::endl;               
      continue; // if not going to move more than 3 second of driving, hold onto packet
    }
    if (CalculateDistance(neighborPos, vehiclePos) >= m_maxDistance*m_txCutoffPercentage) 
    {
      std::cout << "\nToo far, may not make it to them. They are  within(m): " << m_maxDistance*m_txCutoffPercentage << std::endl;               
      continue; // if not going to move more than 3 second of driving, hold onto packet
    }
    neighborDist = CalculateDistance(neighborPos, centerBA);
    std::cout << "Current dist is:"<< currentMax << ", best idx is "<< furthestIdx << std::endl;
    std::cout << "Neighbor id is: "<< neighborInfo->Get1HopNeighborIP(idx) << ", idx is: " << idx << ", Neighbor dist is: " << neighborDist << ", Neighbor dist is: "<< neighborDist<<std::endl;
    if (neighborDist > currentMax) 
    {
      // if closer to broadcast area, change current id  
      currentMax = neighborDist;           
      furthestIdx = idx;                
    }
  }
  return furthestIdx; 
}

int 
RoutingProtocol::FindNextHopHighTrafficUpstreamAwayBA(Vector centerBA, Vector vehiclePos, float stopDist)
{
  //used in heavy traffic
  //finds next hop based on vehicle min Dist to BA    
  // High traffic upstream: 
  // movement towards BA : consider neighbors behind, farthest from BA
  // movement away BA : consider neighbors behind, min Dist to BA
  std::cout << "Next hop based on high traffic, Min dist to BA" << std::endl;
  Ptr<VbpNeighbors> neighborInfo = m_neighborsListPointer->GetObject<VbpNeighbors>();
  uint16_t numNeighbors = neighborInfo->Get1HopNumNeighbors(); 
  float currentMin = std::numeric_limits<float>::max(); //if current node closer, hold onto packet
  int furthestIdx = -1;    
  float neighborDist;
  Vector neighborPos;
  for(uint16_t idx = 0; idx < numNeighbors; idx++) 
  {  
    if (neighborInfo->Get1HopDirection(idx) == 1) 
    { // car is ahead, then skip, only change from high traffic
      continue;
    }      
    neighborPos = Vector3D(neighborInfo->GetNeighborPositionX(idx), neighborInfo->GetNeighborPositionY(idx),0);
    if (CalculateDistance(neighborPos, vehiclePos) < stopDist) 
    {
      std::cout << "\nToo close, They are  within(m): " << stopDist << std::endl;               
      continue; // if not going to move more than 3 second of driving, hold onto packet
    }
    if (CalculateDistance(neighborPos, vehiclePos) >= m_maxDistance*m_txCutoffPercentage) 
    {
      std::cout << "\nToo far, may not make it to them. They are  within(m): " << m_maxDistance*m_txCutoffPercentage << std::endl;               
      continue; // if not going to move more than 3 second of driving, hold onto packet
    }
    neighborDist = CalculateDistance(neighborPos, centerBA);
    std::cout << "Current dist is:"<< currentMin << ", best idx is "<< furthestIdx << std::endl;
    std::cout << "Neighbor id is: "<< neighborInfo->Get1HopNeighborIP(idx) << ", idx is: " << idx << ", Neighbor dist is: " << neighborDist << ", Neighbor dist is: "<< neighborDist<<std::endl;
    if (neighborDist < currentMin) 
    {
      // if closer to broadcast area, change current id  
      currentMin = neighborDist;           
      furthestIdx = idx;                
    }
  }
  return furthestIdx;   
}

int 
RoutingProtocol::FindNextHopMidTrafficUpstreamToBA(float neighborhoodSpeed,Vector centerBA, Vector vehiclePos, float stopDist)
{
  // used in mid traffic
  //finds next hop based on vehicle with Max of sqrt(MDT^2 + distToNeighbor^2)
  // Mid traffic upstream: 
  // movement towards BA : consider neighbors behind, Max of sqrt(MDT^2 + distToNeighbor^2)
  // movement away BA : consider neighbors behind, min of sqrt(speed^2 + distToNeighborToBA^2)
  std::cout << "Next hop based on mid traffic, Max of sqrt(MDT^2 + distToNeighbor^2)" << std::endl;
  Ptr<VbpNeighbors> neighborInfo = m_neighborsListPointer->GetObject<VbpNeighbors>();
  uint16_t numNeighbors = neighborInfo->Get1HopNumNeighbors(); 
  float currentMax = -1; 
  int bestIdx = -1;    
  float neighborMDT;
  float neighborMax;
  float neighborVel;
  float neighborDist;
  float distToNeighbor;
  Vector neighborPos;
  for(uint16_t idx = 0; idx < numNeighbors; idx++) 
  {   
    if (neighborInfo->Get1HopDirection(idx) == 1)
    { // car is ahead, then skip, only change from high traffic
    continue;
    } 
    // wil include vehicles ahead and behind in case they have max MDT    
    neighborPos = Vector3D(neighborInfo->GetNeighborPositionX(idx), neighborInfo->GetNeighborPositionY(idx),0);
    distToNeighbor = CalculateDistance(neighborPos, vehiclePos);
    if (distToNeighbor < stopDist) 
    {
      std::cout << "\nToo close, They are  within(m): " << stopDist << std::endl;               
      continue; // if not going to move more than 3 second of driving, hold onto packet
    }
    if (distToNeighbor >= m_maxDistance*m_txCutoffPercentage) 
    {
      std::cout << "\nToo far, may not make it to them. They are  within(m): " << m_maxDistance*m_txCutoffPercentage << std::endl;               
      continue; // if not going to move more than 3 second of driving, hold onto packet
    }
    neighborDist = CalculateDistance(neighborPos, centerBA);
    neighborVel = Vector3D(neighborInfo->GetNeighborSpeedX(idx), neighborInfo->GetNeighborSpeedY(idx),0).GetLength();
    neighborMDT = neighborDist/neighborVel;
    neighborMax = std::sqrt(neighborMDT*neighborMDT + distToNeighbor*distToNeighbor); 
    std::cout << "Current Max is:" << currentMax << ", best idx is "<< bestIdx << std::endl;
    std::cout << "Neighbor id is: "<< neighborInfo->Get1HopNeighborIP(idx) << ", idx is: " << idx << ", dist To Neighbor is: " << distToNeighbor << ", Neighbor Max is: " << neighborMax << std::endl;
    if (neighborMax > currentMax) 
    { 
      currentMax = neighborMax;        
      bestIdx = idx;                
    }                
  }
  return bestIdx; 
}

int 
RoutingProtocol::FindNextHopMidTrafficUpstreamAwayBA(float neighborhoodSpeed,Vector centerBA, Vector vehiclePos, float stopDist)
{
  // used in mid traffic
  //finds next hop based on vehicle with min of sqrt(speed^2 + distNeighborToBA^2)
  // Mid traffic upstream: 
  // movement towards BA : consider neighbors behind, Max of sqrt(MDT^2 + distToNeighbor^2)
  // movement away BA : consider neighbors behind, min of sqrt(speed^2 + distNeighborToBA^2)
  std::cout << "Next hop based on mid traffic, min of sqrt(speed^2 + distToNeighborToBA^2)" << std::endl;
  Ptr<VbpNeighbors> neighborInfo = m_neighborsListPointer->GetObject<VbpNeighbors>();
  uint16_t numNeighbors = neighborInfo->Get1HopNumNeighbors(); 
  float currentMin = std::numeric_limits<float>::max();
  int bestIdx = -1;    
  float neighborDist;
  float neighborVel;
  float neighborMin;
  float distToNeighbor;
  Vector neighborPos;
  for(uint16_t idx = 0; idx < numNeighbors; idx++) 
  {   
    if (neighborInfo->Get1HopDirection(idx) == 1)
    { // car is ahead, then skip, only change from high traffic
    continue;
    } 
    // wil include vehicles ahead and behind in case they have max MDT    
    neighborPos = Vector3D(neighborInfo->GetNeighborPositionX(idx), neighborInfo->GetNeighborPositionY(idx),0);
    distToNeighbor = CalculateDistance(neighborPos, vehiclePos);
    if (distToNeighbor < stopDist) 
    {
      std::cout << "\nToo close, They are  within(m): " << stopDist << std::endl;               
      continue; // if not going to move more than 3 second of driving, hold onto packet
    }
    if (distToNeighbor >= m_maxDistance*m_txCutoffPercentage) 
    {
      std::cout << "\nToo far, may not make it to them. They are  within(m): " << m_maxDistance*m_txCutoffPercentage << std::endl;               
      continue;
    }
    neighborDist = CalculateDistance(neighborPos, centerBA);
    neighborVel = Vector3D(neighborInfo->GetNeighborSpeedX(idx), neighborInfo->GetNeighborSpeedY(idx),0).GetLength();
    neighborMin = std::sqrt(neighborVel*neighborVel + neighborDist*neighborDist); 
    std::cout << "Current Min is:" << currentMin << ", best idx is "<< bestIdx << std::endl;
    std::cout << "Neighbor id is: "<< neighborInfo->Get1HopNeighborIP(idx) << ", idx is: " << idx << ", dist To Neighbor is: " << distToNeighbor << ", Neighbor Min is: " << neighborMin << std::endl;
    if (neighborMin < currentMin) 
    { 
    //if (neighborInfo->Get1HopNumberOfNodesBehindOfNeighbor(idx) == 0) {
        // if node doesn't have neighbors ahead, move on
        // continue;
    //} 
      currentMin = neighborMin;        
      bestIdx = idx;                
    }                
  }
  return bestIdx;    
}

int 
RoutingProtocol::FindNextHopLowTrafficUpstreamToBA(float neighborhoodSpeed,Vector centerBA, Vector vehiclePos, float stopDist)
{
  // used in low traffic
  //finds next hop based on vehicle with maximum message delivery time
  // Low traffic upstream: 
  // movement towards BA : consider neighbors behind, max MDT
  // movement away BA : consider neighbors behind, min speed
  std::cout << "Next hop based on low traffic, Max MDT" << std::endl;
  Ptr<VbpNeighbors> neighborInfo = m_neighborsListPointer->GetObject<VbpNeighbors>();
  uint16_t numNeighbors = neighborInfo->Get1HopNumNeighbors(); 
  float currentMDT = -1;
  int bestIdx = -1;    
  float neighborDist;
  float neighborVel;
  float neighborMDT;
  Vector neighborPos;
  for(uint16_t idx = 0; idx < numNeighbors; idx++) 
  {  
    // will consider vehicles going behind, change from low traffic 
    if (neighborInfo->Get1HopDirection(idx) == 1) 
    { // car is ahead, then skip,only change from high traffic
      continue;
    }     
    neighborPos = Vector3D(neighborInfo->GetNeighborPositionX(idx), neighborInfo->GetNeighborPositionY(idx),0);
    if (CalculateDistance(neighborPos, vehiclePos)  < stopDist) 
    {
      std::cout << "\nToo close, They are  within(m): " << stopDist << std::endl;               
      continue; // if not going to move more than 3 second of driving, hold onto packet
    }
    if (CalculateDistance(neighborPos, vehiclePos) >= m_maxDistance*m_txCutoffPercentage) 
    {
      std::cout << "\nToo far, may not make it to them. They are  within(m): " << m_maxDistance*m_txCutoffPercentage << std::endl;               
      continue;
    }
    neighborDist = CalculateDistance(neighborPos, centerBA);
    neighborVel = Vector3D(neighborInfo->GetNeighborSpeedX(idx), neighborInfo->GetNeighborSpeedY(idx),0).GetLength();
    neighborMDT = neighborDist/neighborVel;
    std::cout << "Current MDT is:"<< currentMDT << ", best idx is "<< bestIdx << std::endl;
    std::cout << "Neighbor id is: "<< neighborInfo->Get1HopNeighborIP(idx) << ", idx is: " << idx;
    std::cout << ", Neighbor dist is: " << neighborDist << ", Neighbor MDT is: "<< neighborMDT<<std::endl;
    if (neighborMDT > currentMDT) 
    {
      currentMDT = neighborMDT;           
      bestIdx = idx;              
    }
  }
  return bestIdx;   
}

int 
RoutingProtocol::FindNextHopLowTrafficUpstreamAwayBA(float neighborhoodSpeed,Vector centerBA, Vector vehiclePos, float stopDist)
{
  // used in low traffic
  //finds next hop based on vehicle with minimum speed
  // Low traffic upstream: 
  // movement towards BA : consider neighbors behind, max MDT
  // movement away BA : consider neighbors behind, min speed
  std::cout << "Next hop based on low traffic, Min speed" << std::endl;
  Ptr<VbpNeighbors> neighborInfo = m_neighborsListPointer->GetObject<VbpNeighbors>();
  uint16_t numNeighbors = neighborInfo->Get1HopNumNeighbors(); 
  float currentSpeed = std::numeric_limits<float>::max(); 
  int bestIdx = -1;    
  float neighborVel;
  Vector neighborPos;
  for(uint16_t idx = 0; idx < numNeighbors; idx++) 
  {  
    // will consider vehicles going behind
    if (neighborInfo->Get1HopDirection(idx) == 1) 
    { // car is ahead, then skip
      continue;
    }     
    neighborPos = Vector3D(neighborInfo->GetNeighborPositionX(idx), neighborInfo->GetNeighborPositionY(idx),0);
    if (CalculateDistance(neighborPos, vehiclePos)  < stopDist) 
    {
      std::cout << "\nToo close, They are  within(m): " << stopDist << std::endl;               
      continue; // if not going to move more than 3 second of driving, hold onto packet
    }
    if (CalculateDistance(neighborPos, vehiclePos) >= m_maxDistance*m_txCutoffPercentage) 
    {
      std::cout << "\nToo far, may not make it to them. They are  within(m): " << m_maxDistance*m_txCutoffPercentage << std::endl;               
      continue; 
    }
    neighborVel = Vector3D(neighborInfo->GetNeighborSpeedX(idx), neighborInfo->GetNeighborSpeedY(idx),0).GetLength();
    std::cout << "Current speed is:"<< currentSpeed << ", best idx is "<< bestIdx << std::endl;
    std::cout << "Neighbor speed is: "<< neighborVel << std::endl;
    if (neighborVel <= currentSpeed) 
    {
      // if will drive slower, change current id  
      currentSpeed = neighborVel;           
      bestIdx = idx;              
    }
  }
  return bestIdx; 
}

Ptr<Ipv4Route>
RoutingProtocol::LoopbackRoute (const Ipv4Header & hdr, Ptr<NetDevice> oif) const
{
  //This is the same LoopbackRoute implemented in AODV
  NS_LOG_FUNCTION (this << hdr);
  NS_ASSERT (m_lo != 0);

  Ptr<Ipv4Route> routePtr; // = Create<Ipv4Route> ();
  RoutingTableEntry rt;
  rt.SetNextHop(Ipv4Address("127.0.0.1")); 
  rt.SetOutputDevice(m_lo);
  //rt.SetInterface(iface);
  routePtr = rt.GetRoute();
  routePtr->SetDestination (hdr.GetDestination ());
  //
  // Source address selection here is tricky.  The loopback route is
  // returned when AODV does not have a route; this causes the packet
  // to be looped back and handled (cached) in RouteInput() method
  // while a route is found. However, connection-oriented protocols
  // like TCP need to create an endpoint four-tuple (src, src port,
  // dst, dst port) and create a pseudo-header for checksumming.  So,
  // AODV needs to guess correctly what the eventual source address
  // will be.
  //
  // For single interface, single address nodes, this is not a problem.
  // When there are possibly multiple outgoing interfaces, the policy
  // implemented here is to pick the first available AODV interface.
  // If RouteOutput() caller specified an outgoing interface, that
  // further constrains the selection of source address
  //
  std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin ();
  if (oif)
    {
      // Iterate to find an address on the oif device
      for (j = m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
        {
          Ipv4Address addr = j->second.GetLocal ();
          int32_t interface = m_ipv4->GetInterfaceForAddress (addr);
          if (oif == m_ipv4->GetNetDevice (static_cast<uint32_t> (interface)))
            {
              routePtr->SetSource (addr);
              break;
            }
        }
    }
  else
    {
      routePtr->SetSource (j->second.GetLocal ());
    }
  NS_ASSERT_MSG (routePtr->GetSource () != Ipv4Address (), "Valid VBP source address not found");
  routePtr->SetGateway (Ipv4Address ("127.0.0.1"));
  routePtr->SetOutputDevice (m_lo);
  return routePtr;
}

void
RoutingProtocol::DeferredRouteOutput (Ptr<const Packet> p, const Ipv4Header & header,
                                      UnicastForwardCallback ucb, ErrorCallback ecb)
{
// This is the same DeferredRouteOutput as AODV
//Append all params DeferredRouteOutput receives packet, header, ucb, ecb
//Check how aodv removes packets from queue. If packet is removed, next hop has been found. Guessing ucb is called. Confirm.
  NS_LOG_FUNCTION (this << p << header);
  NS_ASSERT (p != 0 && p != Ptr<Packet> ());
  //check if queue full. If full, can not append. If not full, append.
  //What to do if queue size == 100?
  if (m_queuePointer->GetObject<VbpQueue>()->QueueFull())
  {
    std::cout << "Empty Queue: Queue Size Limit Reached" << std::endl;
    Ptr<const Packet> p = m_queuePointer->GetObject<VbpQueue>()->GetPacket();
    Ipv4Header header = m_queuePointer->GetObject<VbpQueue>()->GetHeader();
    Ipv4RoutingProtocol::ErrorCallback ecb = m_queuePointer->GetObject<VbpQueue>()->GetEcb();
    ecb (p, header, Socket::ERROR_NOTERROR);
  }
  m_queuePointer->GetObject<VbpQueue>()->AppendPacket(p); //append packet to queue
  m_queuePointer->GetObject<VbpQueue>()->AppendHeader(header); 
  // m_queuePointer->GetObject<VbpQueue>()->AppendUcb(ucb);
  // m_queuePointer->GetObject<VbpQueue>()->AppendEcb(ecb); 
  std::cout << "Queue Size: " << m_queuePointer->GetObject<VbpQueue>()->GetQueueSize() << std::endl;

  // QueueEntry newEntry (p, header, ucb, ecb);
  // bool result = m_queue.Enqueue (newEntry);
  // if (result)
  //   {
  //     NS_LOG_LOGIC ("Add packet " << p->GetUid () << " to queue. Protocol " << (uint16_t) header.GetProtocol ());
  //     RoutingTableEntry rt;
  //     bool result = m_routingTable.LookupRoute (header.GetDestination (), rt);
  //     if (!result || ((rt.GetFlag () != IN_SEARCH) && result))
  //       {
  //         NS_LOG_LOGIC ("Send new RREQ for outbound packet to " << header.GetDestination ());
  //         SendRequest (header.GetDestination ());
  //       }
  //   }
}

//work on the hybrid of case two and case three. This requires the use of FindNextHop. Need to figure out
//how to integrate with LCB. Put FindNextHop in RoutePacket. 
//return true = LCB. return false = NO LCB
// bool FindNextHop(&nextHopAhead, &nextHopBehind)
// {
//   //determine next hop
//   Ipv4Address nextHopAhead = Ipv4Address("102.102.102.102");
//   Ipv4Address nextHopBehind = Ipv4Address("102.102.102.102");
//   Ptr<VbpNeighbors> neighborsList = m_neighborsListPointer->GetObject<VbpNeighbors>();
//   Vector BA1 = Vector3D(routingHeader.GetPosition1X(), routingHeader.GetPosition1Y(),0); // for broadcast area point one
//   Vector BA2 = Vector3D(routingHeader.GetPosition2X(), routingHeader.GetPosition2Y(),0); // for broadcast area point two        
//   Vector centerBA = Vector3D((BA1.x+BA2.x)/2,(BA1.y+BA2.y)/2,0);
//   if (neighborsList->Get1HopDirectionByIP(routingHead.GetPrevHopIP()) == 0) //packet sent from behind. send downstream
//   {
//     nextHopAhead = FindNextHopDownstream(centerBA, movingToBA)
//     if (nextHopAhead == Ipv4Address("102.102.102.102"))
//     {
//       std::cout << "Add to queue" << std::endl;
//       m_queuePointer->GetObject<VbpQueue>()->AppendPacket(p); //append packet to queue
//       m_queuePointer->GetObject<VbpQueue>()->AppendHeader(header); 
//       m_queuePointer->GetObject<VbpQueue>()->AppendUcb(ucb);
//       m_queuePointer->GetObject<VbpQueue>()->AppendEcb(ecb); 
//       return false;
//     }
//   }
//   else
//   {
//     if (closeToBA)
//     {
//       nextHopBehind = FindNextHopUpstream(centerBA, movingToBA)
//     }
//     else
//     {
//       std::cout << "Will stop sending upstream" << std::endl;
//       return false;
//     }
//     if (nextHopBehind == Ipv4Address("102.102.102.102"))
//     {
//       std::cout << "added to queue" << std::endl;
//       m_queuePointer->GetObject<VbpQueue>()->AppendPacket(p); //append packet to queue
//       m_queuePointer->GetObject<VbpQueue>()->AppendHeader(header); 
//       m_queuePointer->GetObject<VbpQueue>()->AppendUcb(ucb);
//       m_queuePointer->GetObject<VbpQueue>()->AppendEcb(ecb); 
//       return false;
//     }
//   }
//   VbpRoutingHeader routingHeader;
//   routingHeader.SetData(m_dataPacketType, origin, m_broadcastArea[0], m_broadcastArea[1], m_broadcastArea[2], m_broadcastArea[3], m_BroadcastTime);
//   p->Addheader(routingHeader);
// }

  bool
  RoutingProtocol::FindNextHop(Ipv4Address* nextHopAheadPtr,Ipv4Address* nextHopBehindPtr, Vector centerBA, bool movingToBA, bool closeToBA, bool *enqueuePacketIndicator, Ipv4Address prevHopIP)
  {
    //determine next hop
    Ipv4Address nextHopAhead = Ipv4Address("102.102.102.102");
    Ipv4Address nextHopBehind = Ipv4Address("102.102.102.102");
    if (neighborsList->Get1HopDirectionByIP(prevHopIP) == 0) //packet sent from behind. send downstream
    {
      nextHopAhead = FindNextHopDownstream(centerBA, movingToBA);
      if (nextHopAhead == Ipv4Address("102.102.102.102"))
      {
        //std::cout << "Add to queue" << std::endl;
        // m_queuePointer->GetObject<VbpQueue>()->AppendPacket(p); //append packet to queue
        // m_queuePointer->GetObject<VbpQueue>()->AppendHeader(routingHeader); 
        // m_queuePointer->GetObject<VbpQueue>()->AppendUcb(ucb);
        // m_queuePointer->GetObject<VbpQueue>()->AppendEcb(ecb);
        *enqueuePacketIndicator = true; 
        return false;
      }
    }
    else //previous hop is from ahead
    {
      if (closeToBA)
      {
        nextHopBehind = FindNextHopUpstream(centerBA, movingToBA);
        if (nextHopBehind == Ipv4Address("102.102.102.102"))
          {
            // std::cout << "added to queue" << std::endl;
            // m_queuePointer->GetObject<VbpQueue>()->AppendPacket(p); //append packet to queue
            // m_queuePointer->GetObject<VbpQueue>()->AppendHeader(header); 
            // m_queuePointer->GetObject<VbpQueue>()->AppendUcb(ucb);
            // m_queuePointer->GetObject<VbpQueue>()->AppendEcb(ecb);
            *enqueuePacketIndicator = true;  
            return false;
          }
        *enqueuePacketIndicator = false; 
        return true;
      }
      else
      {
        //turn below line into NSLOG function
        std::cout << "Will stop sending upstream because vehicle won't reach BA before expiration" << std::endl;
        *enqueuePacketIndicator = false; 
        return false;
      }
    }
  }

bool
RoutingProtocol::RoutePacket(Ptr<const Packet> p, Ipv4Address dst, Ipv4Address src, bool *packetSentIndicator) //VbpRoutingHeader routingHeader)
{
  //if can't use constant packet, duplicate packet and use copy
  *packetSentIndicator = false;
  Vector vehiclePos = m_thisNode->GetObject<MobilityModel>()->GetPosition();
  Vector vehicleVel = m_thisNode->GetObject<MobilityModel>()->GetVelocity();
  Ptr<VbpNeighbors> neighborsList = m_neighborsListPointer->GetObject<VbpNeighbors>();
  //case 1: vehicle already in broadcast area
  VbpRoutingHeader routingHeader;
  p->PeekHeader(routingHeader);
  if ((routingHeader.GetPosition1X() <= vehiclePos.x) && (vehiclePos.x <= routingHeader.GetPosition2X()))
  {
    if ((routingHeader.GetPosition1Y() <= vehiclePos.y) && (vehiclePos.y <= routingHeader.GetPosition2Y()))
    {
      std::cout << "In broadcast area" << std::endl;
      p->RemoveHeader(routingHeader);
      return true; //true = lcb. local delivery of vbp packet without routingheader. before call lcb, remove routing header (might need to do this in routeinput, before lcb)
    }
  }
  //case 2: vehicle not in BA and may reach BA before expiration
  Vector BA1 = Vector3D(routingHeader.GetPosition1X(), routingHeader.GetPosition1Y(),0); // for broadcast area point one
  Vector BA2 = Vector3D(routingHeader.GetPosition2X(), routingHeader.GetPosition2Y(),0); // for broadcast area point two        
  Vector centerBA = Vector3D((BA1.x+BA2.x)/2,(BA1.y+BA2.y)/2,0);
  float neighborhoodSpeed = Vector3D(neighborsList->GetNeighborHoodSpeedMeanX(), neighborsList->GetNeighborHoodSpeedMeanY(),0).GetLength();
  float currentMDT = CalculateDistance(vehiclePos, centerBA)/neighborhoodSpeed;
  Vector vehicleToBA = centerBA - vehiclePos;
  bool movingToBA = (vehicleVel.x*vehicleToBA.x + vehicleVel.y*vehicleToBA.y) > 0; // true if moving towards BA
  bool closeToBA = false;

  if ((Simulator::Now()/1e9 + Seconds(currentMDT)) <= Seconds(m_BroadcastTime))
  {
    p->RemoveHeader(routingHeader);
    closeToBA = true; //will need to return closeToBA after forwarding. return true
  }
  
  //case 3: hybrid of case 2 (determine next hop) and case 3: vehicle not in BA and will not reach BA before expiration
  //determine next hop
  // Ipv4Address nextHopAhead = Ipv4Address("102.102.102.102");
  // Ipv4Address nextHopBehind = Ipv4Address("102.102.102.102");
  // //put below code in FindNextHop
  // //need to return closeToBA (do it in this function)
  // if (neighborsList->Get1HopDirectionByIP(routingHeader.GetPrevHopIP()) == 0)
  // {
  //   nextHopAhead = FindNextHopDownstream(centerBA, movingToBA)
  //   if (nextHopIPAhead == Ipv4Address("102.102.102.102"))
  //   {
  //     std::cout << "Append to queue" << std::endl;
  //     m_queuePointer->GetObject<VbpQueue>()->AppendPacket(p); //append packet to queue
  //     m_queuePointer->GetObject<VbpQueue>()->AppendHeader(header); 
  //     return;
  //   }
  // }
  // else
  // {
  //   if (closeToBA)
  //   {
  //     nextHopIPBehind = FindNextHopUpstream(centerBA, movingToBA)
  //   }
  //   else
  //   {
  //     std::cout << "Not sent upstream" << std::endl;
  //     return;
  //   }
  //   if (nextHopIPBehind == Ipv4Address("102.102.102.102"))
  //   {
  //     std::cout << "Append to queue" << std::endl;
  //     m_queuePointer->GetObject<VbpQueue>()->AppendPacket(p); //append packet to queue
  //     m_queuePointer->GetObject<VbpQueue>()->AppendHeader(header); 
  //     return;
  //   }
  // }
  bool enqueuePacketIndicator = false;
  Ipv4Address prevHopIP = routingHeader.GetPrevHopIP();
  Ipv4Address nextHopAhead;
  Ipv4Address nextHopBehind;
  if (FindNextHop(&nextHopAhead, &nextHopBehind, centerBA, movingToBA, closeToBA, &enqueuePacketIndicator, prevHopIP))
  {
    Ipv4InterfaceAddress iface = m_socketAddresses.begin()->second;
    Ptr<NetDevice> dev = m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (iface.GetLocal ()));
    RoutingTableEntry rt;
    rt.SetOutputDevice(dev);
    rt.SetInterface(iface);
    Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol>();
    Ipv4Address thisVehicleIP = iface.GetAddress();
    routingHeader.SetData(m_dataPacketType, thisVehicleIP, routingHeader.GetPosition1X(), routingHeader.GetPosition1Y(), routingHeader.GetPosition2X(), routingHeader.GetPosition2Y(), routingHeader.GetBroadcastingTime());
    std::cout << "Find Next Hop: " << std::endl;
    if(nextHopAhead != Ipv4Address("102.102.102.102"))
    {
      std::cout << "Next Hop Ahead Only" << std::endl;
      Ptr<Ipv4Route> routeDownstream;
      rt.SetNextHop(nextHopAhead);
      routeDownstream = rt.GetRoute();
      l3->Send(p, src, dst, PROT_NUMBER, routeDownstream);
      *packetSentIndicator = true;
    } 
    else
    {
      std::cout << "Next Hop Behind Only" << std::endl;
      Ptr<Ipv4Route> routeUpstream;
      rt.SetNextHop(nextHopBehind);
      routeUpstream = rt.GetRoute();
      l3->Send(p, src, dst, PROT_NUMBER, routeUpstream);
      *packetSentIndicator = true;
    }
  }
  if (enqueuePacketIndicator)
  {
    m_queuePointer->GetObject<VbpQueue>()->AppendPacket(p); //append packet to queue
    m_queuePointer->GetObject<VbpQueue>()->AppendHeader(routingHeader); 
    // m_queuePointer->GetObject<VbpQueue>()->AppendUcb(ucb);
    // m_queuePointer->GetObject<VbpQueue>()->AppendEcb(ecb);
  }
  return closeToBA; 

}


  } // namespace vbp
} // namespace ns3