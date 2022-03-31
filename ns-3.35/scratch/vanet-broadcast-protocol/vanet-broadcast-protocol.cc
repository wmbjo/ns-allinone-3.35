#include "vanet-broadcast-protocol.h"
#include "vbp-neighbor.h"

namespace ns3
{
  NS_LOG_COMPONENT_DEFINE("VanetBroadcastProtocol");

  namespace vbp
  {
    NS_OBJECT_ENSURE_REGISTERED(RoutingProtocol);
    const uint32_t RoutingProtocol::VBP_PORT = 655;
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
        : m_routingTable(Time(5)),
          m_helloPacketType('h') // 104 is an 'h' in ascii

    {
      Ptr<vbpneighbors> m_neighborsListPointer2 = CreateObject<vbpneighbors>();
      m_neighborsListPointer->AggregateObject(m_neighborsListPointer2);
    }

    RoutingProtocol::~RoutingProtocol()
    {
    }

    Ptr<Ipv4Route>
    RoutingProtocol::RouteOutput(Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
    {
      NS_LOG_FUNCTION (this << header << (oif ? oif->GetIfIndex () : 0));
      if (m_socketAddresses.empty ())
        {
          sockerr = Socket::ERROR_NOROUTETOHOST;
          NS_LOG_LOGIC ("No vbp interfaces");
          Ptr<Ipv4Route> route;
          return route;
        }
      sockerr = Socket::ERROR_NOTERROR;  
      Ipv4Address nextHop = m_neighborsListPointer->GetObject<vbpneighbors>()->Get1HopNeighborIPAhead(0);  
      Ipv4InterfaceAddress iface = m_socketAddresses.begin()->second;
      Ptr<NetDevice> dev = m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (iface.GetLocal ()));

      RoutingTableEntry rt;

      rt.SetNextHop(nextHop);
      rt.SetOutputDevice(dev);
      rt.SetInterface(iface);
      Ptr<Ipv4Route> rtentry;
      return rt.GetRoute();


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
      Ipv4Address origin = header.GetSource();
      helloPacketHeader destinationHeader;
      p->PeekHeader(destinationHeader);

      // VBP is not a multicast routing protocol
      if (dst.IsMulticast())
      {
        std::cout << "Multicast Return False" << std::endl;
        return false;
      }

      // Unicast local delivery
      if (m_ipv4->IsDestinationAddress(dst, iif))
      {
        if (lcb.IsNull() == false)
        {
          NS_LOG_LOGIC ("Unicast local delivery to " << dst);
          lcb(p, header, iif);
        }
        else
        {
          NS_LOG_ERROR ("Unable to deliver packet locally due to null callback " << p->GetUid () << " from " << origin);
          ecb (p, header, Socket::ERROR_NOROUTETOHOST);
        }
        return true;
      }

      // Forwarding
      Ipv4Address nextHop = m_neighborsListPointer->GetObject<vbpneighbors>()->Get1HopNeighborIPAhead(0);  //SetGateway
      Ipv4InterfaceAddress iface = m_socketAddresses.begin()->second;
      Ptr<NetDevice> dev = m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (iface.GetLocal ()));
      //create routing table entry using these four parameters
      RoutingTableEntry rt;
      //look at vbp-rtable to set destination
      rt.SetNextHop(nextHop); //not needed, going to pass parameter
      rt.SetOutputDevice(dev);
      rt.SetInterface(iface);
      ucb(rt.GetRoute(),p,header);
      return true;
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
      socket->Bind(InetSocketAddress(iface.GetLocal(), VBP_PORT));
      socket->SetAllowBroadcast(true);
      socket->SetIpRecvTtl(true);
      m_socketAddresses.insert(std::make_pair(socket, iface));

