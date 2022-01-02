/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/netanim-module.h"
#include "ns3/vector.h"
#include "ns3/v4ping-helper.h"
#include "MyRandomExpTrafficApp.h"
// ====================================================================
//
//                          Wifi 10.1.1.0
//                *
//                |
//               src
//
//
//                *               *               *
//                |               |               |
//               n0              n1              n2
//
//
//                *               *               *
//                |               |               |
//               n3              n4              n5
//
//
//                *               *               *
//                |               |               |
//               n6              n7              n8
//
//
//                                                *
//                                                |
//                                               dst
//
// Broadcast = True
//
//   Interference:
//
//     ./waf --run "scratch/ELEC673_example_8 --TimeToInterfere=0
//                  --PrimaryTxGain=0 --InterferingTxGain=0
//                  --Broadcast=true"
//
//   No interference:
//
//     ./waf --run "scratch/ELEC673_example_8 --TimeToInterfere=5
//                  --PrimaryTxGain=0 --InterferingTxGain=0
//                  --Broadcast=true"
//
//     ./waf --run "scratch/ELEC673_example_8 --TimeToInterfere=500000
//                  --PrimaryTxGain=0 --InterferingTxGain=0
//                  --Broadcast=true"
//
//     ./waf --run "scratch/ELEC673_example_8 --TimeToInterfere=0
//                  --PrimaryTxGain=5 --InterferingTxGain=0
//                  --Broadcast=true"
//
//
// Broadcast = False
//
//   Interference:
//
//      None
//
//   No interference:
//
//     ./waf --run "scratch/ELEC673_example_8 --TimeToInterfere=0
//                  --PrimaryTxGain=0 --InterferingTxGain=0
//                  --Broadcast=false"
//
//     ./waf --run "scratch/ELEC673_example_8 --TimeToInterfere=5
//                  --PrimaryTxGain=0 --InterferingTxGain=0
//                  --Broadcast=false"
//
//     ./waf --run "scratch/ELEC673_example_8 --TimeToInterfere=500000
//                  --PrimaryTxGain=0 --InterferingTxGain=0
//                  --Broadcast=false"
//
//     ./waf --run "scratch/ELEC673_example_8 --TimeToInterfere=0
//                  --PrimaryTxGain=5 --InterferingTxGain=0
//                  --Broadcast=false"
//
// ====================================================================

#define NET_ADDRESS "10.1.1.0"
#define NET_MASK_ADDRESS "255.255.255.0"
#define BROADCAST_ADDRESS "255.255.255.255"
#define UDP_PORT 8080
#define SOURCES_START_TIME 1 // seconds
#define SOURCE_START_TIME 1 // seconds
#define PK_INTERARRIVAL_TIME 1 // seconds
#define DISTANCE 5 // meters
#define FREQ 2.4e9 // Hz
#define SYS_LOSS 1 // unitless
#define MIN_LOSS 0 // dB
#define TX_POWER 42 // dBm
#define TX_GAIN 0 // dB
#define RX_GAIN 0 // dB
#define RX_SENSITIVITY -80 // dBm

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ELEC673_example_8");

// Callback executed by the sink socket when a data packet is received
void ReceivePacket(Ptr<Socket> socket)
{
  while(Ptr<Packet> packet = socket->Recv())
    NS_LOG_INFO("Application Layer:" << packet->GetSize() << " bytes received");
}

// Function called to schedule the transmission of the next datagram from the source sockets
void SendDatagram(Ptr<Socket> socket, uint32_t pktsize, uint32_t numpkts, double interarrival_time)
{
  if(numpkts > 0)
  {
    socket->Send(Create<Packet>(pktsize));
    Simulator::Schedule(Seconds(interarrival_time), &SendDatagram, socket, pktsize, numpkts - 1, interarrival_time);
  }
  else
    socket->Close();
}

