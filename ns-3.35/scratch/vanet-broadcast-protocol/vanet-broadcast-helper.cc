// #include "ns3/node.h"
#include "vanet-broadcast-helper.h"
#include "vanet-broadcast-protocol.h"
#include "ns3/node-list.h"
#include "ns3/names.h"
// #include "ns3/ptr.h"
#include "ns3/ipv4-list-routing.h"

namespace ns3 {
VanetBroadcastHelper::~VanetBroadcastHelper ()
{

}  
VanetBroadcastHelper::VanetBroadcastHelper():
    Ipv4RoutingHelper ()
{
    //m_agentFactory.SetTypeId ("ns3::vbp::RoutingProtocol");
}

VanetBroadcastHelper* 
VanetBroadcastHelper::Copy (void) const 
{
  return new VanetBroadcastHelper (*this); 
}

Ptr<Ipv4RoutingProtocol> 
VanetBroadcastHelper::Create (Ptr<Node> node) const 
{
  Ptr<vbp::RoutingProtocol> agent = CreateObject<vbp::RoutingProtocol>();
  node->AggregateObject (agent);
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  agent->SetIpv4(ipv4);
  agent->ScheduleHelloTx(); 
  return agent;


  // Ptr<vbp::RoutingProtocol> agent = m_agentFactory.Create<vbp::RoutingProtocol>();
  //   std::cout << "HELLO" << std::endl;
  // node->AggregateObject (agent);
  // Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  // agent->SetIpv4(ipv4); 
  // return agent;
}

} //namespace ns3