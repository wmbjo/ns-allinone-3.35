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

#include <iostream>
#include <cmath>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/aodv-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/netanim-module.h"
#include "ns3/ns2-mobility-helper.h"
#include "ns3/wifi-80211p-helper.h"
#include "ns3/wave-mac-helper.h"
// code that I have written:
#include "simulationConfiguration.h"

// To run:
//       sudo ./waf --run "scratch/sumoSim --SimulationTime=100 --TraceFile=/home/rventura/Sumo/factor80/mobility.tcl --OutputFile=sumoF80.xml --NumNodes=40"
// For debugger:
        //sudo ./waf --run scratch/sumoSim --command-template="gdb --args %s --SimulationTime=100 --TraceFile=/home/rventura/Sumo/factor80/mobility.tcl --OutputFile=sumoF80.xml --NumNodes=40"

// Simulation
int main (int argc, char **argv) {
  // Allow user to set different inputs
  std::string PhyMode("OfdmRate6MbpsBW10MHz"); 
  std::string PhyModeB("DsssRate11Mbps");
  std::string TraceFile; // to specify path of SUMO generated file for mobility model
  std::string OutputFile = "sumoSim.xml"; // specify name for animation file
  uint32_t NumNodes = 16;
  double SimulationTime = 300; // seconds
  uint32_t PRNGRunNumber = 1;
  CommandLine cmd;
  cmd.AddValue("NumNodes", "Number of nodes", NumNodes);
  cmd.AddValue("TraceFile", "Specify path of mobility file to use", TraceFile);
  cmd.AddValue("OutputFile", "Specify name for animation file", OutputFile);
  cmd.AddValue("SimulationTime", "Simulation time (seconds)", SimulationTime);
  cmd.AddValue("PRNGRunNumber", 
            "Run number of the pseudo random number generator (PRNG) that determines packet transmission times", PRNGRunNumber);
  cmd.Parse(argc, argv);

  // Create nodes and keep them on a container. 
  // Install the Internet stack and set the routing protocol to AODV.
  //cars allVehicles(NumNodes); // to use nodes, call allVehicles.nodes
  NodeContainer nodes;
  void (*rtePacketPtr) (Ptr<Socket> socket,Ptr<Packet> packet);
  rtePacketPtr = &RoutePacketFromQueue;
  for (uint32_t i = 0; i < NumNodes; i++) {
        Ptr<car> mycar = CreateObject<car> ();
        mycar->setRoutePacketPointer(rtePacketPtr);
        nodes.Add(mycar);       
  }
  InternetStackHelper stack;
  stack.Install(nodes); 
  
  // Create Ns2MobilityHelper with the specified trace log file as parameter
  Ns2MobilityHelper ns2 = Ns2MobilityHelper (TraceFile);
  ns2.Install (); // configure movements for each node, while reading trace file
  
  // Create channel of constant propagation speed and Friis loss. Enable Radiotap link
  // layer information. Configure the wifi MAC layer in Ad Hoc mode. Use the 802.11b
  // standard (DSSS PHY (Clause 15) and HR/DSSS PHY (Clause 18)). Configure the transmission
  // mode specified by the user with PhyMode. Connect devices to nodes and channel using
  // the MAC and physical-layer configuration previously identified.
  YansWifiChannelHelper wifiChannelHelper = YansWifiChannelHelper::Default();
  wifiChannelHelper.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannelHelper.AddPropagationLoss("ns3::FriisPropagationLossModel", "Frequency", DoubleValue(FREQ)
                                            , "SystemLoss", DoubleValue(SYS_LOSS), "MinLoss", DoubleValue(MIN_LOSS));
  YansWifiPhyHelper wifiPhyHelper = YansWifiPhyHelper::Default();
  wifiPhyHelper.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11);
  wifiPhyHelper.SetChannel(wifiChannelHelper.Create());
  Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default ();  
  wifi80211p.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue (PhyMode)
                                                , "ControlMode",StringValue (PhyMode));
  Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue(PhyMode));
  NqosWaveMacHelper wifi80211pMac = NqosWaveMacHelper::Default ();
  NetDeviceContainer devices = wifi80211p.Install (wifiPhyHelper, wifi80211pMac, nodes); 

  // Configure frequency, transmission power, tx gain, rx gain, and sensitivity of the physical layer
  std::ostringstream configpath;
  configpath.str(""); configpath.clear();
  Vector origin = Vector3D(0,4000,0);
  uint32_t min = 0; //assume 0 is closest to origin
  uint32_t minD = CalculateDistance((nodes.Get(0))->GetObject<MobilityModel>()->GetPosition (),origin);
  uint32_t currentD = 0;
  for(uint32_t i = 0; i < NumNodes; i++) {
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
    if (i >1) {
            Vector pos1 = (nodes.Get(i))->GetObject<MobilityModel>()->GetPosition (); // Get position
            currentD = CalculateDistance(pos1,origin);
            if (currentD < minD) {
                min = i;
                minD = currentD;
            }
    }
  }
  //min = 38;
  std::cout << "node closest to origin is: " << min << std::endl;

  // Assign IP addresses to wifi network devices
  Ipv4AddressHelper address;
  address.SetBase(NET_ADDRESS, NET_MASK_ADDRESS);
  Ipv4InterfaceContainer interfaces = address.Assign(devices); // each node has one address
  
  // data Packet 
  Ptr<Socket> udpSourceSocketData = Socket::CreateSocket(nodes.Get(min), UdpSocketFactory::GetTypeId());
  Ptr<dataApp> udpSourceAppPtrData = CreateObject<dataApp>();
  udpSourceAppPtrData->Setup(udpSourceSocketData, UDP_PORT_ROUTING, DATA_PACKET_RATE, PRNGRunNumber); //
  nodes.Get(min)->AddApplication(udpSourceAppPtrData);
  udpSourceAppPtrData->SetStartTime(Seconds(DATA_START_TIME));
  for(uint32_t i = 0; i < NumNodes; i++) {
        // add routing app
        Ptr<Socket> routeSocket = Socket::CreateSocket(nodes.Get(i), UdpSocketFactory::GetTypeId());
        Ptr<routingApp> routingAppPtr = CreateObject<routingApp>();
        routingAppPtr->Setup(routeSocket, UDP_PORT_ROUTING);
        nodes.Get(i)->AddApplication(routingAppPtr);
        routeSocket->SetRecvCallback(MakeCallback(&RoutePacketFromSocket));
        routingAppPtr->SetStartTime(Seconds(PERIODIC_START_TIME));
        nodes.Get(i)->GetObject<car>()->setSocketPointer(routeSocket);
        
        // periodic Packet       
        Ptr<Socket> udpSourceSocket = Socket::CreateSocket(nodes.Get(i), UdpSocketFactory::GetTypeId()); // for every node
        Ptr<periodicApp> udpSourceAppPtr = CreateObject<periodicApp>();
        udpSourceAppPtr->Setup(udpSourceSocket, UDP_PORT_PERIODIC, PERIODIC_PACKET_RATE, PRNGRunNumber);
        nodes.Get(i)->AddApplication(udpSourceAppPtr);
        udpSourceSocket->SetRecvCallback(MakeCallback(&ReceivePeriodicPacket));      
        udpSourceAppPtr->SetStartTime(Seconds(PERIODIC_START_TIME)); 
        nodes.Get(i)->GetObject<car>()->CheckForQueueRemoval(); // schedule packets for first time, will schedule itself after
        nodes.Get(i)->GetObject<car>()->getSpeedValue();
  } 
    
  // Enable network animation
  AnimationInterface anim(OutputFile);
  for (uint32_t nodeId = 0; nodeId < NumNodes; ++nodeId) {
      anim.UpdateNodeSize (nodeId, 150, 150);
      anim.UpdateNodeColor(nodeId, 0, 255, 0); // initialize green, will use this color for not on network
  }
  anim.EnablePacketMetadata(true); // allows to see info of packets
  anim.SetMaxPktsPerTraceFile(1000000); // reached 10000 within 30 sec with 20 nodes
  
  // Configure time resolution, simulation start and stop times.
  Time::SetResolution(Time::NS);
  Simulator::Stop(Seconds(SimulationTime));

  // Execute simulation
  Simulator::Run();
  Simulator::Destroy();

  // Success
  return 0;
}
