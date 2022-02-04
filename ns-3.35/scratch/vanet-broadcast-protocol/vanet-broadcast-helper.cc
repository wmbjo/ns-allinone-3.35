#include "ns3/node.h"
#include "vanet-broadcast-helper.h"
#include "vanet-broadcast-protocol.h"
#include "ns3/node-list.h"
#include "ns3/names.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-list-routing.h"

namespace ns3 {
  
VanetBroadcastHelper::VanetBroadcastHelper():
    Ipv4RoutingHelper ()
{
    //m_agentFactory.SetTypeId ("VanetBroadcastProtocol");
}

VanetBroadcastHelper* 
VanetBroadcastHelper::Copy (void) const 
{
  return new VanetBroadcastHelper (*this); 
}

Ptr<Ipv4RoutingProtocol> 
VanetBroadcastHelper::Create (Ptr<Node> node) const 
{
  Ptr<VanetBroadcastProtocol> agent = CreateObject<VanetBroadcastProtocol>();
  node->AggregateObject (agent);
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  agent->SetIpv4(ipv4); 
  return agent;
}

} //namespace ns3