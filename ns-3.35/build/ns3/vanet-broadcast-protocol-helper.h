/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef VANET_BROADCAST_PROTOCOL_HELPER_H
#define VANET_BROADCAST_PROTOCOL_HELPER_H

#include "/Users/william/Documents/ns-allinone-3.35/ns-3.35/contrib/vanet-broadcast-protocol/model/vanet-broadcast-protocol.h"
#include "ns3/object-factory.h"
#include "ns3/node.h"
#include "ns3/node-container.h"
#include "ns3/ipv4-routing-helper.h"

namespace ns3 {

class VanetBroadcastHelper : public Ipv4RoutingHelper
{

public:
  VanetBroadcastHelper();
  ~VanetBroadcastHelper();
  VanetBroadcastHelper* Copy (void) const;

  virtual Ptr<Ipv4RoutingProtocol> Create (Ptr<Node> node) const;
  void Set (std::string name, const AttributeValue &value);


private:
  ObjectFactory m_agentFactory;
  
};


} //namespace ns3
#endif /* VANET_BROADCAST_HELPER_H */

