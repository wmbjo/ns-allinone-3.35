#include "vanet-broadcast-protocol.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/random-variable-stream.h"
#include "ns3/inet-socket-address.h"
#include "ns3/trace-source-accessor.h"  
#include "ns3/udp-l4-protocol.h"
#include "ns3/udp-header.h"
#include "ns3/wifi-net-device.h"
#include "ns3/adhoc-wifi-mac.h"
#include "ns3/wifi-mac-queue-item.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include <algorithm>
#include <limits>
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/names.h"
#include "ns3/packet.h"
#include "ns3/node.h"
#include "ns3/simulator.h"
#include "ns3/ipv4-route.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "vbp-packet.h"


namespace ns3 {
NS_LOG_COMPONENT_DEFINE ("VanetBroadcastProtocol");

namespace vbp {
NS_OBJECT_ENSURE_REGISTERED (RoutingProtocol);
/// UDP Port for VBP control traffic
const uint32_t RoutingProtocol::VBP_PORT = 655;
uint64_t m_uniformRandomVariable;
const uint32_t Period_HelloTx = 95;
const uint32_t Jitter_HelloTx = 10;


/// Tag used by DSDV implementation
struct DeferredRouteOutputTag : public Tag
{
  /// Positive if output device is fixed in RouteOutput
  int32_t oif;

  /**
   * Constructor
   *
   * \param o outgoing interface (OIF)
   */
  DeferredRouteOutputTag (int32_t o = -1)
    : Tag (),
      oif (o)
  {
  }

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId
  GetTypeId ()
  {
    static TypeId tid = TypeId ("ns3::vbp::DeferredRouteOutputTag")
      .SetParent<Tag> ()
      .SetGroupName ("Vbp")
      .AddConstructor<DeferredRouteOutputTag> ()
    ;
    return tid;
  }

  TypeId
  GetInstanceTypeId () const
  {
    return GetTypeId ();
  }

  uint32_t
  GetSerializedSize () const
  {
    return sizeof(int32_t);
  }

  void
  Serialize (TagBuffer i) const
  {
    i.WriteU32 (oif);
  }

  void
  Deserialize (TagBuffer i)
  {
    oif = i.ReadU32 ();
  }