// Callback executed the wifi physical layer of each node
void RssCallback (std::string context, Ptr<const Packet> packet, uint16_t channelFreqMhz, WifiTxVector wifiTxVector, MpduInfo mpduInfo, SignalNoiseDbm signalNoiseDbm)
{
  NS_LOG_INFO(context <<  ": Received Signal Strength (RSS) = " << signalNoiseDbm.signal << " dBm, Noise Power = " << signalNoiseDbm.noise << " dBm");
}

// Simulation
int main(int argc, char* argv[])
{
  // Allow user to set the transmission mode used for RTS, data, and non-unicast packet transmissions,
  // tx gain of the primary and interfering nodes, time from the start of primary transmission to the
  // start of the interfering transmission, packet size of primary and interfering nodes, and number
  // of packets transmitted by primary and interfering nodes.
  std::string PhyMode("DsssRate1Mbps");
  uint32_t NumNodes = 11;
  double PrimaryTxGain = TX_GAIN; // dBm
  double InterferingTxGain = TX_GAIN; // dBm
  double TimeToInterfere = 0; // uS
  double SimulationTime = 100; // seconds
  uint32_t PrimaryPktSize = 1024; // bytes
  uint32_t InterferingPktSize = 1024; // bytes
  std::string AppDataRate = {'8', '1', '9', '2'}; // bits per second
  uint32_t PacketSize = 512; // bytes
  uint32_t PRNGRunNumber = 1;
  uint32_t NumPackets = 10;
  bool Broadcast = true;
  bool showPings = true;
  CommandLine cmd;
  cmd.AddValue("PhyMode", "Transmission mode used for RTS, data, and non-unicast packet transmissions", PhyMode);
  cmd.AddValue("PrimaryTxGain", "Transmission gain of the primary node (dB)", PrimaryTxGain);
  cmd.AddValue("InterferingTxGain", "Transmission gain of the interfering node (dB)", InterferingTxGain);
  cmd.AddValue("TimeToInterfere", "Time from start of primary transmission to start of interfering transmission (uS)", TimeToInterfere);
  cmd.AddValue("PrimaryPktSize", "Packet size of the primary node (bytes)", PrimaryPktSize);
  cmd.AddValue("InterferingPktSize", "Packet size of the interfering node (bytes)", InterferingPktSize);
  cmd.AddValue("NumPackets", "Number of packets transmitted by each node, primary and interfering", NumPackets);
  cmd.AddValue("Broadcast", "Enable broadcast transmissions on primary and interfering nodes", Broadcast);
  cmd.AddValue ("showPings", "Show Ping6 reception", showPings);
  cmd.Parse(argc, argv);

  // Create nodes. Install Internet stack. Set location of the nodes and configure them as static (no movement).
  NodeContainer nodes;
  nodes.Create(11); //Create desired number of nodes
  InternetStackHelper stack;
  stack.Install(nodes);
  Ptr<ListPositionAllocator> PositionAllocator = CreateObject<ListPositionAllocator>(); //Goes in order from node 0 to node n
  //positive y points down (positive distance)
  PositionAllocator->Add(Vector3D(0, 0, 0)); //node src
  PositionAllocator->Add(Vector3D(0, DISTANCE, 0)); //node 0
  PositionAllocator->Add(Vector3D(DISTANCE, DISTANCE, 0)); //node 1
  PositionAllocator->Add(Vector3D(2*DISTANCE, DISTANCE, 0)); //node 2
  PositionAllocator->Add(Vector3D(0, 2*DISTANCE, 0)); //node 3
  PositionAllocator->Add(Vector3D(2*DISTANCE, DISTANCE, 0)); //node 4
  PositionAllocator->Add(Vector3D(2*DISTANCE, 2*DISTANCE, 0)); //node 5
  PositionAllocator->Add(Vector3D(0, 3*DISTANCE, 0)); //node 6
  PositionAllocator->Add(Vector3D(DISTANCE, 3*DISTANCE, 0)); //node 7
  PositionAllocator->Add(Vector3D(2*DISTANCE, 3*DISTANCE, 0)); //node 8
  PositionAllocator->Add(Vector3D(2*DISTANCE, 4*DISTANCE, 0)); //node dst
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator(PositionAllocator);
  mobility.Install(nodes);

  // Create channel of constant propagation speed and Friis loss. Enable Radiotap link
  // layer information. Configure the wifi MAC layer in Ad Hoc mode. Use the 802.11b
  // standard (DSSS PHY (Clause 15) and HR/DSSS PHY (Clause 18)). Configure the transmission
  // mode specified by the user with PhyMode. Connect devices to nodes and channel using
  // the MAC and physical-layer configuration previously identified.
  //TODO Add all of this in new script, customized for 5 meter distance between nodes
  YansWifiChannelHelper wifiChannelHelper;
  wifiChannelHelper.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannelHelper.AddPropagationLoss("ns3::FriisPropagationLossModel", "Frequency", DoubleValue(FREQ), "SystemLoss", DoubleValue(SYS_LOSS), "MinLoss", DoubleValue(MIN_LOSS));
  YansWifiPhyHelper wifiPhyHelper;
  wifiPhyHelper.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
  wifiPhyHelper.SetChannel(wifiChannelHelper.Create());
  WifiMacHelper wifiMacHelper;
  wifiMacHelper.SetType("ns3::AdhocWifiMac");
  WifiHelper wifiHelper;
  wifiHelper.SetStandard(WIFI_STANDARD_80211b);
  Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue(PhyMode));
  wifiHelper.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", StringValue(PhyMode), "ControlMode", StringValue(PhyMode));
  NetDeviceContainer devices = wifiHelper.Install(wifiPhyHelper, wifiMacHelper, nodes);
  // Configure frequency, transmission power, tx gain, rx gain, and sensitivity of the physical layer of n0, n1, n2
    //TODO Add all of this in new script, customized for 5 meter distance between nodes (parameters of physical layer)
  std::ostringstream configpath;
  for(int i = 0; i < 11; i++)
  {
    configpath << "/NodeList/" << nodes.Get(i)->GetId() << "/DeviceList/1/$ns3::WifiNetDevice/Phy/Frequency";
    Config::Set(configpath.str(), UintegerValue((uint32_t) FREQ/1e6));
    configpath.str(""); configpath.clear();
    configpath << "/NodeList/" << nodes.Get(i)->GetId() << "/DeviceList/1/$ns3::WifiNetDevice/Phy/TxPowerEnd";
    Config::Set(configpath.str(), DoubleValue(TX_POWER));
    configpath.str(""); configpath.clear();
    configpath << "/NodeList/" << nodes.Get(i)->GetId() << "/DeviceList/1/$ns3::WifiNetDevice/Phy/TxPowerStart";
    Config::Set(configpath.str(), DoubleValue(TX_POWER));
    configpath.str(""); configpath.clear();
    configpath << "/NodeList/" << nodes.Get(i)->GetId() << "/DeviceList/1/$ns3::WifiNetDevice/Phy/TxGain";
    Config::Set(configpath.str(), DoubleValue(TX_GAIN)); //default value
    configpath.str(""); configpath.clear();
    configpath << "/NodeList/" << nodes.Get(i)->GetId() << "/DeviceList/1/$ns3::WifiNetDevice/Phy/RxGain";
    Config::Set(configpath.str(), DoubleValue(RX_GAIN));
    configpath.str(""); configpath.clear();
    configpath << "/NodeList/" << nodes.Get(i)->GetId() << "/DeviceList/1/$ns3::WifiNetDevice/Phy/RxSensitivity";
    Config::Set(configpath.str(), DoubleValue(RX_SENSITIVITY));
    configpath.str(""); configpath.clear();
  }


  // Set IP addresses on wifi devices
  Ipv4AddressHelper address;
  address.SetBase(NET_ADDRESS, NET_MASK_ADDRESS);
  Ipv4InterfaceContainer interfaces = address.Assign(devices);

  // WRITE YOUR CODE HERE:
  // Create and bind the socket on the destination node. Set the receive
  // callback that prints the number of data bytes received in every packet.
  Ptr<Socket> udpSinkSocket = Socket::CreateSocket(nodes.Get(0), UdpSocketFactory::GetTypeId());
  udpSinkSocket->Bind(InetSocketAddress(Ipv4Address::GetAny(), UDP_PORT));
  udpSinkSocket->SetRecvCallback(MakeCallback(&ReceivePacket));

  // WRITE YOUR CODE HERE:
  // Create and install the source application over UDP on the last node that
  // generates random traffic directed to the sink (n0)
  Address udpSinkAddress(InetSocketAddress(interfaces.GetAddress(0), UDP_PORT));
  Ptr<Socket> udpSourceSocket = Socket::CreateSocket(nodes.Get(NumNodes - 1), UdpSocketFactory::GetTypeId());
  Ptr<MyRandomExpTrafficApp> udpSourceAppPtr = CreateObject<MyRandomExpTrafficApp>();
  udpSourceAppPtr->Setup(udpSourceSocket, udpSinkAddress, PacketSize, DataRate(AppDataRate), PRNGRunNumber);
  nodes.Get(NumNodes - 1)->AddApplication(udpSourceAppPtr);

  // Configure time resolution, simulation start and stop times.
  Time::SetResolution(Time::NS);
  udpSourceAppPtr->SetStartTime(Seconds(SOURCE_START_TIME));
  Simulator::Stop(Seconds(SimulationTime));

  // Enable network animation
  AnimationInterface anim("ELEC673_final_project.xml");
  anim.EnablePacketMetadata(true);

  // Execute simulation
  Simulator::Run();
  Simulator::Destroy();

  // Success
  return 0;
}