      // create also a subnet broadcast socket
      socket = Socket::CreateSocket(GetObject<Node>(), UdpSocketFactory::GetTypeId());
      NS_ASSERT(socket != 0);
      socket->SetRecvCallback(MakeCallback(&RoutingProtocol::RecvVbp, this));
      socket->BindToNetDevice(l3->GetNetDevice(interface));
      socket->Bind(InetSocketAddress(iface.GetBroadcast(), VBP_PORT));
      socket->SetAllowBroadcast(true);
      socket->SetIpRecvTtl(true);
      m_socketSubnetBroadcastAddresses.insert(std::make_pair(socket, iface));

      NS_LOG_FUNCTION(Simulator::Now().GetSeconds() << " Seconds --- "
                << "NotifyInterfaceUp "
                << "--- " << m_ipv4->GetNInterfaces() << " Interfaces");

      m_thisNode = socket->GetNode();
      NS_LOG_FUNCTION("This Node: " << m_thisNode->GetObject<MobilityModel>()->GetPosition());
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
      helloPacketHeader destinationHeader;
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
      helloPacketHeader helloHeader;
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
      m_neighborsListPointer->GetObject<vbpneighbors>()->AddNode(sender,
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
        helloPacketHeader HelloHeader;

        // get info needed in packet from sockets
        Vector pos = (socket->GetNode())->GetObject<MobilityModel>()->GetPosition();
        Vector vel = (socket->GetNode())->GetObject<MobilityModel>()->GetVelocity();
        // set dummy values to header setData (pass hardcoded values)
        Vector furthestAhead = Vector3D(NAN, NAN, 0);
        int furthestIdxAhead = m_neighborsListPointer->GetObject<vbpneighbors>()->GetNeighborFurthestAheadByIndex(pos);
        if (furthestIdxAhead >= 0)
        {
          furthestAhead = Vector3D(m_neighborsListPointer->GetObject<vbpneighbors>()->GetNeighborPositionX(furthestIdxAhead), m_neighborsListPointer->GetObject<vbpneighbors>()->GetNeighborPositionY(furthestIdxAhead), 0);
        }
        Vector furthestBehind = Vector3D(NAN, NAN, 0);
        int furthestIdxBehind = m_neighborsListPointer->GetObject<vbpneighbors>()->GetNeighborFurthestBehindByIndex(pos);
        if (furthestIdxBehind >= 0)
        {
          furthestBehind = Vector3D(m_neighborsListPointer->GetObject<vbpneighbors>()->GetNeighborPositionX(furthestIdxBehind), m_neighborsListPointer->GetObject<vbpneighbors>()->GetNeighborPositionY(furthestIdxBehind), 0);
        }
        HelloHeader.SetData(m_helloPacketType,
                            pos.x,
                            pos.y,
                            vel.x,
                            vel.y,
                            m_neighborsListPointer->GetObject<vbpneighbors>()->Get1HopNumNeighborsAhead(),
                            m_neighborsListPointer->GetObject<vbpneighbors>()->Get1HopNumNeighborsBehind(),
                            furthestAhead.x,
                            furthestAhead.y,
                            furthestBehind.x,
                            furthestBehind.y,
                            m_neighborsListPointer->GetObject<vbpneighbors>()->GetAvgSpeedNeighborX(vel.x),
                            m_neighborsListPointer->GetObject<vbpneighbors>()->GetAvgSpeedNeighborY(vel.y));

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
      m_neighborsListPointer->GetObject<vbpneighbors>()->PrintNeighborState();
    }

    void
    RoutingProtocol::SendTo(Ptr<Socket> socket, Ptr<Packet> packet, Ipv4Address destination)
    {
      socket->SendTo(packet, 0, InetSocketAddress(destination, VBP_PORT));
    }

    void RoutingProtocol::StartHelloTx()
    {
      m_uniformRandomVariable = CreateObject<UniformRandomVariable>();
      Time jitter = Time(MilliSeconds(Period_HelloTx + m_uniformRandomVariable->GetInteger(0, Jitter_HelloTx)));
      Simulator::Schedule(jitter, &RoutingProtocol::SendHello, this);
    }

  } // namespace vbp
} // namespace ns3