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
  static TypeId GetTypeId (void);
    static const uint32_t VBP_PORT;
    RoutingProtocol ();
    virtual ~RoutingProtocol ();

  // Inherited from Ipv4RoutingProtocol
  Ptr<Ipv4Route> RouteOutput (Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr);
  bool RouteInput (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                  UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                   LocalDeliverCallback lcb, ErrorCallback ecb);
  virtual void SetIpv4 (Ptr<Ipv4> ipv4);
  virtual void PrintRoutingTable (Ptr<OutputStreamWrapper> stream, Time::Unit unit = Time::S) const;
  virtual void NotifyAddAddress(uint32_t interface, Ipv4InterfaceAddress address);
  virtual void NotifyInterfaceDown(uint32_t interface);
  virtual void NotifyInterfaceUp(uint32_t interface);
  virtual void NotifyRemoveAddress(uint32_t interface, Ipv4InterfaceAddress address);
  void SendHello (void);
  void StartHelloTx(void);

  private:
    Time m_activeRouteTimeout;
 Ptr<Socket> FindSocketWithInterfaceAddress (Ipv4InterfaceAddress iface) const;
  void SendTo (Ptr<Socket> socket, Ptr<Packet> packet, Ipv4Address destination);
  // IP protocol
  Ptr<Ipv4> m_ipv4;
    /// Raw subnet directed broadcast socket per each IP interface, map socket -> iface address (IP + mask)
  std::map< Ptr<Socket>, Ipv4InterfaceAddress > m_socketSubnetBroadcastAddresses;
  // Loopback device used to defer RREQ until packet will be fully formed
  Ptr<NetDevice> m_lo;
  
  /// Raw unicast socket per each IP interface, map socket -> iface address (IP + mask)
  std::map< Ptr<Socket>, Ipv4InterfaceAddress > m_socketAddresses;

  void RecvVbp(Ptr<Socket> socket);

  /// Provides uniform random variables.
  Ptr<UniformRandomVariable> m_uniformRandomVariable;
};


} //namespace vbp
} //namespace ns3

#endif /* RoutingProtocol_H */