// /* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
// /*
//  * This program is free software; you can redistribute it and/or modify
//  * it under the terms of the GNU General Public License version 2 as
//  * published by the Free Software Foundation;
//  *
//  * This program is distributed in the hope that it will be useful,
//  * but WITHOUT ANY WARRANTY; without even the implied warranty of
//  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  * GNU General Public License for more details.
//  *
//  * You should have received a copy of the GNU General Public License
//  * along with this program; if not, write to the Free Software
//  * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//  */

// #include <iostream>
// #include <cmath>
// #include "ns3/core-module.h"
// #include "ns3/network-module.h"
// #include "ns3/aodv-module.h"
// #include "ns3/mobility-module.h"
// #include "ns3/internet-module.h"
// #include "ns3/yans-wifi-helper.h"
// #include "ns3/ssid.h"
// #include "ns3/netanim-module.h"
// #include "ns3/vector.h"
// #include "MyRandomExpTrafficApp.h"


// // ==========================================================================
// //
// // ELEC 673 Final Project:
// //
// // You need to complete the ns3 script below where it says WRITE YOUR CODE
// // HERE. Several lines of code can be written there. On the other hand, code
// // that is already in the script cannot be modified. The script below was
// // completed on the basis of:
// //
// // - The ns3 script aodv.cc of the ns3 available at
// //   https://www.nsnam.org/docs/release/3.31/doxygen/aodv_8cc_source.html
// // - ELEC673_example_6.cc
// // - ELEC673_example_7.cc
// //
// // You can take a look at the scripts listed above to complete the final
// // project.
// //
// // ==========================================================================


