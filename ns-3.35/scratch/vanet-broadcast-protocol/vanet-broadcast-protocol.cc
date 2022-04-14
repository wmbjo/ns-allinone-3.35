#include "vanet-broadcast-protocol.h"

namespace ns3
{
  NS_LOG_COMPONENT_DEFINE("VanetBroadcastProtocol");

  namespace vbp
  {
    NS_OBJECT_ENSURE_REGISTERED(RoutingProtocol);
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
        : m_queueRemovalPeriod(0.5),
          m_BroadcastTime(500),
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
    }

    RoutingProtocol::~RoutingProtocol()
    {
    }

    // Ptr<Ipv4Route>
    // RoutingProtocol::RouteOutput(Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
    // {
    //   Ptr<Ipv4Route> route;
    //   NS_LOG_FUNCTION (this << header << (oif ? oif->GetIfIndex () : 0));
    //   if (m_socketAddresses.empty ())
    //     {
    //       sockerr = Socket::ERROR_NOROUTETOHOST;
    //       NS_LOG_LOGIC ("No vbp interfaces");
    //       return route;
    //     }
    //   sockerr = Socket::ERROR_NOTERROR;

    //   VbpRoutingHeader routingHeader;
    //   RoutingTableEntry rt;

    //   Ipv4InterfaceAddress iface = m_socketAddresses.begin()->second;
    //   Ipv4Address origin = iface.GetAddress();
    //   Ptr<NetDevice> dev = m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (iface.GetLocal ()));
    //   routingHeader.SetData(m_dataPacketType, origin, m_broadcastArea[0], m_broadcastArea[1], m_broadcastArea[2], m_broadcastArea[3], m_BroadcastTime);
    //   p->AddHeader(routingHeader);
    //   std::cout << "Route Output: " << std::endl; 
    //   routingHeader.Print(std::cout);
    //   m_neighborsListPointer->GetObject<VbpNeighbors>()->AppendQueue(p); //append packet to queue
    //   std::cout << "Queue Size: " << m_neighborsListPointer->GetObject<VbpNeighbors>()->GetQueueSize() << std::endl;
    //   int numNextHops = m_neighborsListPointer->GetObject<VbpNeighbors>()->Get1HopNumNeighbors(); //get num 1 hop  neighbors
    //   if (numNextHops > 0) //if more than 0 neighbors
    //   {
    //       p = m_neighborsListPointer->GetObject<VbpNeighbors>()->GetPacketQueue();//remove packet from queue
    //       std::cout << "Queue Size 2: " << m_neighborsListPointer->GetObject<VbpNeighbors>()->GetQueueSize() << std::endl;
    //       Ipv4Address nextHop = m_neighborsListPointer->GetObject<VbpNeighbors>()->Get1HopNeighborIPAhead(0);//get next hop
    //       rt.SetNextHop(nextHop);//set route
    //       rt.SetOutputDevice(dev);
    //       rt.SetInterface(iface);
    //       std::cout << "Next Hop > 0 " << std::endl; 
    //       route = rt.GetRoute();
    //   }
    //   else //no neighbors
    //   {
    //     std::cout << "Socket Error " << std::endl; 
    //     sockerr = Socket::ERROR_NOROUTETOHOST;
    //   }

    //   return route;
    // }

    Ptr<Ipv4Route>
    RoutingProtocol::RouteOutput(Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
    {
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
      Ipv4Address dst = header.GetDestination ();
      Ipv4Address src = header.GetSource();
      std::cout << "RO DST: " << dst << std::endl;
      std::cout << "RO Src: " << src << std::endl;

      Ipv4InterfaceAddress iface = m_socketAddresses.begin()->second;
      Ipv4Address origin = iface.GetAddress();
      std::cout << "RO Origin: " << origin << std::endl;
      Ptr<NetDevice> dev = m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (iface.GetLocal ()));
      routingHeader.SetData(m_dataPacketType, origin, m_broadcastArea[0], m_broadcastArea[1], m_broadcastArea[2], m_broadcastArea[3], m_BroadcastTime);
      p->AddHeader(routingHeader);
      std::cout << "Route Output: " << std::endl; 
      routingHeader.Print(std::cout);
      m_queuePointer->GetObject<VbpQueue>()->AppendQueue(p); //append packet to queue
      std::cout << "Queue Size: " << m_queuePointer->GetObject<VbpQueue>()->GetQueueSize() << std::endl;
      //Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol>();
      //l3->SendWithHeader(p, header, route);
      
      std::cout << "Queue Size 2: " << m_queuePointer->GetObject<VbpQueue>()->GetQueueSize() << std::endl;
      //Ptr<Ipv4Address> nextHopPtr = CreateObject<Ipv4Address>();
      Ipv4Address nextHop;
      if (FindNextHop(&nextHop)) //find next hop
      {
         p = m_queuePointer->GetObject<VbpQueue>()->GetPacketQueue();//remove packet from queue
         std::cout << "Find Next Hop: " << nextHop << std::endl;
         rt.SetNextHop(nextHop); //set route
         rt.SetOutputDevice(dev);
         rt.SetInterface(iface);
        route = rt.GetRoute();
      } 
      else
      {
         std::cout << "Socket Error " << std::endl; 
         sockerr = Socket::ERROR_NOROUTETOHOST;
      }

      return route;
    }
    
