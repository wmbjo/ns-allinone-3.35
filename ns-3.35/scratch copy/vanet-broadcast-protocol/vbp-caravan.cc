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
#include "vanet-broadcast-helper.h"
#include "ns3/constant-velocity-mobility-model.h"

//#include <limits>
// ====================================================================
//
//                          Wifi 10.1.1.0
//                *               *
//                |               |
//               dst (1)         promiscuous (0)
//               10.1.1.2        10.1.1.1
//
//                *               *               *
//                |               |               |
//                n (2)           n (3)           n (4)
//                10.1.1.3        10.1.1.4        10.1.1.5
//
//                *               *               *
//                |               |               |
//                n (5)           n (6)           n (7)
//                10.1.1.6        10.1.1.7        10.1.1.8
//
//                *               *               *
//                |               |               |
//                n (8)           n (9)           n (10)
//                10.1.1.9        10.1.1.10       10.1.1.11
//
//                                                *
//                                                |
//                                               src (11)
//                                               10.1.1.12
//
//
// ====================================================================

#define NET_ADDRESS "10.1.1.0"
#define NET_MASK_ADDRESS "255.255.255.0" //first three bytes are for indicating network address. last byte indicates node in network address.
//#define BROADCAST_ADDRESS "255.255.255.255" Broadcast to every node on every network
#define UDP_PORT 8080
#define SOURCES_START_TIME 1 // seconds
#define SOURCE_START_TIME 1 // seconds
#define PK_INTERARRIVAL_TIME 1 // seconds
#define DISTANCE 5 // meters
#define SPEED 3
#define FREQ 2.4e9 // Hz
#define SYS_LOSS 1 // unitless
#define MIN_LOSS 0 // dB
#define TX_POWER 42 // dBm
#define TX_GAIN 0 // dB
#define RX_GAIN 0 // dB
#define RX_SENSITIVITY -80 // dBm

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("wireless-grid");

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
//   uint32_t PRNGRunNumber = 1;
//   double MinSpeed = 1; // meters per second
//   double MaxSpeed = 20; // meters per second
  std::string AppDataRate = {'8', '1', '9', '2'}; // bits per second
  //uint32_t PacketSize = 512; // bytes
  //uint32_t PRNGRunNumber = 1;
  uint32_t NumPackets = 10;
  bool Broadcast = true;
  bool showPings = true;
  bool printRoutes = true;

  CommandLine cmd;
  cmd.AddValue ("printRoutes", "Print routing table dumps.", printRoutes);
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
  nodes.Create(NumNodes); //Create desired number of nodes

  //vbp
  InternetStackHelper stack;
  VanetBroadcastHelper vbp;
  stack.SetRoutingHelper(vbp);
  stack.Install(nodes);




  //Ptr<ListPositionAllocator> PositionAllocator = CreateObject<ListPositionAllocator>(); //Goes in order from node 0 to node n
  //positive y points down (positive distance)
  //PositionAllocator->Add(Vector3D(12*DISTANCE, 0, 0)); //node 0(promiscuous)
//   PositionAllocator->Add(Vector3D(11*DISTANCE,0,0)); //node 1 dst
//   PositionAllocator->Add(Vector3D(10*DISTANCE,0,0)); //node 2
//   PositionAllocator->Add(Vector3D(9*DISTANCE,0,0)); //node 3
//   PositionAllocator->Add(Vector3D(8*DISTANCE,0,0)); //node 4
//   PositionAllocator->Add(Vector3D(7*DISTANCE,0,0)); //node 5
//   PositionAllocator->Add(Vector3D(6*DISTANCE,0,0)); //node 6
//   PositionAllocator->Add(Vector3D(5*DISTANCE,0,0)); //node 7
//   PositionAllocator->Add(Vector3D(4*DISTANCE,0,0)); //node 8
//   PositionAllocator->Add(Vector3D(3*DISTANCE,0,0)); //node 9
//   PositionAllocator->Add(Vector3D(2*DISTANCE,0,0)); //node 10
//   PositionAllocator->Add(Vector3D(DISTANCE,0,0)); //node 11 src



  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
  //mobility.SetPositionAllocator(PositionAllocator);
  mobility.Install(nodes);

