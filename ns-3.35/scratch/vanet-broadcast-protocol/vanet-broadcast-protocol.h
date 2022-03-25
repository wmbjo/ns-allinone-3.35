#ifndef RoutingProtocol_H
#define RoutingProtocol_H

#include <vector>
#include "ns3/mobility-model.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/random-variable-stream.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/udp-header.h"
#include "ns3/packet.h"
#include "ns3/node.h"
#include "vbp-packet.h"
#include "vbp-neighbor.h"
#include "vbp-rtable.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/ipv4-route.h"

namespace ns3
{

  namespace vbp
  {
    class RoutingProtocol : public Ipv4RoutingProtocol
    {
    public:
      /**
       * \brief Get the type ID.
       * \return the object TypeId
       */
      static TypeId GetTypeId(void);
      static const uint32_t VBP_PORT;
      /// constructor
      RoutingProtocol();
      virtual ~RoutingProtocol();
      // Inherited from Ipv4RoutingProtocol
      Ptr<Ipv4Route> RouteOutput(Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr);
      bool RouteInput(Ptr<const Packet> p,
                      const Ipv4Header &header,
                      Ptr<const NetDevice> idev,
                      UnicastForwardCallback ucb,
                      MulticastForwardCallback mcb,
                      LocalDeliverCallback lcb,
                      ErrorCallback ecb);
      virtual void SetIpv4(Ptr<Ipv4> ipv4);
      virtual void PrintRoutingTable(Ptr<OutputStreamWrapper> stream, Time::Unit unit = Time::S) const;
      virtual void NotifyAddAddress(uint32_t interface, Ipv4InterfaceAddress address);
      virtual void NotifyInterfaceDown(uint32_t interface);
      virtual void NotifyInterfaceUp(uint32_t interface);
      virtual void NotifyRemoveAddress(uint32_t interface, Ipv4InterfaceAddress address);
      // Periodic Packets
      void SendHello(void);
      void StartHelloTx(void);

    private:
      /// Routing table
      RoutingTable m_routingTable;
      //Protocol parameters
      uint8_t m_helloPacketType; ///< Set packet type to hello 'h'
      Time m_activeRouteTimeout; ///< Period of time during which the route is considered to be valid.
      // Loopback device used to defer RREQ until packet will be fully formed
      Ptr<NetDevice> m_lo;
      /// Provides uniform random variables
      Ptr<UniformRandomVariable> m_uniformRandomVariable;
      /// IP protocol
      Ptr<Ipv4> m_ipv4;
      Ptr<Node> m_thisNode;
      Ptr<Object> m_neighborsListPointer = CreateObject<Object>();
      // Raw subnet directed broadcast socket per each IP interface, map socket -> iface address (IP + mask)
      std::map<Ptr<Socket>, Ipv4InterfaceAddress> m_socketSubnetBroadcastAddresses;
      /// Raw unicast socket per each IP interface, map socket -> iface address (IP + mask)
      std::map<Ptr<Socket>, Ipv4InterfaceAddress> m_socketAddresses;

      Ptr<Socket> FindSocketWithInterfaceAddress(Ipv4InterfaceAddress iface) const;
      /**
       * Send packet to neighbor
       * \param socket input socket
       * \param p the packet to route
       * \param destination is supposed to be IP address of my neighbor.
       */
      void SendTo(Ptr<Socket> socket, Ptr<Packet> packet, Ipv4Address destination);

      /**
       * Receive and process packets
       * \param socket input socket
       */
      void RecvVbp(Ptr<Socket> socket);
      /**
       * Receive and process periodic packets.
       * \param p the packet to route
       * \param receiver is supposed to be my interface
       * \param sender is supposed to be IP address of my neighbor.
       */
      void RecvHello(Ptr<Packet> p, Ipv4Address receiver, Ipv4Address sender);
    };

  } // namespace vbp
} // namespace ns3

#endif /* RoutingProtocol_H */