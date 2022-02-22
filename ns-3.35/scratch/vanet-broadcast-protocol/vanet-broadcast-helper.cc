#include "vanet-broadcast-helper.h"
#include "vanet-broadcast-protocol.h"

namespace ns3 {
VanetBroadcastHelper::~VanetBroadcastHelper ()
{

}  
VanetBroadcastHelper::VanetBroadcastHelper():
    Ipv4RoutingHelper ()
{
    m_agentFactory.SetTypeId ("ns3::vbp::RoutingProtocol");
}

VanetBroadcastHelper* 
VanetBroadcastHelper::Copy (void) const 
{
  return new VanetBroadcastHelper (*this); 
}

Ptr<Ipv4RoutingProtocol> 
VanetBroadcastHelper::Create (Ptr<Node> node) const 
{
  Ptr<vbp::RoutingProtocol> agent = m_agentFactory.Create<vbp::RoutingProtocol> ();
  node->AggregateObject (agent);
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  agent->SetIpv4(ipv4);
  agent->StartHelloTx(); 
   std::cout << "---Start Hello Tx Helper--- "  << std::endl;
  return agent;


  // Ptr<vbp::RoutingProtocol> agent = m_agentFactory.Create<vbp::RoutingProtocol>();
  //   std::cout << "HELLO" << std::endl;
  // node->AggregateObject (agent);
  // Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  // agent->SetIpv4(ipv4); 
  // return agent;
}

void 
VanetBroadcastHelper::Set (std::string name, const AttributeValue &value)
{
  std::cout << "NOP" << std::endl;
  m_agentFactory.Set (name, value);
  std::cout << "QRS" << std::endl;
}


} //namespace ns3