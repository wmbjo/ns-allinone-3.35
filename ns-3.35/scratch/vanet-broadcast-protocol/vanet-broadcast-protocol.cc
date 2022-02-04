#include "vanet-broadcast-protocol.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/random-variable-stream.h"
#include "ns3/inet-socket-address.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
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


namespace ns3 {
NS_LOG_COMPONENT_DEFINE ("VanetBroadcastProtocol");

VanetBroadcastProtocol::VanetBroadcastProtocol ()
{

}

VanetBroadcastProtocol::~VanetBroadcastProtocol ()
{
}

Ptr<Ipv4Route> 
VanetBroadcastProtocol::RouteOutput (Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
    {
        Ptr<Ipv4Route> dummyvar;
        return dummyvar;
    }
bool 
VanetBroadcastProtocol::RouteInput (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                   UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                   LocalDeliverCallback lcb, ErrorCallback ecb)
                   {
                        
                    return false;
                   }
void 
VanetBroadcastProtocol::NotifyInterfaceUp (uint32_t interface)
  {

  }
void 
VanetBroadcastProtocol::NotifyInterfaceDown (uint32_t interface)
  {

  }
void 
VanetBroadcastProtocol::NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address)
  {

  }
void 
VanetBroadcastProtocol::NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address)
  {

  }

void
VanetBroadcastProtocol::SetIpv4 (Ptr<Ipv4> ipv4)
{
  NS_ASSERT (ipv4 != 0);
  NS_ASSERT (m_ipv4 == 0);

  m_ipv4 = ipv4; //m_ipv4 set here

  // Create lo route. It is asserted that the only one interface up for now is loopback
  NS_ASSERT (m_ipv4->GetNInterfaces () == 1 && m_ipv4->GetAddress (0, 0).GetLocal () == Ipv4Address ("127.0.0.1"));
  m_lo = m_ipv4->GetNetDevice (0);
  NS_ASSERT (m_lo != 0);
  std::cout << "Set Ipv4" << std::endl;
  return;
}

void
VanetBroadcastProtocol::PrintRoutingTable (Ptr<OutputStreamWrapper> stream, Time::Unit unit) const
{
    
}

}//namespace ns3