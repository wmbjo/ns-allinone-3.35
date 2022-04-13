#ifndef VBPQUEUE_H
#define VBPQUEUE_H

#include <iostream>
#include <vector>
#include <algorithm>
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/simulator.h"
#include "vanet-broadcast-protocol.h"
#include "vbp-data-packet-header.h"

namespace ns3
{
    namespace vbp
    {
        class VbpQueue : public Object
        {
        public:
            VbpQueue();
            virtual ~VbpQueue();
            static TypeId GetTypeId (void);
            void AppendQueue(Ptr<Packet> p);
            bool QueueEmpty();
            void RemoveQueue(VbpRoutingHeader dataHeader);
            Ptr<Packet> GetPacketQueue();
            uint16_t GetQueueSize();
            //void HelloWorld();

        private:
            std::vector<Ptr<Packet>> m_packetQ; // to hold queue of packet
        };

    } // namespace vbp
}     // namespace ns3

#endif /* VBP_QUEUE_H */