  void
  Print (std::ostream &os) const
  {
    os << "DeferredRouteOutputTag: output interface = " << oif;
  }
};

TypeId
RoutingProtocol::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::vbp::RoutingProtocol")
    .SetParent<Ipv4RoutingProtocol> ()
    .SetGroupName ("Vbp")
    .AddConstructor<RoutingProtocol> ();
  return tid;
}


RoutingProtocol::RoutingProtocol ()
{

}

RoutingProtocol::~RoutingProtocol ()
{
}

Ptr<Ipv4Route> 
RoutingProtocol::RouteOutput (Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
    {
        Ptr<Ipv4Route> dummyvar;
        return dummyvar;
    }
bool
RoutingProtocol::RouteInput (Ptr<const Packet> p, const Ipv4Header &header,
                             Ptr<const NetDevice> idev, UnicastForwardCallback ucb,
                             MulticastForwardCallback mcb, LocalDeliverCallback lcb, ErrorCallback ecb)
{
    std::cout << "Route INput" << std::endl;
  if (m_socketAddresses.empty ())
    {
      NS_LOG_LOGIC ("No aodv interfaces");
      return false;
    }

  int32_t iif = m_ipv4->GetInterfaceForDevice (idev);

  Ipv4Address dst = header.GetDestination ();
  //Ipv4Address origin = header.GetSource ();

  // // Deferred route request
  // if (idev == m_lo)
  //   {
  //     DeferredRouteOutputTag tag;
  //     if (p->PeekPacketTag (tag))
  //       {
  //         DeferredRouteOutput (p, header, ucb, ecb);
  //         return true;
  //       }
  //   }

  // // Duplicate of own packet
  // if (IsMyOwnAddress (origin))
  //   {
  //     return true;
  //   }

  // AODV is not a multicast routing protocol
  if (dst.IsMulticast ())
    {
      std::cout << "Multicast Return False" << std::endl;
      return false;
    }

  // Broadcast local delivery/forwarding
  // for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j =
  //        m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
  //   {
  //     Ipv4InterfaceAddress iface = j->second;
  //     if (m_ipv4->GetInterfaceForAddress (iface.GetLocal ()) == iif)
  //       {
  //         if (dst == iface.GetBroadcast () || dst.IsBroadcast ())
  //           {
  //             if (m_dpd.IsDuplicate (p, header))
  //               {
  //                 NS_LOG_DEBUG ("Duplicated packet " << p->GetUid () << " from " << origin << ". Drop.");
  //                 return true;
  //               }
  //             UpdateRouteLifeTime (origin, m_activeRouteTimeout);
  //             Ptr<Packet> packet = p->Copy ();
  //             if (lcb.IsNull () == false)
  //               {
  //                 NS_LOG_LOGIC ("Broadcast local delivery to " << iface.GetLocal ());
  //                 lcb (p, header, iif);
  //                 // Fall through to additional processing
  //               }
  //             else
  //               {
  //                 NS_LOG_ERROR ("Unable to deliver packet locally due to null callback " << p->GetUid () << " from " << origin);
  //                 ecb (p, header, Socket::ERROR_NOROUTETOHOST);
  //               }
  //             if (!m_enableBroadcast)
  //               {
  //                 return true;
  //               }
  //             if (header.GetProtocol () == UdpL4Protocol::PROT_NUMBER)
  //               {
  //                 UdpHeader udpHeader;
  //                 p->PeekHeader (udpHeader);
  //                 if (udpHeader.GetDestinationPort () == AODV_PORT)
  //                   {
  //                     // AODV packets sent in broadcast are already managed
  //                     return true;
  //                   }
  //               }
  //             if (header.GetTtl () > 1)
  //               {
  //                 NS_LOG_LOGIC ("Forward broadcast. TTL " << (uint16_t) header.GetTtl ());
  //                 RoutingTableEntry toBroadcast;
  //                 if (m_routingTable.LookupRoute (dst, toBroadcast))
  //                   {
  //                     Ptr<Ipv4Route> route = toBroadcast.GetRoute ();
  //                     ucb (route, packet, header);
  //                   }
  //                 else
  //                   {
  //                     NS_LOG_DEBUG ("No route to forward broadcast. Drop packet " << p->GetUid ());
  //                   }
  //               }
  //             else
  //               {
  //                 NS_LOG_DEBUG ("TTL exceeded. Drop packet " << p->GetUid ());
  //               }
  //             return true;
  //           }
  //       }
  //   }

  //Unicast local delivery
  if (m_ipv4->IsDestinationAddress (dst, iif))
    {
      std::cout << "Unicast" << std::endl;
      // std::cout << "AAA" << std::endl;
      // UpdateRouteLifeTime (origin, m_activeRouteTimeout);
      // std::cout << "BBB" << std::endl;
      // RoutingTableEntry toOrigin;
      // std::cout << "CCC" << std::endl;
      // if (m_routingTable.LookupValidRoute (origin, toOrigin))
      //   {
      //     UpdateRouteLifeTime (toOrigin.GetNextHop (), m_activeRouteTimeout);
      //     m_nb.Update (toOrigin.GetNextHop (), m_activeRouteTimeout);
      //   }
      if (lcb.IsNull () == false)
        {
          std::cout << "AAA" << std::endl;
          NS_LOG_LOGIC ("Unicast local delivery to " << dst);
          std::cout << "BBB" << std::endl;
          lcb (p, header, iif);
          std::cout << "CCC" << std::endl;
        }
      // else
      //   {
      //     NS_LOG_ERROR ("Unable to deliver packet locally due to null callback " << p->GetUid () << " from " << origin);
      //     ecb (p, header, Socket::ERROR_NOROUTETOHOST);
      //   }
      std::cout << "DDD" << std::endl;
      return true;
    }

  // //Check if input device supports IP forwarding
  // if (m_ipv4->IsForwarding (iif) == false)
  //   {
  //     NS_LOG_LOGIC ("Forwarding disabled for this interface");
  //     ecb (p, header, Socket::ERROR_NOROUTETOHOST);
  //     return true;
  //   }

  // //Forwarding
  // std::cout << "FOWARDING 123" << std::endl;
  // return Forwarding (p, header, ucb, ecb);
  // std::cout << "FOWARDING 321" << std::endl;
  return true;
}
void 
RoutingProtocol::NotifyInterfaceUp (uint32_t interface)
  {
    std::cout << "Notify Interface UP" << std::endl;
  NS_LOG_FUNCTION (this << m_ipv4->GetAddress (interface, 0).GetLocal ());
  Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol> ();
  if (l3->GetNAddresses (interface) > 1)
    {
      NS_LOG_WARN ("AODV does not work with more then one address per each interface.");
    }
  Ipv4InterfaceAddress iface = l3->GetAddress (interface, 0);
  if (iface.GetLocal () == Ipv4Address ("127.0.0.1"))
    {
      return;
    }

  // Create a socket to listen only on this interface
  Ptr<Socket> socket = Socket::CreateSocket (GetObject<Node> (),UdpSocketFactory::GetTypeId ());
  NS_ASSERT (socket != 0);
  socket->SetRecvCallback (MakeCallback (&RoutingProtocol::RecvVbp, this));
  socket->BindToNetDevice (l3->GetNetDevice (interface));
  socket->Bind (InetSocketAddress (iface.GetLocal (), VBP_PORT));
  socket->SetAllowBroadcast (true);
  socket->SetIpRecvTtl (true);
  m_socketAddresses.insert (std::make_pair (socket, iface));

  //create also a subnet broadcast socket
  socket = Socket::CreateSocket (GetObject<Node> (), UdpSocketFactory::GetTypeId ());
  NS_ASSERT (socket != 0);
  socket->SetRecvCallback (MakeCallback (&RoutingProtocol::RecvVbp, this));
  socket->BindToNetDevice (l3->GetNetDevice (interface));
  socket->Bind (InetSocketAddress (iface.GetBroadcast (), VBP_PORT));
  socket->SetAllowBroadcast (true);
  socket->SetIpRecvTtl (true);
  m_socketSubnetBroadcastAddresses.insert (std::make_pair (socket, iface));

    std::cout << Simulator::Now().GetSeconds() << " Seconds --- " << "NotifyInterfaceUp " << "--- "<< m_ipv4->GetNInterfaces() << " Interfaces" << std::endl;
  }
void 
RoutingProtocol::NotifyInterfaceDown (uint32_t interface)
  {
    std::cout << "Notify Interface Down " << "--- "  << std::endl;
  }
  
void 
RoutingProtocol::NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address)
  {
  Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol>();

  if (!l3->IsUp (interface))
    {
      return;
    }
    std::cout << Simulator::Now().GetSeconds() << " Seconds --- " <<  "NotifyAddAddress " << "--- "<< m_ipv4->GetNInterfaces() << " Interfaces" << std::endl;


  }

void 
RoutingProtocol::NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address)
  {
    std::cout << Simulator::Now().GetSeconds() << " Seconds --- " << "Notify Remove Address " << "--- "<< m_ipv4->GetNInterfaces() << " Interfaces" << std::endl;
  }