// // ==========================================================================
// //
// //                   Wifi 10.1.1.0
// //
// //       *           *           *           *
// //       |           |           |           |
// //       n0          n1          n2          n3
// //
// //
// //       *           *           *           *
// //       |           |           |           |
// //       n4          n5          n6          n7
// //
// //
// //       *           *           *           *
// //       |           |           |           |
// //       n8          n9          n10         n11
// //
// //
// //       *           *           *           *
// //       |           |           |           |
// //       n12         n13         n14         n15
// //
// // The user can add more wifi nodes beyond n15. The wifi network operates in
// // ad-hoc mode. The last node sends data randomly to n0 via UDP. All nodes
// // are mobile (2D Random Walk), and they are initially placed on a grid as
// // shown above. The 2D random walk is configured with two parameters: speed,
// // distance. The speed is uniformly distributed between a minimum and maximum
// // value which can be specified by the user. The distance is fixed at the
// // following value: initial distance between adjacent nodes divided by 20.
// // The initial distance between adjacent nodes is set by the user.
// //
// // Given that the nodes are mobile and the network is configured in ad-hoc
// // mode, packets are delivered from source to destination on a multi-hop
// // basis. The routing algorithm is Ad hoc On-Demand Distance Vector Routing,
// // which is a widely known algorithm to route data in ad-hoc networks. The
// // purpose of the final project is to evaluate the ability of this routing
// // algorithm to deliver packets as the nodes move faster and the number
// // of nodes increases.
// //
// // The following results are to be collected:
// //
// // - Network animation in ELEC673_final_project.xml
// //
// // - Printout for every packet received at the application layer of the last
// //   that indicates the number of bytes received
// //
// // The previous results need to be analyzed for the following scenarios in
// // which the application data rate is always 8192 bps and packets carry 4096
// // bits (i.e., 512 bytes), so the packet rate is 2 packet/s. The scenarios
// // considered include the cases of 16 and 25 nodes. In each scenario the
// // nodes increase their speed in every simulation. The second result listed
// // above shows that as the speed increases, the number of packets
// // successfully delivered to the last node decreases.
// //
// // The following simulations need to be run:
// //
// //  - As the speed increases, less packets are delivered to the destination.
// //    There are 16 nodes in this case:
// //
// //       sudo ./waf --run "scratch/ELEC673_final_project --NumNodes=16
// //                             --MinSpeed=0 --MaxSpeed=0.1 --NodeDistance=30"
// //
// //       sudo ./waf --run "scratch/ELEC673_final_project --NumNodes=16
// //                             --MinSpeed=32 --MaxSpeed=64 --NodeDistance=30"
// //
// //       sudo ./waf --run "scratch/ELEC673_final_project --NumNodes=16
// //                            --MinSpeed=64 --MaxSpeed=128 --NodeDistance=30"
// //
// //  - As the speed increases, less packets are delivered to the destination.
// //    There are 25 nodes in this case, and the number of packets delivered
// //    to the destination decreases faster with the speed.
// //
// //       sudo ./waf --run "scratch/ELEC673_final_project --NumNodes=25
// //                             --MinSpeed=0 --MaxSpeed=0.1 --NodeDistance=30"
// //
// //       sudo ./waf --run "scratch/ELEC673_final_project --NumNodes=25
// //                             --MinSpeed=10 --MaxSpeed=20 --NodeDistance=30"
// //
// //       sudo ./waf --run "scratch/ELEC673_final_project --NumNodes=25
// //                             --MinSpeed=32 --MaxSpeed=64 --NodeDistance=30"
// //
// //       sudo ./waf --run "scratch/ELEC673_final_project --NumNodes=25
// //                            --MinSpeed=64 --MaxSpeed=128 --NodeDistance=30"
// //
// // ==========================================================================


