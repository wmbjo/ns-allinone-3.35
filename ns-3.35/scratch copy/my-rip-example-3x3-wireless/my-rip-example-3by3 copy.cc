// src
//  |  10.1.1.0
//  |
//  |   10.1.2.0          10.1.3.0         
// n0 -------------- n1 -------------- n2
//  |                 |                 |                 
//  |                 |                 |                 
//  |10.1.4.0         |10.1.5.0         |10.1.6.0         
//  |                 |                 |                 
//  |    10.1.7.0     |    10.1.8.0     |       
// n3 -------------- n4 -------------- n5f
//  |                 |                 |                 
//  |                 |                 |                 
//  |10.1.9.0         |10.1.10.0         |10.1.11.0        
//  |                 |                 |                 
//  |    10.1.12.0    |    10.1.13.0    |       
// n6 -------------- n7 -------------- n8
//                                      |
//                                      |10.1.14.0
//                                     dst


#include <fstream>
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-apps-module.h"
#include "./my-ipv4-static-routing-helper.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/netanim-module.h"
#include "ns3/mobility-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("MyRipSimpleRouting");

void TearDownLink (Ptr<Node> nodeA, Ptr<Node> nodeB, uint32_t interfaceA, uint32_t interfaceB)
{
nodeA->GetObject<Ipv4> ()->SetDown (interfaceA);
nodeB->GetObject<Ipv4> ()->SetDown (interfaceB);
}

int main (int argc, char **argv)
{
bool verbose = false;
bool printRoutingTables = false;
bool showPings = true;
std::string SplitHorizon ("PoisonReverse");

CommandLine cmd (__FILE__);
cmd.AddValue ("verbose", "turn on log components", verbose);
cmd.AddValue ("printRoutingTables", "Print routing tables at 30, 60 and 90 seconds", printRoutingTables);
cmd.AddValue ("showPings", "Show Ping6 reception", showPings);
cmd.AddValue ("splitHorizonStrategy", "Split Horizon strategy to use (NoSplitHorizon, SplitHorizon, PoisonReverse)", SplitHorizon);
cmd.Parse (argc, argv);

if (verbose)
    {
    LogComponentEnableAll (LogLevel (LOG_PREFIX_TIME | LOG_PREFIX_NODE));
    LogComponentEnable ("RipSimpleRouting", LOG_LEVEL_INFO);
    LogComponentEnable ("Rip", LOG_LEVEL_ALL);
    LogComponentEnable ("Ipv4Interface", LOG_LEVEL_ALL);
    LogComponentEnable ("Icmpv4L4Protocol", LOG_LEVEL_ALL);
    LogComponentEnable ("Ipv4L3Protocol", LOG_LEVEL_ALL);
    LogComponentEnable ("ArpCache", LOG_LEVEL_ALL);
    LogComponentEnable ("V4Ping", LOG_LEVEL_ALL);
    }

if (SplitHorizon == "NoSplitHorizon")
    {
        Config::SetDefault ("ns3::Rip::SplitHorizon", EnumValue (RipNg::NO_SPLIT_HORIZON));
    }
    else if (SplitHorizon == "SplitHorizon")
    {
        Config::SetDefault ("ns3::Rip::SplitHorizon", EnumValue (RipNg::SPLIT_HORIZON));
    }
    else
    {
        Config::SetDefault ("ns3::Rip::SplitHorizon", EnumValue (RipNg::POISON_REVERSE));
    }


    NS_LOG_INFO ("Create nodes.");
    Ptr<Node> src = CreateObject<Node> ();
    Names::Add ("SrcNode", src);
    Ptr<Node> dst = CreateObject<Node> ();
    Names::Add ("DstNode", dst);
    Ptr<Node> a = CreateObject<Node> ();
    Names::Add ("RouterA", a);
    Ptr<Node> b = CreateObject<Node> ();
    Names::Add ("RouterB", b);
    Ptr<Node> c = CreateObject<Node> ();
    Names::Add ("RouterC", c);
    Ptr<Node> d = CreateObject<Node> ();
    Names::Add ("RouterD", d);
    Ptr<Node> e = CreateObject<Node> ();
    Names::Add ("RouterE", e);
    Ptr<Node> f = CreateObject<Node> ();
    Names::Add ("RouterF", f);
    Ptr<Node> g = CreateObject<Node> ();
    Names::Add ("RouterG", g);
    Ptr<Node> h = CreateObject<Node> ();
    Names::Add ("RouterH", h);
    Ptr<Node> i = CreateObject<Node> ();
    Names::Add ("RouterI", i);
    NodeContainer net1 (src, a);
    NodeContainer net2 (a, b);
    NodeContainer net3 (b, c);
    NodeContainer net4 (a, d);
    NodeContainer net5 (b, e);
    NodeContainer net6 (c, f);
    NodeContainer net7 (d, e);
    NodeContainer net8 (e, f);
    NodeContainer net9 (d, g);
    NodeContainer net10 (e, h);
    NodeContainer net11 (f, i);
    NodeContainer net12 (g, h);
    NodeContainer net13 (h, i);
    NodeContainer net14 (i, dst);

    

    NodeContainer routers (a, b, c, d,e);
    routers.Add(f);
    routers.Add(g);
    routers.Add(h);
    routers.Add(i);
    NodeContainer nodes (src, dst);

    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);
    mobility.Install(routers);


    NS_LOG_INFO ("Create channels.");
    CsmaHelper csma;
    csma.SetChannelAttribute ("DataRate", DataRateValue (5000000));
    csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
    NetDeviceContainer ndc1 = csma.Install (net1);
    NetDeviceContainer ndc2 = csma.Install (net2);
    NetDeviceContainer ndc3 = csma.Install (net3);
    NetDeviceContainer ndc4 = csma.Install (net4);
    NetDeviceContainer ndc5 = csma.Install (net5);
    NetDeviceContainer ndc6 = csma.Install (net6);
    NetDeviceContainer ndc7 = csma.Install (net7);
    NetDeviceContainer ndc8 = csma.Install (net8);
    NetDeviceContainer ndc9 = csma.Install (net9);
    NetDeviceContainer ndc10 = csma.Install (net10);
    NetDeviceContainer ndc11 = csma.Install (net11);
    NetDeviceContainer ndc12 = csma.Install (net12);
    NetDeviceContainer ndc13 = csma.Install (net13);
    NetDeviceContainer ndc14 = csma.Install (net14);

    NS_LOG_INFO ("Create IPv4 and routing");
    RipHelper ripRouting;

    // Rule of thumb:
    // Interfaces are added sequentially, starting from 0
    // However, interface 0 is always the loopback...
    ripRouting.ExcludeInterface (a, 1);
    ripRouting.ExcludeInterface (d, 3);

    ripRouting.SetInterfaceMetric (c, 3, 10);
    ripRouting.SetInterfaceMetric (d, 1, 10);

    Ipv4ListRoutingHelper listRH;
    listRH.Add (ripRouting, 0);