void 
RoutingProtocol::RecvVbp (Ptr<Socket> socket)
  {
    //write code to convert array of 4 bytes (ip address) to Ipv4Address 
    std::cout << "RecvVbp"  << std::endl;
    NS_LOG_FUNCTION (this << socket);
    Address sourceAddress;
    std::cout << "SOURCE ADDRESS ---" << sourceAddress << std::endl;
    Ptr<Packet> packet = socket->RecvFrom (sourceAddress);
    std::cout << "SOURCE ADDRESS After " << sourceAddress << std::endl;
    InetSocketAddress inetSourceAddr = InetSocketAddress::ConvertFrom (sourceAddress);
    Ipv4Address sender = inetSourceAddr.GetIpv4 ();
    Ipv4Address receiver;
    std::cout << "Sender " << sender << std::endl;

    if (m_socketAddresses.find (socket) != m_socketAddresses.end ())
        {
          receiver = m_socketAddresses[socket].GetLocal ();
        }
      else if (m_socketSubnetBroadcastAddresses.find (socket) != m_socketSubnetBroadcastAddresses.end ())
        {
          receiver = m_socketSubnetBroadcastAddresses[socket].GetLocal ();
        }
      else
        {
          NS_ASSERT_MSG (false, "Received a packet from an unknown socket");
        }
    std::cout << "receiver " << receiver << std::endl;
    std::cout << "---Received Transmission--- " << std::endl;
  }

void
RoutingProtocol::SetIpv4 (Ptr<Ipv4> ipv4)
{
  NS_ASSERT (ipv4 != 0);
  //NS_ASSERT (m_ipv4 == 0);

  m_ipv4 = ipv4; //m_ipv4 set here

  // Create lo route. It is asserted that the only one interface up for now is loopback
  NS_ASSERT (m_ipv4->GetNInterfaces () == 1 && m_ipv4->GetAddress (0, 0).GetLocal () == Ipv4Address ("127.0.0.1"));
  m_lo = m_ipv4->GetNetDevice (0);
  NS_ASSERT (m_lo != 0);
  std::cout << Simulator::Now().GetSeconds() << " Seconds --- " << "Set Ipv4 " << "--- "<< m_ipv4->GetNInterfaces() << " Interfaces" << std::endl;
  return;
}

void
RoutingProtocol::PrintRoutingTable (Ptr<OutputStreamWrapper> stream, Time::Unit unit) const
{
  std::cout << "---PRT--- " <<  std::endl;
}

// void
// RoutingProtocol::SendHello ()
// {
//   //In this version of SendHello I fill SetData with dummy values to see if I can print out getters. This works
//   for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
//     {