// #define NET_ADDRESS "10.1.1.0"
// #define NET_MASK_ADDRESS "255.255.255.0"
// #define UDP_PORT 8080
// #define SOURCE_START_TIME 1 // seconds

// using namespace ns3;

// // Callback that connects to UDP socket of the destination node to print the
// // number of packets delivered to the application layer of the destination
// // node
// void ReceivePacket(Ptr<Socket> socket)
// {
//   while(Ptr<Packet> packet = socket->Recv())
//   {
//     std::cout << "Application Layer:" << packet->GetSize() << " bytes received" << std::endl;
//   }
// }

// // Simulation
// int main (int argc, char **argv)
// {
//   // Allow user to set the number of wifi nodes, distance between adjacent
//   // nodes, simulation time, applications' data rate, packet size, run number
//   // of the pseudo random number generator (PRNG),and minimum and maximum
//   // speed of the uniformly distributed node speed.
//   uint32_t NumNodes = 16;
//   double NodeDistance = 30; // meters
//   double SimulationTime = 10; // seconds
//   std::string AppDataRate = {'8', '1', '9', '2'}; // bits per second
//   uint32_t PacketSize = 512; // bytes
//   uint32_t PRNGRunNumber = 1;
//   double MinSpeed = 64; // meters per second
//   double MaxSpeed = 128; // meters per second
//   CommandLine cmd;
//   cmd.AddValue ("NumNodes", "Number of nodes", NumNodes);
//   cmd.AddValue ("NodeDistance", "Distance between adjacent nodes (meters)", NodeDistance);
//   cmd.AddValue ("SimulationTime", "Simulation time (seconds)", SimulationTime);
//   cmd.AddValue("AppDataRate", "Application's average data rate (bps)", AppDataRate);
//   cmd.AddValue("PacketSize", "Application's packet size (bytes)", PacketSize);
//   cmd.AddValue("PRNGRunNumber", "Run number of the pseudo random number generator (PRNG) that determines packet transmission times", PRNGRunNumber);
//   cmd.AddValue("MinSpeed", "The speed of nodes is uniformly distributed from MinSpeed to MaxSpeed (m/s)", MinSpeed);
//   cmd.AddValue("MaxSpeed", "The speed of nodes is uniformly distributed from MinSpeed to MaxSpeed (m/s)", MaxSpeed);
//   cmd.Parse (argc, argv);


