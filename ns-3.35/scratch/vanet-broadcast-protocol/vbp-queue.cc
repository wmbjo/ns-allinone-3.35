#include "vbp-queue.h"
#include <fstream>

namespace ns3 {
    NS_LOG_COMPONENT_DEFINE("VbpQueue");
namespace vbp {

VbpQueue::VbpQueue ()   
    {

    }

 VbpQueue::~VbpQueue () {
 }

 TypeId VbpQueue::GetTypeId (void) {
   static TypeId tid = TypeId ("ns3::vbp::VbpQueue")
     .SetParent<Object> ()
     .AddConstructor<VbpQueue> ();
   return tid;
 }

 void
VbpQueue::AppendQueue(Ptr<Packet> p)
{
    std::cout << "Append Queue" << std::endl;
    m_packetQ.push_back(p);
}
// void
// VbpQueue::HelloWorld()
// {
//     std::cout << "HELLO WORLD " << std::endl;
// }
Ptr<Packet>
VbpQueue::GetPacketQueue()
{
    std::cout << "Get Packet Queue" << std::endl;
    Ptr<Packet> p = m_packetQ.front();

    m_packetQ.erase(m_packetQ.begin());
    return p;
}

uint16_t
VbpQueue::GetQueueSize()
{
    return m_packetQ.size();
}

}
}
