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
 * Authors: Pavel Boyko <boyko@iitp.ru>, written after OlsrHelper by Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include "my-aodv-helper.h"
#include "aodv-routing-protocol.h"
#include "ns3/node-list.h"
#include "ns3/names.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-list-routing.h"

namespace ns3
{

MyAodvHelper::MyAodvHelper() : 
  Ipv4RoutingHelper ()
{
  m_agentFactory.SetTypeId ("ns3::aodv2::RoutingProtocol");
}

MyAodvHelper* 
MyAodvHelper::Copy (void) const 
{
  return new MyAodvHelper (*this); 
}

Ptr<Ipv4RoutingProtocol> 
MyAodvHelper::Create (Ptr<Node> node) const //method executed when .install in aodv-3x3 script. do not modify for my own code
{
  std::cout << "Create" << std::endl;

  Ptr<aodv::RoutingProtocol> agent = m_agentFactory.Create<aodv::RoutingProtocol> (); //creates instance of routing protocol. protocol initialized. how does m_ipv4 given a value to start with? disable data traffic coming from src node. no packet transmissions. just try to get code to compile

  node->AggregateObject (agent); //aggregates instance to node

  //setipv4
  return agent;

}

void 
MyAodvHelper::Set (std::string name, const AttributeValue &value)
{
  std::cout << "Set" << std::endl;

  m_agentFactory.Set (name, value);

}

int64_t
MyAodvHelper::AssignStreams (NodeContainer c, int64_t stream)
{
  std::cout << "AssignStreams" << std::endl;
  int64_t currentStream = stream;
  Ptr<Node> node;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      node = (*i);
      Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> (); //node already has ipv4 protocol, this line retrives it. Do this for the Create method. Pass this value to setipv4 when I call it in the Create method
      NS_ASSERT_MSG (ipv4, "Ipv4 not installed on node");
      Ptr<Ipv4RoutingProtocol> proto = ipv4->GetRoutingProtocol ();
      NS_ASSERT_MSG (proto, "Ipv4 routing not installed on node");
      Ptr<aodv::RoutingProtocol> aodv = DynamicCast<aodv::RoutingProtocol> (proto); //routing protocol created here
      if (aodv)
        {
          currentStream += aodv->AssignStreams (currentStream);
          continue;
        }
      // Aodv may also be in a list
      Ptr<Ipv4ListRouting> list = DynamicCast<Ipv4ListRouting> (proto);
      if (list)
        {
          int16_t priority;
          Ptr<Ipv4RoutingProtocol> listProto;
          Ptr<aodv::RoutingProtocol> listAodv;
          for (uint32_t i = 0; i < list->GetNRoutingProtocols (); i++)
            {
              listProto = list->GetRoutingProtocol (i, priority);
              listAodv = DynamicCast<aodv::RoutingProtocol> (listProto);
              if (listAodv)
                {
                  currentStream += listAodv->AssignStreams (currentStream);
                  break;
                }
            }
        }
    }
  return (currentStream - stream);
}
}