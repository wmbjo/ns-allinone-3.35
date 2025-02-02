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

namespace vbp {

/// UDP Port for VBP control traffic
const uint32_t RoutingProtocol::VBP_PORT = 655;
uint64_t m_uniformRandomVariable;
const uint32_t Period_HelloTx = 95;
const uint32_t Jitter_HelloTx = 10;

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
RoutingProtocol::RouteInput (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                   UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                   LocalDeliverCallback lcb, ErrorCallback ecb)
                   {
                        
                   return false;
                   }
void 
RoutingProtocol::NotifyInterfaceUp (uint32_t interface)
  {
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
    Ptr<Socket> socket = Socket::CreateSocket (GetObject<Node> (), UdpSocketFactory::GetTypeId ());
    NS_ASSERT (socket != 0);
    std::cout << "Before" << std::endl;
    socket->SetRecvCallback (MakeCallback (&RoutingProtocol::RecvVbp,this));
    std::cout << "AFter" << std::endl;
    socket->BindToNetDevice (l3->GetNetDevice (interface));
    socket->Bind (InetSocketAddress (iface.GetLocal (), VBP_PORT));
    socket->SetAllowBroadcast (true);
    socket->SetIpRecvTtl (true);
    m_socketAddresses.insert (std::make_pair (socket, iface));

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

}

void
RoutingProtocol::SendHello ()
{
  for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
    {

      Ptr<Socket> socket = j->first;
      Ipv4InterfaceAddress iface = j->second;
      std::cout << "---Interface Info--- " << iface << std::endl;
      Ptr<Packet> packet = Create<Packet> ();
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