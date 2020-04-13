#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"

using namespace ns3;

#include "Logger.h"
#include "Initialize.h"

NS_LOG_COMPONENT_DEFINE("NC-vs-ECDH");

int main(int argc, char *argv[])
{
    bool enableFlowMonitor = false;
    uint32_t refreshTime = 20;
    uint32_t maxRounds = 0;
    uint32_t startSim = 0;
    uint32_t stopSim = 60;
    std::string keyProtocol = "ecdh";
    std::string dataRate = "3Mbps";
    std::string delay = "10ms";

    CommandLine cmd;
    cmd.AddValue("refreshTime", "Time between key negotiations in seconds [default=20]", refreshTime);
    cmd.AddValue("stopSim", "Simulation runtime in seconds [default=60]", stopSim);
    cmd.AddValue("maxRounds", "Maximum number of key negotiations (0=no limit) [default=0]", maxRounds);
    cmd.AddValue("keyProtocol", "Key negotiation protocol to use ['ecdh' or 'nc', default='ecdh']", keyProtocol);
    cmd.AddValue("dataRate", "Data rate for the network [XMbps or XKbps, X integer, default='3Mbps']", dataRate);
    cmd.AddValue("delay", "Connection delay for the network [Xms or Xs, X integer, default='10ms']", delay);
    cmd.Parse(argc, argv);

    if (keyProtocol.compare("ecdh") != 0 && keyProtocol.compare("nc") != 0)
    {
        std::cout << "Please choose either 'ecdh' or 'nc' as keyProtocol.\n"
                  << "For general options use the --help option.\n";
        return 0;
    }

    Time::SetResolution(Time::NS);
    //LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
    //LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

    NodeContainer nodes;
    nodes.Create(2);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue(dataRate));
    pointToPoint.SetChannelAttribute("Delay", StringValue(delay));
    NetDeviceContainer devices = pointToPoint.Install(nodes);

    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    // Create Apps
    NS_LOG_DEBUG("Initialize Simulation");
    std::cout << "Initialize Simulation, refreshTime=" << refreshTime
              << ", stopSim=" << stopSim
              << ", maxRounds=" << maxRounds
              << ", keyProtocol=" << keyProtocol << ".\n";
    Initialize *ncvsecdh = new Initialize(nodes, interfaces, devices, startSim, stopSim,
                                          refreshTime, maxRounds, keyProtocol, dataRate, delay);
    ncvsecdh->Setup();

    // Flow Monitor (TODO: what does this do, is it useful?)
    /*Ptr<FlowMonitor> flowmon;
    if (enableFlowMonitor)
    {
        FlowMonitorHelper flowmonHelper;
        flowmon = flowmonHelper.InstallAll();
    }*/

    NS_LOG_DEBUG("Run simulation.");
    std::cout << "Run simulation.\n";
    Simulator::Stop(Seconds(stopSim));
    Simulator::Run();

    /*if (enableFlowMonitor)
    {
        flowmon->CheckForLostPackets();
        flowmon->SerializeToXmlFile("nc-vs-ecdh.flowmon", true, true);
    }*/

    Simulator::Destroy();
    NS_LOG_DEBUG("Done.");
    std::cout << "Done.\n";
    Logger::GetInstance()->End();

    return 0;
}