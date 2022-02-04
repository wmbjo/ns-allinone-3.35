
#ifndef VANET_BROADCAST_HELPER_H
#define VANET_BROADCAST_HELPER_H

#include "ns3/object-factory.h"
#include "ns3/node.h"
#include "ns3/node-container.h"
#include "ns3/ipv4-routing-helper.h"

namespace ns3 {

class VanetBroadcastHelper : public Ipv4RoutingHelper
{

public:
  VanetBroadcastHelper();
  VanetBroadcastHelper* Copy (void) const;
  virtual Ptr<Ipv4RoutingProtocol> Create (Ptr<Node> node) const;


private:
 // ObjectFactory m_agentFactory;
  
};


} //namespace ns3
#endif /* VANET_BROADCAST_HELPER_H */