//       Ptr<Socket> socket = j->first;
//       Ipv4InterfaceAddress iface = j->second;
//       std::cout << "---Interface Info--- " << iface << std::endl;
//       Ptr<Packet> packet = Create<Packet> ();
//       //create header here
//       periodicPacketHeader HelloHeader;
//       //set dummy values to header setData (pass hardcoded values)
//       HelloHeader.SetData(0, 0, 0.0, 0.0, 0.0, 0.0, 0, 0, 0.0, 0.0, 0.0, 0.0 , 0.0, 0.0);
//       // add header to packet
//       packet->AddHeader (HelloHeader);
//       // print the content of my packet on the standard output.
//       packet->Print (std::cout);
//       std::cout << std::endl;
//       // remove the header from the packet:
//       periodicPacketHeader destinationHeader;
//       packet->RemoveHeader (destinationHeader);
//       std::cout << destinationHeader.GetSpeedX() << std::endl;
   

//       // Send to all-hosts broadcast if on /32 addr, subnet-directed otherwise
//       Ipv4Address destination;
//       if (iface.GetMask () == Ipv4Mask::GetOnes ())
//         {
//           destination = Ipv4Address ("255.255.255.255");
//           std::cout << "---Test 1--- " << destination << std::endl;
//         }
//       else
//         {
//           destination = iface.GetBroadcast ();
//           std::cout << "---Test 2--- " << destination << std::endl;
//         }
//       Time jitter = Time (MilliSeconds (Period_HelloTx + m_uniformRandomVariable->GetInteger (0, Jitter_HelloTx)));
//       Simulator::Schedule (jitter, &RoutingProtocol::SendHello, this);
//       SendTo(socket,packet,destination);
//     }
//   std::cout << Simulator::Now().GetSeconds() << " Seconds --- " << "Schedule Send Hello " << "--- "<< m_ipv4->GetNInterfaces() << " Interfaces" << std::endl;

// }


void
RoutingProtocol::SendHello ()
{
  //In this version of SendHello I fill SetData with real values. This is a work in progress
  for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
    {

      Ptr<Socket> socket = j->first;
      Ipv4InterfaceAddress iface = j->second;
      std::cout << "---Interface Info--- " << iface << std::endl;
      Ptr<Packet> packet = Create<Packet> ();
      //create header here
      periodicPacketHeader HelloHeader;

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
      //set dummy values to header setData (pass hardcoded values)
      HelloHeader.SetData(PERIODIC_PACKET_TYPE, (m_socketptr->GetNode())->GetId(), pos.x, pos.y, vel.x, vel.y
            , neighbor1Hop->Get1HopNumNeighborsAhead(), neighbor1Hop->Get1HopNumNeighborsBehind()
            , furthestAhead.x, furthestAhead.y, furthestBehind.x, furthestBehind.y, neighbor1Hop->GetAvgSpeedNeighborX(vel.x)
            , neighbor1Hop->GetAvgSpeedNeighborY(vel.y)); 
      // add header to packet
      packet->AddHeader (HelloHeader);
      // print the content of my packet on the standard output.
      packet->Print (std::cout);
      std::cout << std::endl;
      // remove the header from the packet:
      periodicPacketHeader destinationHeader;
      packet->RemoveHeader (destinationHeader);
      std::cout << destinationHeader.GetSpeedX() << std::endl;
   

      // Send to all-hosts broadcast if on /32 addr, subnet-directed otherwise
      Ipv4Address destination;
      if (iface.GetMask () == Ipv4Mask::GetOnes ())
        {
          destination = Ipv4Address ("255.255.255.255");
          std::cout << "---Test 1--- " << destination << std::endl;
        }
      else
        {
          destination = iface.GetBroadcast ();
          std::cout << "---Test 2--- " << destination << std::endl;
        }
      Time jitter = Time (MilliSeconds (Period_HelloTx + m_uniformRandomVariable->GetInteger (0, Jitter_HelloTx)));
      Simulator::Schedule (jitter, &RoutingProtocol::SendHello, this);
      SendTo(socket,packet,destination);
    }
  std::cout << Simulator::Now().GetSeconds() << " Seconds --- " << "Schedule Send Hello " << "--- "<< m_ipv4->GetNInterfaces() << " Interfaces" << std::endl;

}


void
RoutingProtocol::SendTo (Ptr<Socket> socket, Ptr<Packet> packet, Ipv4Address destination)
{
  socket->SendTo (packet, 0, InetSocketAddress (destination, VBP_PORT));
    std::cout << "---Send To--- " << std::endl;
}

void RoutingProtocol::StartHelloTx()
{
  m_uniformRandomVariable = CreateObject<UniformRandomVariable>();
  Time jitter = Time (MilliSeconds (Period_HelloTx + m_uniformRandomVariable->GetInteger (0, Jitter_HelloTx)));
  Simulator::Schedule(jitter, &RoutingProtocol::SendHello, this);
  std::cout << "---Start Hello TX--- " << std::endl;
}


}// namespace vbp
}// namespace ns3