#ifndef RoutingProtocol_H
#define RoutingProtocol_H


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

namespace ns3{

namespace vbp{
class RoutingProtocol : public Ipv4RoutingProtocol
{
  public:
    static const uint32_t VBP_PORT;
    RoutingProtocol ();
    virtual ~RoutingProtocol ();

  // Inherited from Ipv4RoutingProtocol
  Ptr<Ipv4Route> RouteOutput (Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr);
  bool RouteInput (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                  UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                   LocalDeliverCallback lcb, ErrorCallback ecb);
  virtual void NotifyInterfaceUp (uint32_t interface);
  virtual void NotifyInterfaceDown (uint32_t interface);
  virtual void NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address);
  virtual void NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address);
  virtual void SetIpv4 (Ptr<Ipv4> ipv4);
  virtual void PrintRoutingTable (Ptr<OutputStreamWrapper> stream, Time::Unit unit = Time::S) const;

  virtual void SetL3HelloSocket();
  //virtual void BroadcastHelloPacket();
  // IP protocol
  Ptr<Ipv4> m_ipv4;
  // Loopback device used to defer RREQ until packet will be fully formed
  Ptr<NetDevice> m_lo;
  //Create socket object
  Ptr<Socket> m_L3HelloSocket;

  private:
  EventId m_sendEvent;
  void ScheduleHelloTx(void);
  void SendTo (Ptr<Socket> socket, Ptr<Packet> packet, Ipv4Address destination);
};

} //namespace vbp
} //namespace ns3

#endif /* RoutingProtocol_H */