//   // WRITE YOUR CODE HERE:
//   // Create nodes and keep them on a container. Names are added to the nodes.
//   // Install the Internet stack and set the routing protocol to AODV.
//   NodeContainer nodes;
//   nodes.Create(NumNodes);
//   InternetStackHelper stack;
//   AodvHelper aodv;
//   stack.SetRoutingHelper(aodv);
//   stack.Install(nodes);

//   // Ipv4AddressHelper address;
//   // address.SetBase(NET_ADDRESS, NET_MASK_ADDRESS);
  

//   // WRITE YOUR CODE HERE:
//   // Nodes are mobile (2D random walk). The initial location of the nodes is
//   // a grid. The area of movement is the square that encloses the grid. The
//   // side length of the square is specified by the user. The 2D random walk
//   // is configured with two parameters: speed, distance. The speed is
//   // uniformly distributed between a minimum and maximum value which can be
//   // specified by the user. The distance is fixed at the following value:
//   // initial distance between adjacent nodes divided by 20. The initial
//   // distance between adjacent nodes is set by the user.
//  //example: -MinSpeed=64 --MaxSpeed=128 --NodeDistance=30
//   int GridWidth = (int) std::ceil(std::sqrt((double) NumNodes));
//   MobilityHelper mobility;
//   mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel");
//   mobility.Install(nodes);
//   Ptr<GridPositionAllocator> MyPositionAllocator = CreateObject<GridPositionAllocator>();
//   MyPositionAllocator->SetAttribute("MinX", DoubleValue(0.0));
//   MyPositionAllocator->SetAttribute("MinY", DoubleValue(0.0));
//   MyPositionAllocator->SetAttribute("DeltaX", DoubleValue(NodeDistance));
//   MyPositionAllocator->SetAttribute("DeltaY", DoubleValue(NodeDistance));
//   MyPositionAllocator->SetAttribute("GridWidth", UintegerValue(GridWidth));
//   MyPositionAllocator->SetAttribute("LayoutType", StringValue("RowFirst"));
//   for(uint32_t i = 0; i < NumNodes; i++)
//   {
//     RngSeedManager::SetRun(PRNGRunNumber + i);
//     Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable>();
//     x->SetAttribute("Min",DoubleValue(MinSpeed));
//     x->SetAttribute("Max",DoubleValue(MaxSpeed));
//     nodes.Get(i)->GetObject<RandomWalk2dMobilityModel>()->SetAttribute("Bounds", RectangleValue(Rectangle(0, GridWidth*NodeDistance, 0, GridWidth*NodeDistance)));
//     nodes.Get(i)->GetObject<RandomWalk2dMobilityModel>()->SetPosition(MyPositionAllocator->GetNext());
//     nodes.Get(i)->GetObject<RandomWalk2dMobilityModel>()->SetAttribute("Speed", PointerValue(x));
//     nodes.Get(i)->GetObject<RandomWalk2dMobilityModel>()->SetAttribute("Distance", DoubleValue(NodeDistance/20));
//   }