//Set position of all nodes same. Then slightly vary velocity so they will be spread out as simulation time increases. Check for neighbor removal.
//Expect many neighbors in beginning. Expect dropped neighbors later on.

    for(int i = 0; i < int(NumNodes); i++)
  {
    nodes.Get(i)->GetObject<ConstantVelocityMobilityModel>()->SetPosition(Vector((float) DISTANCE, 0, 0));
    nodes.Get(i)->GetObject<ConstantVelocityMobilityModel>()->SetVelocity(Vector((float) 0.1*i, 0, 0)); //0.1*i because vehicles will lose neighbors about 50 seconds into simulation. SPEED is original variable here.
  }



  // Create channel of constant propagation speed and Friis loss. Enable Radiotap link
  // layer information. Configure the wifi MAC layer in Ad Hoc mode. Use the 802.11b
  // standard (DSSS PHY (Clause 15) and HR/DSSS PHY (Clause 18)). Configure the transmission
  // mode specified by the user with PhyMode. Connect devices to nodes and channel using
  // the MAC and physical-layer configuration previously identified.
  //TODO Add all of this in new script, customized for 5 meter distance between nodes
  YansWifiChannelHelper wifiChannelHelper = YansWifiChannelHelper::Default ();
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
  for(int i = 0; i < int(NumNodes); i++)
  {
    // //promiscuous node
    // if (i == 0){ //TODO Figure out good values for TX and RX.  TX very negative -inf. RX very big +inf. Sensitivity(what are weakest signals you can listen to) -inf.
    //   configpath << "/NodeList/" << nodes.Get(i)->GetId() << "/DeviceList/*/$ns3::WifiNetDevice/Phy/$ns3::WifiPhy/ChannelNumber";
    //   Config::Set(configpath.str(), UintegerValue(1));
    //   configpath << "/NodeList/" << nodes.Get(i)->GetId() << "/DeviceList/1/$ns3::WifiNetDevice/Phy/Frequency";
    //   Config::Set(configpath.str(), UintegerValue((uint32_t) FREQ/1e6));
    //   configpath.str(""); configpath.clear();
    //   configpath << "/NodeList/" << nodes.Get(i)->GetId() << "/DeviceList/1/$ns3::WifiNetDevice/Phy/TxPowerEnd";
    //   Config::Set(configpath.str(), DoubleValue(-1e3));
    //   configpath.str(""); configpath.clear();
    //   configpath << "/NodeList/" << nodes.Get(i)->GetId() << "/DeviceList/1/$ns3::WifiNetDevice/Phy/TxPowerStart";
    //   Config::Set(configpath.str(), DoubleValue(-1e3));
    //   configpath.str(""); configpath.clear();
    //   configpath << "/NodeList/" << nodes.Get(i)->GetId() << "/DeviceList/1/$ns3::WifiNetDevice/Phy/TxGain";
    //   Config::Set(configpath.str(), DoubleValue(-1e3));
    //   configpath.str(""); configpath.clear();
    //   configpath << "/NodeList/" << nodes.Get(i)->GetId() << "/DeviceList/1/$ns3::WifiNetDevice/Phy/RxGain";
    //   Config::Set(configpath.str(), DoubleValue(1e3));
    //   configpath.str(""); configpath.clear();
    //   configpath << "/NodeList/" << nodes.Get(i)->GetId() << "/DeviceList/1/$ns3::WifiNetDevice/Phy/RxSensitivity";
    //   Config::Set(configpath.str(), DoubleValue(-1e3));
    //   configpath.str(""); configpath.clear();
    // //   Ptr<ConstantVelocityMobilityModel> mob = nodes.Get(i)->GetObject<ConstantVelocityMobilityModel>();
    // //   mob->SetVelocity(Vector(10, 0, 0));
    // }
    // else {
      //Set src, dst, regular nodes to default values
      configpath << "/NodeList/" << nodes.Get(i)->GetId() << "/DeviceList/*/$ns3::WifiNetDevice/Phy/$ns3::WifiPhy/ChannelNumber";
      Config::Set(configpath.str(), UintegerValue(1));
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
      Config::Set(configpath.str(), DoubleValue(TX_GAIN));
      configpath.str(""); configpath.clear();
      configpath << "/NodeList/" << nodes.Get(i)->GetId() << "/DeviceList/1/$ns3::WifiNetDevice/Phy/RxGain";
      Config::Set(configpath.str(), DoubleValue(RX_GAIN));
      configpath.str(""); configpath.clear();
      configpath << "/NodeList/" << nodes.Get(i)->GetId() << "/DeviceList/1/$ns3::WifiNetDevice/Phy/RxSensitivity";
      Config::Set(configpath.str(), DoubleValue(RX_SENSITIVITY));
      configpath.str(""); configpath.clear();
    //   Ptr<ConstantVelocityMobilityModel> mob = nodes.Get(i)->GetObject<ConstantVelocityMobilityModel>();
    //   mob->SetVelocity(Vector(10, 0, 0));
    //}

  }

    // Ptr<ConstantVelocityMobilityModel> mob = nodes.Get(1)->GetObject<ConstantVelocityMobilityModel>();
    // mob->SetPosition(Vector(11*DISTANCE, 0, 0));
    // mob->SetVelocity(Vector(9, 0, 0));
    // Ptr<ConstantVelocityMobilityModel> mob1 = nodes.Get(2)->GetObject<ConstantVelocityMobilityModel>();
    // mob->SetPosition(Vector(10*DISTANCE, 0, 0));
    // mob->SetVelocity(Vector(10, 0, 0));
    // Ptr<ConstantVelocityMobilityModel> mob3 = nodes.Get(3)->GetObject<ConstantVelocityMobilityModel>();
    // mob->SetPosition(Vector(9*DISTANCE, 0, 0));
    // mob3->SetVelocity(Vector(5, 0, 0));
    // Ptr<ConstantVelocityMobilityModel> mob4 = nodes.Get(4)->GetObject<ConstantVelocityMobilityModel>();
    // mob->SetPosition(Vector(8*DISTANCE, 0, 0));
    // mob4->SetVelocity(Vector(2, 0, 0));
    // Ptr<ConstantVelocityMobilityModel> mob5 = nodes.Get(5)->GetObject<ConstantVelocityMobilityModel>();
    // mob->SetPosition(Vector(7*DISTANCE, 0, 0));
    // mob5->SetVelocity(Vector(3, 0, 0));
    // Ptr<ConstantVelocityMobilityModel> mob6 = nodes.Get(6)->GetObject<ConstantVelocityMobilityModel>();
    // mob->SetPosition(Vector(6*DISTANCE, 0, 0));
    // mob6->SetVelocity(Vector(4, 0, 0));
    // Ptr<ConstantVelocityMobilityModel> mob7 = nodes.Get(7)->GetObject<ConstantVelocityMobilityModel>();
    // mob->SetPosition(Vector(5*DISTANCE, 0, 0));
    // mob7->SetVelocity(Vector(6, 0, 0));
    // Ptr<ConstantVelocityMobilityModel> mob8 = nodes.Get(8)->GetObject<ConstantVelocityMobilityModel>();
    // mob->SetPosition(Vector(4*DISTANCE, 0, 0));
    // mob8->SetVelocity(Vector(8, 0, 0));
    // Ptr<ConstantVelocityMobilityModel> mob9 = nodes.Get(9)->GetObject<ConstantVelocityMobilityModel>();
    // mob->SetPosition(Vector(3*DISTANCE, 0, 0));
    // mob9->SetVelocity(Vector(9, 0, 0));
    // Ptr<ConstantVelocityMobilityModel> mob10 = nodes.Get(10)->GetObject<ConstantVelocityMobilityModel>();
    // mob->SetPosition(Vector(2*DISTANCE, 0, 0));
    // mob10->SetVelocity(Vector(2, 0, 0));
    // Ptr<ConstantVelocityMobilityModel> mob11 = nodes.Get(11)->GetObject<ConstantVelocityMobilityModel>();
    // mob->SetPosition(Vector(1*DISTANCE, 0, 0));
    // mob11->SetVelocity(Vector(15, 0, 0));


  // Set IP addresses on wifi devices
  //assigns second interface, first interface is by default. First interface is for myself
  Ipv4AddressHelper address;
  address.SetBase(NET_ADDRESS, NET_MASK_ADDRESS);
  Ipv4InterfaceContainer interfaces = address.Assign(devices); //notify methods (called from this line) will allow us to access interface to tx hello-packets


  // Enable promiscuous pcap tracing on sink node (n0) and enable network animation
  wifiPhyHelper.EnablePcap("vbp-caravan.pcap", nodes.Get(0)->GetDevice(1), false, true);
  AnimationInterface anim("vbp-caravan.xml");
  anim.EnablePacketMetadata(true);

  // Configure time resolution, simulation start and stop times.
  Time::SetResolution(Time::NS);
  //udpSourceAppPtr->SetStartTime(Seconds(SOURCE_START_TIME));
  Simulator::Stop(Seconds(SimulationTime));

 

//   if (printRoutes)
//     {
//       Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("vbp.routes", std::ios::out);
//       vbp.PrintRoutingTableAllAt (Seconds (20), routingStream);
//     }

  // Execute simulation

  Simulator::Run();
  Simulator::Destroy();

  // Success
  return 0;
}
