/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 IITP RAS
 *
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
 *
 * Based on
 *      NS-2 AODV model developed by the CMU/MONARCH group and optimized and
 *      tuned by Samir Das and Mahesh Marina, University of Cincinnati;
 *
 *      AODV-UU implementation by Erik Nordström of Uppsala University
 *      http://core.it.uu.se/core/index.php/AODV-UU
 *
 * Authors: Elena Buchatskaia <borovkovaes@iitp.ru>
 *          Pavel Boyko <boyko@iitp.ru>
 */

#include "aodv-routing-protocol.h"
#include "ns3/log.h"
//#include "ns3/boolean.h"
// #include "ns3/random-variable-stream.h"
// #include "ns3/inet-socket-address.h"
// #include "ns3/trace-source-accessor.h"
// #include "ns3/udp-socket-factory.h"
// #include "ns3/udp-l4-protocol.h"
// #include "ns3/udp-header.h"
// #include "ns3/wifi-net-device.h"
// #include "ns3/adhoc-wifi-mac.h"
// #include "ns3/wifi-mac-queue-item.h"
// #include "ns3/string.h"
// #include "ns3/pointer.h"
// #include <algorithm>
// #include <limits>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("AodvRoutingProtocol2");

namespace aodv {
NS_OBJECT_ENSURE_REGISTERED (RoutingProtocol);

/// UDP Port for AODV control traffic
//const uint32_t RoutingProtocol::AODV_PORT = 654;

// /**
// * \ingroup aodv
// * \brief Tag used by AODV implementation
// */
// class DeferredRouteOutputTag : public Tag
// {

// public:
//   /**
//    * \brief Constructor
//    * \param o the output interface
//    */
//   // DeferredRouteOutputTag (int32_t o = -1) : Tag (),
//   //                                           m_oif (o)
//   // {
//   // }

  // /**
  //  * \brief Get the type ID.
  //  * \return the object TypeId
  //  */
  // static TypeId GetTypeId ()
  // {
  //   static TypeId tid = TypeId ("ns3::aodv2::DeferredRouteOutputTag")
  //     .SetParent<Tag> ()
  //     .SetGroupName ("Aodv")
  //     //.AddConstructor<> ("")
  //   ;
  //   return tid;
  // }

//   // TypeId  GetInstanceTypeId () const
//   // {
//   //   return GetTypeId ();
//   // }

//   /**
//    * \brief Get the output interface
//    * \return the output interface
//    */
//   // int32_t GetInterface () const
//   // {
//   //   return m_oif;
//   // }

//   // /**
//   //  * \brief Set the output interface
//   //  * \param oif the output interface
//   //  */
//   // void SetInterface (int32_t oif)
//   // {
//   //   m_oif = oif;
//   // }

//   // uint32_t GetSerializedSize () const
//   // {
//   //   return sizeof(int32_t);
//   // }

//   // void  Serialize (TagBuffer i) const
//   // {
//   //   i.WriteU32 (m_oif);
//   // }

//   // void  Deserialize (TagBuffer i)
//   // {
//   //   m_oif = i.ReadU32 ();
//   // }

//   // void  Print (std::ostream &os) const
//   // {
//   //   os << "DeferredRouteOutputTag: output interface = " << m_oif;
//   // }

// private:
//   /// Positive if output device is fixed in RouteOutput
//   //int32_t m_oif;
// };

// NS_OBJECT_ENSURE_REGISTERED (DeferredRouteOutputTag);

// //minimize initializations and set ipv4 values. get access to ip layer. modify helper to call SetIpv4.
// //-----------------------------------------------------------------------------
RoutingProtocol::RoutingProtocol () = default; // '= default' to deal with my empty constructor
// // : 
// // {

// // }

TypeId
RoutingProtocol::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::aodv2::RoutingProtocol")
    .SetParent<Ipv4RoutingProtocol> ()
    .SetGroupName ("Aodv")
    .AddConstructor<RoutingProtocol> ()
  ;
  return tid;
 }

// RoutingProtocol::~RoutingProtocol ()
// {
// }

// void
// RoutingProtocol::DoDispose ()
// {
//   m_ipv4 = 0;
//   for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::iterator iter =
//          m_socketAddresses.begin (); iter != m_socketAddresses.end (); iter++)
//     {
//       iter->first->Close ();
//     }
//   m_socketAddresses.clear ();
//   for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::iterator iter =
//          m_socketSubnetBroadcastAddresses.begin (); iter != m_socketSubnetBroadcastAddresses.end (); iter++)
//     {
//       iter->first->Close ();
//     }
//   m_socketSubnetBroadcastAddresses.clear ();
//   Ipv4RoutingProtocol::DoDispose ();
// }

void
RoutingProtocol::TestMethod(Ptr<Ipv4> ipv4)
{
  // NS_ASSERT (ipv4 != 0);
  // NS_ASSERT (m_ipv4 == 0);

  m_ipv4 = ipv4; //m_ipv4 set here

  // Create lo route. It is asserted that the only one interface up for now is loopback
  NS_ASSERT (m_ipv4->GetNInterfaces () == 1 && m_ipv4->GetAddress (0, 0).GetLocal () == Ipv4Address ("127.0.0.1"));
  m_lo = m_ipv4->GetNetDevice (0);
  NS_ASSERT (m_lo != 0);
  std::cout << "Test Method" << std::endl;
  return;
}

void
RoutingProtocol::SetIpv4 (Ptr<Ipv4> ipv4)
{

  // NS_ASSERT (ipv4 != 0);
  // NS_ASSERT (m_ipv4 == 0);

  m_ipv4 = ipv4; //m_ipv4 set here

  // Create lo route. It is asserted that the only one interface up for now is loopback
  NS_ASSERT (m_ipv4->GetNInterfaces () == 1 && m_ipv4->GetAddress (0, 0).GetLocal () == Ipv4Address ("127.0.0.1"));
  m_lo = m_ipv4->GetNetDevice (0);
  NS_ASSERT (m_lo != 0);
  std::cout << "Set Ipv4" << std::endl;
  return;

}


// Ptr<Ipv4Route>
// RoutingProtocol::RouteOutput (Ptr<Packet> p, const Ipv4Header &header,
//                               Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
// {
//   Ptr<Ipv4Route> route;
//   return route;
// }



} //namespace aodv2
} //namespace ns3