    bool
    RoutingProtocol::RouteInput(Ptr<const Packet> p, const Ipv4Header &header,
                                Ptr<const NetDevice> idev, UnicastForwardCallback ucb,
                                MulticastForwardCallback mcb, LocalDeliverCallback lcb, ErrorCallback ecb)
    {
      std::cout << "Route Input: " << std::endl;
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
      Ipv4Address origin = header.GetSource();
      //Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol>();

      // VBP is not a multicast routing protocol
      if (dst.IsMulticast())
      {
        NS_LOG_LOGIC("Multicast Return False");
        return false;
      }

      // Unicast local delivery
      std::cout << "LCB " << dst << std::endl;
      if (m_ipv4->IsDestinationAddress(dst, iif))
      {
        if (lcb.IsNull() == false)
        {
          std::cout << "Local Delivery " << dst << std::endl;
          NS_LOG_LOGIC ("Unicast local delivery to " << dst);
          lcb(p, header, iif);
        }
        else
        {
          std::cout << "Error Delivery " << dst << std::endl;
          NS_LOG_ERROR ("Unable to deliver packet locally due to null callback " << p->GetUid () << " from " << origin);
          ecb (p, header, Socket::ERROR_NOROUTETOHOST);
        }
        return true;
      }
      // Forwarding

      VbpRoutingHeader routingHeader;
      Ipv4InterfaceAddress iface = m_socketAddresses.begin()->second;
      p->PeekHeader(routingHeader);
      routingHeader.SetPrevHopIP(iface.GetAddress()); 
      routingHeader.Print(std::cout);

      //change from original code: commented out nextHop and rt.SetNextHop(nextHop)
      //Ipv4Address nextHop = FindNextHop(); 
      //Ipv4Address nextHop = m_neighborsListPointer->GetObject<VbpNeighbors>()->Get1HopNeighborIPAhead(0);//get next hop

      Ipv4Address nextHop;
      if (FindNextHop(&nextHop)) //find next hop
      {
         std::cout << "RI Find Next Hop TRUE: " << nextHop << std::endl;
         RoutingTableEntry rt;
         Ptr<NetDevice> dev = m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (iface.GetLocal ()));
         rt.SetNextHop(nextHop); //set route
         rt.SetOutputDevice(dev);
         rt.SetInterface(iface);
         ucb(rt.GetRoute(),p,header);
         return true;
      } 
      std::cout << "RI Find Next Hop FALSE: " << nextHop << std::endl; //why is nextHop returning 102.102.102.102. RouteOutput transmits a 102.102.102.102 packet, why? Should not be Tx. Packet needs to be initialized.
      //Add a vector to AppendQueue to pair packet and header. Treat them in unison.
      //Step After: Schedule packet removal. Follow Roberto's code "CheckForQueueRemoval" and follow logic in my method, QueueRemoval. I need to remove packet and header from queue, and only send packet. To send packet, use Ipv4L3 Send().
      return false;

    }

    // bool
    // RoutingProtocol::RouteInput(Ptr<const Packet> p, const Ipv4Header &header,
    //                             Ptr<const NetDevice> idev, UnicastForwardCallback ucb,
    //                             MulticastForwardCallback mcb, LocalDeliverCallback lcb, ErrorCallback ecb)
    // {
    //   NS_LOG_FUNCTION (this << p->GetUid () << header.GetDestination () << idev->GetAddress ());
    //   if (m_socketAddresses.empty())
    //   {
    //     NS_LOG_LOGIC("No vbp interfaces");
    //     return false;
    //   }
    //   NS_ASSERT (m_ipv4 != 0);
    //   NS_ASSERT (p != 0);
    //   // Check if input device supports IP
    //   NS_ASSERT (m_ipv4->GetInterfaceForDevice (idev) >= 0);
    //   int32_t iif = m_ipv4->GetInterfaceForDevice(idev);

    //   Ipv4Address dst = header.GetDestination();
    //   Ipv4Address origin = header.GetSource();


    //   // VBP is not a multicast routing protocol
    //   if (dst.IsMulticast())
    //   {
    //     NS_LOG_LOGIC("Multicast Return False");
    //     return false;
    //   }

    //   // Unicast local delivery
    //   if (m_ipv4->IsDestinationAddress(dst, iif))
    //   {
    //     if (lcb.IsNull() == false)
    //     {
    //       NS_LOG_LOGIC ("Unicast local delivery to " << dst);
    //       lcb(p, header, iif);
    //     }
    //     else
    //     {
    //       NS_LOG_ERROR ("Unable to deliver packet locally due to null callback " << p->GetUid () << " from " << origin);
    //       ecb (p, header, Socket::ERROR_NOROUTETOHOST);
    //     }
    //     return true;
    //   }
    //   // Forwarding

    //   VbpRoutingHeader dataPacket;
    //   Ipv4InterfaceAddress iface = m_socketAddresses.begin()->second;
    //   p->PeekHeader(dataPacket);
    //   //m_neighborsListPointer->GetObject<VbpNeighbors>()->AppendQueue(p); //append packet to queue
    //   dataPacket.SetPrevHopIP(iface.GetAddress());
    //   std::cout << "Packet Route Input: " << std::endl; 
    //   dataPacket.Print(std::cout);

    //   // if (dataPacket.GetPacketType() == m_dataPacketType)
    //   // {
    //   //   std::cout << "Route input received a data packet header" << std::endl;
    //   // }


    //   RoutingTableEntry rt;
    //   Ptr<NetDevice> dev = m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (iface.GetLocal ()));
    //   //m_neighborsListPointer->GetObject<VbpNeighbors>()->AppendQueue(p); //append packet to queue
    //   std::cout << "Queue Size 3: " << m_neighborsListPointer->GetObject<VbpNeighbors>()->GetQueueSize() << std::endl;
    //   int numNextHops = m_neighborsListPointer->GetObject<VbpNeighbors>()->Get1HopNumNeighbors(); //get num 1 hop  neighbors
    //         std::cout << "MMMMM " << numNextHops << std::endl;
    //   if (numNextHops > 0) //if more than 0 neighbors
    //   {

    //       p = m_neighborsListPointer->GetObject<VbpNeighbors>()->GetPacketQueue();//remove packet from queue
    //                                     std::cout << "AAAAA " << numNextHops << std::endl;
    //       std::cout << "Queue Size 4: " << m_neighborsListPointer->GetObject<VbpNeighbors>()->GetQueueSize() << std::endl;

    //       Ipv4Address nextHop = m_neighborsListPointer->GetObject<VbpNeighbors>()->Get1HopNeighborIPAhead(0);//get next hop
    //       rt.SetNextHop(nextHop);//set route
    //       rt.SetOutputDevice(dev);
    //       rt.SetInterface(iface);

    //       ucb(rt.GetRoute(),p,header);
    //       return true;
    //   }
    //   else //no neighbors
    //   {
    //     return false;
    //     // std::cout << "LLLLL" << std::endl;
    //     // ecb (p, header, Socket::ERROR_NOROUTETOHOST);
    //     // return true;
    //   }

    // }



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
   RoutingProtocol::QueueRemoval()
   {
     uint16_t queueSize =  m_queuePointer->GetObject<VbpQueue>()->GetQueueSize();
     if (queueSize == 0)
    {
      return;
    }
    while(queueSize > 0)
    {
      queueSize--;

    }
  
   }

    void 
    RoutingProtocol::ScheduleQueueRemoval()
    {
        //Simulator::Schedule(Seconds(m_queueRemovalPeriod), &VbpQueue::GetPacketQueue, this);  
    }

    bool
    RoutingProtocol::FindNextHop(Ipv4Address* nextHopPtr)
    {
      if (m_neighborsListPointer->GetObject<VbpNeighbors>()->Get1HopNumNeighbors() == 0)
      {
        return false;
      }
      Ipv4Address nextHop = m_neighborsListPointer->GetObject<VbpNeighbors>()->Get1HopNeighborIPAhead(0);
      nextHopPtr->Set(nextHop.Get());
      return true;

    }

  } // namespace vbp
} // namespace ns3