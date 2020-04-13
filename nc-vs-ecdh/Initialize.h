#ifndef INITIALIZE_H
#define INITIALIZE_H

#include <iostream>
#include <sstream>
#include <bits/stdc++.h>

#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include "cryptopp/nbtheory.h"
#include "cryptopp/cryptlib.h"
#include "cryptopp/integer.h"
#include "cryptopp/osrng.h"
#include "cryptopp/eccrypto.h"
#include "cryptopp/secblock.h"
#include "cryptopp/oids.h"

#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-routing-table-entry.h"

// TODO: Check which of these includes are actually necessary

#include "Server.h"
#include "Client.h"

using namespace ns3;

class Initialize
{
public:
	Initialize(NodeContainer nodeContainer, Ipv4InterfaceContainer v4Intf,
			   NetDeviceContainer in_devices, uint32_t startTime, uint32_t stopTime,
			   uint32_t refreshTime, uint32_t maxRounds, std::string keyProtocol,
			   std::string dataRate, std::string delay);
	Initialize(NodeContainer nodeContainer, Ipv6InterfaceContainer v6Intf,
			   NetDeviceContainer in_devices, uint32_t startTime, uint32_t stopTime,
			   uint32_t refreshTime, uint32_t maxRounds, std::string keyProtocol,
			   std::string dataRate, std::string delay);
	~Initialize();
	void Setup();

private:
	bool useIpv6;
	NodeContainer m_nodeContainer;
	Ipv6InterfaceContainer m_v6Intf;
	Ipv4InterfaceContainer m_v4Intf;
	NetDeviceContainer m_netContainer;
	uint32_t m_startTime;
	uint32_t m_stopTime;
	uint32_t m_refreshTime;
	uint32_t m_maxRounds;
	std::string m_keyProtocol;
	Client *client;
	Server *server;
};

#endif /* INTIALIZE_H */
