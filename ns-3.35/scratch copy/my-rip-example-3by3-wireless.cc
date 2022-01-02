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
  double PrimaryTxGain = TX_GAIN; // dBm
  double InterferingTxGain = TX_GAIN; // dBm
  double TimeToInterfere = 0; // uS
  double SimulationTime = 10; // seconds
  uint32_t PrimaryPktSize = 1024; // bytes
  uint32_t InterferingPktSize = 1024; // bytes
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
  Ptr<Socket> udpSinkSocket = Socket::CreateSocket(nodes.Get(10), UdpSocketFactory::GetTypeId());
  udpSinkSocket->Bind(InetSocketAddress(Ipv4Address::GetAny(), UDP_PORT));
  udpSinkSocket->SetRecvCallback(MakeCallback(&ReceivePacket));



  NS_LOG_INFO ("Create Applications.");
  uint32_t packetSize = 1024;
  Time interPacketInterval = Seconds (1.0);
  V4PingHelper ping ("10.1.1.10");

  ping.SetAttribute ("Interval", TimeValue (interPacketInterval));
  ping.SetAttribute ("Size", UintegerValue (packetSize));
  if (showPings)
  {
      ping.SetAttribute ("Verbose", BooleanValue (true));
  }
  ApplicationContainer apps = ping.Install (nodes.Get(0)); //ping application is at ICMP layer
  apps.Start (Seconds (10.0));
  apps.Stop (Seconds (110.0));









  // Enable promiscuous pcap tracing on sink node (n0) and enable network animation
  wifiPhyHelper.EnablePcap("ELEC673_example_8.pcap", nodes.Get(10)->GetDevice(1), false, true);
  AnimationInterface anim("ELEC673_example_8.xml");
  anim.EnablePacketMetadata(true);

  // Configure time resolution and schedule the first packet transmission. The function called for packet
  // transmissions is recursive, and it will call itself as many times as there are data packets to be
  // transmitted. This number of data packets is specified by the user.
  Time::SetResolution(Time::NS);
  Simulator::Stop(Seconds(SimulationTime));
  //Simulator::ScheduleWithContext(nodes.Get(0)->GetId(), Seconds(SOURCES_START_TIME), &SendDatagram, primarySourceSocket, PrimaryPktSize, NumPackets, PK_INTERARRIVAL_TIME);
  //Simulator::ScheduleWithContext(nodes.Get(2)->GetId(), Seconds(SOURCES_START_TIME + TimeToInterfere/1000000), &SendDatagram, interfererSourceSocket, InterferingPktSize, NumPackets, PK_INTERARRIVAL_TIME);

  // Execute simulation
  Simulator::Run();
  Simulator::Destroy();

  // Success
  return 0;
}