//   // WRITE YOUR CODE HERE:
//   // Install the channel and wifi net devices on all nodes. The MAC layer is
//   // configured in ad-hoc mode. Its rate-control algorithm is
//   // ConstantRateWifiManager with the data-mode set to OfdmRate6Mbps, and the
//   // RtsCtsThreshold is set to 0.
//   YansWifiChannelHelper wifiChannelHelper = YansWifiChannelHelper::Default();
//   YansWifiPhyHelper wifiPhyHelper = YansWifiPhyHelper::Default();
//   wifiPhyHelper.SetChannel(wifiChannelHelper.Create());
//   WifiMacHelper wifiMacHelper;
//   wifiMacHelper.SetType("ns3::AdhocWifiMac");
//   WifiHelper wifiHelper;
//   wifiHelper.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", StringValue("OfdmRate6Mbps"), "RtsCtsThreshold", UintegerValue(0));
//   NetDeviceContainer devices = wifiHelper.Install(wifiPhyHelper, wifiMacHelper, nodes);

//   // WRITE YOUR CODE HERE:
//   // Assign IP addresses to wife network devices
//   Ipv4AddressHelper address;
//   address.SetBase(NET_ADDRESS, NET_MASK_ADDRESS);
//   Ipv4InterfaceContainer interfaces = address.Assign(devices);

//   // WRITE YOUR CODE HERE:
//   // Create and bind the socket on the destination node. Set the receive
//   // callback that prints the number of data bytes received in every packet.
//   Ptr<Socket> udpSinkSocket = Socket::CreateSocket(nodes.Get(0), UdpSocketFactory::GetTypeId());
//   udpSinkSocket->Bind(InetSocketAddress(Ipv4Address::GetAny(), UDP_PORT));
//   udpSinkSocket->SetRecvCallback(MakeCallback(&ReceivePacket));

//   // WRITE YOUR CODE HERE:
//   // Create and install the source application over UDP on the last node that
//   // generates random traffic directed to the sink (n0)
//   Address udpSinkAddress(InetSocketAddress(interfaces.GetAddress(0), UDP_PORT));
//   Ptr<Socket> udpSourceSocket = Socket::CreateSocket(nodes.Get(NumNodes - 1), UdpSocketFactory::GetTypeId());
//   Ptr<MyRandomExpTrafficApp> udpSourceAppPtr = CreateObject<MyRandomExpTrafficApp>();
//   udpSourceAppPtr->Setup(udpSourceSocket, udpSinkAddress, PacketSize, DataRate(AppDataRate), PRNGRunNumber);
//   nodes.Get(NumNodes - 1)->AddApplication(udpSourceAppPtr);

//   // Configure time resolution, simulation start and stop times.
//   Time::SetResolution(Time::NS);
//   udpSourceAppPtr->SetStartTime(Seconds(SOURCE_START_TIME));
//   Simulator::Stop(Seconds(SimulationTime));

//   // Enable network animation
//   AnimationInterface anim("ELEC673_final_project.xml");
//   anim.EnablePacketMetadata(true);

//   // Execute simulation
//   Simulator::Run();
//   Simulator::Destroy();

//   // Success
//   return 0;
// }