//  Ipv4StaticRoutingHelper staticRh;
//  listRH.Add (staticRh, 5);

    InternetStackHelper internet;
    internet.SetIpv6StackInstall (false);
    internet.SetRoutingHelper (listRH);
    internet.Install (routers);

    InternetStackHelper internetNodes;
    internetNodes.SetIpv6StackInstall (false);
    internetNodes.Install (nodes);

    // Assign addresses.
    // The source and destination networks have global addresses
    // The "core" network just needs link-local addresses for routing.
    // We assign global addresses to the routers as well to receive
    // ICMPv6 errors.
    NS_LOG_INFO ("Assign IPv4 Addresses.");
    Ipv4AddressHelper ipv4;

    ipv4.SetBase (Ipv4Address ("10.1.1.0"), Ipv4Mask ("255.255.255.0"));
    Ipv4InterfaceContainer iic1 = ipv4.Assign (ndc1);

    ipv4.SetBase (Ipv4Address ("10.1.2.0"), Ipv4Mask ("255.255.255.0"));
    Ipv4InterfaceContainer iic2 = ipv4.Assign (ndc2);

    ipv4.SetBase (Ipv4Address ("10.1.3.0"), Ipv4Mask ("255.255.255.0"));
    Ipv4InterfaceContainer iic3 = ipv4.Assign (ndc3);

    ipv4.SetBase (Ipv4Address ("10.1.4.0"), Ipv4Mask ("255.255.255.0"));
    Ipv4InterfaceContainer iic4 = ipv4.Assign (ndc4);

    ipv4.SetBase (Ipv4Address ("10.1.5.0"), Ipv4Mask ("255.255.255.0"));
    Ipv4InterfaceContainer iic5 = ipv4.Assign (ndc5);

    ipv4.SetBase (Ipv4Address ("10.1.6.0"), Ipv4Mask ("255.255.255.0"));
    Ipv4InterfaceContainer iic6 = ipv4.Assign (ndc6);

    ipv4.SetBase (Ipv4Address ("10.1.7.0"), Ipv4Mask ("255.255.255.0"));
    Ipv4InterfaceContainer iic7 = ipv4.Assign (ndc7);

    ipv4.SetBase (Ipv4Address ("10.1.8.0"), Ipv4Mask ("255.255.255.0"));
    Ipv4InterfaceContainer iic8 = ipv4.Assign (ndc8);

    ipv4.SetBase (Ipv4Address ("10.1.9.0"), Ipv4Mask ("255.255.255.0"));
    Ipv4InterfaceContainer iic9 = ipv4.Assign (ndc9);

    ipv4.SetBase (Ipv4Address ("10.1.10.0"), Ipv4Mask ("255.255.255.0"));
    Ipv4InterfaceContainer iic10 = ipv4.Assign (ndc10);

    ipv4.SetBase (Ipv4Address ("10.1.11.0"), Ipv4Mask ("255.255.255.0"));
    Ipv4InterfaceContainer iic11 = ipv4.Assign (ndc11);

    ipv4.SetBase (Ipv4Address ("10.1.12.0"), Ipv4Mask ("255.255.255.0"));
    Ipv4InterfaceContainer iic12 = ipv4.Assign (ndc12);

    ipv4.SetBase (Ipv4Address ("10.1.13.0"), Ipv4Mask ("255.255.255.0"));
    Ipv4InterfaceContainer iic13 = ipv4.Assign (ndc13);

    ipv4.SetBase (Ipv4Address ("10.1.14.0"), Ipv4Mask ("255.255.255.0"));
    Ipv4InterfaceContainer iic14 = ipv4.Assign (ndc14);

    Ptr<Ipv4StaticRouting> staticRouting;
    staticRouting = Ipv4RoutingHelper::GetRouting <Ipv4StaticRouting> (src->GetObject<Ipv4> ()->GetRoutingProtocol ());
    staticRouting->SetDefaultRoute ("10.1.1.2", 1 );
    staticRouting = Ipv4RoutingHelper::GetRouting <Ipv4StaticRouting> (dst->GetObject<Ipv4> ()->GetRoutingProtocol ());
    staticRouting->SetDefaultRoute ("10.1.14.1", 1 );

    // if (printRoutingTables)
    // {
    //     RipHelper routingHelper;

    //     Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> (&std::cout);

    //     routingHelper.PrintRoutingTableAt (Seconds (30.0), a, routingStream);
    //     routingHelper.PrintRoutingTableAt (Seconds (30.0), b, routingStream);
    //     routingHelper.PrintRoutingTableAt (Seconds (30.0), c, routingStream);
    //     routingHelper.PrintRoutingTableAt (Seconds (30.0), d, routingStream);

    //     routingHelper.PrintRoutingTableAt (Seconds (60.0), a, routingStream);
    //     routingHelper.PrintRoutingTableAt (Seconds (60.0), b, routingStream);
    //     routingHelper.PrintRoutingTableAt (Seconds (60.0), c, routingStream);
    //     routingHelper.PrintRoutingTableAt (Seconds (60.0), d, routingStream);

    //     routingHelper.PrintRoutingTableAt (Seconds (90.0), a, routingStream);
    //     routingHelper.PrintRoutingTableAt (Seconds (90.0), b, routingStream);
    //     routingHelper.PrintRoutingTableAt (Seconds (90.0), c, routingStream);
    //     routingHelper.PrintRoutingTableAt (Seconds (90.0), d, routingStream);
    // }

    NS_LOG_INFO ("Create Applications.");
    uint32_t packetSize = 1024;
    Time interPacketInterval = Seconds (1.0);
    V4PingHelper ping ("10.1.14.2");

    ping.SetAttribute ("Interval", TimeValue (interPacketInterval));
    ping.SetAttribute ("Size", UintegerValue (packetSize));
    if (showPings)
    {
        ping.SetAttribute ("Verbose", BooleanValue (true));
    }
    ApplicationContainer apps = ping.Install (src); //ping application is at ICMP layer
    apps.Start (Seconds (10.0));
    apps.Stop (Seconds (110.0));

    AsciiTraceHelper ascii;
    csma.EnableAsciiAll (ascii.CreateFileStream ("my-rip-simple-routing.tr"));
    csma.EnablePcapAll ("my-rip-simple-routing", true);

    //Simulator::Schedule (Seconds (40), &TearDownLink, b, d, 3, 2);


   AnimationInterface anim ("my-rip-example-3by3.xml");
   anim.SetConstantPosition(src, 0, -10);
   anim.SetConstantPosition(a, 0, 0);
   anim.SetConstantPosition(b, 10, 0);
   anim.SetConstantPosition(c, 20, 0);
   anim.SetConstantPosition(d, 0, 10);
   anim.SetConstantPosition(e, 10, 10);
   anim.SetConstantPosition(f, 20, 10);
   anim.SetConstantPosition(g, 0, 20);
   anim.SetConstantPosition(h, 10, 20);
   anim.SetConstantPosition(i, 20, 20);
   anim.SetConstantPosition(dst, 20, 30);
   anim.EnablePacketMetadata(true);

    /* Now, do the actual simulation. */
    NS_LOG_INFO ("Run Simulation.");
    Simulator::Stop (Seconds (131.0));
    Simulator::Run ();
    Simulator::Destroy ();
    NS_LOG_INFO ("Done.");
}

//Replicate this exact file with the 2 classes in my scratch folder. RIP Helper and RIP Protocol ().h and .cc, for each). 4 files total. Append "my-" in front of these files. Update this script to make sure I'm using my custom classes, not the ones from ns-3.
