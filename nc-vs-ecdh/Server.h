#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <bits/stdc++.h>

#include "cryptopp/cryptlib.h"
#include "cryptopp/integer.h"
#include "cryptopp/eccrypto.h"

#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-routing-table-entry.h"

#include "ReceiverApplication.h"
#include "SenderApplication.h"
#include "ECDH.h"
#include "NC.h"

// TODO: Check which of these includes are actually necessary

using namespace ns3;

class Server
{
public:
	Server(uint32_t serverId, NodeContainer nodeContainer, Ipv4InterfaceContainer v4Intf,
		   std::string keyProtocol, uint32_t clientId);
	Server(uint32_t serverId, NodeContainer nodeContainer, Ipv6InterfaceContainer v6Intf,
		   std::string keyProtocol, uint32_t clientId);
	~Server();
	void Setup();
	static void SendPacketCallBack(Server *server, Ptr<const Packet> packet);
	static void ReceivePacketCallBack(Server *server, Ptr<const Packet> packet);
	uint32_t GetServerId();

private:
	bool useIpv6;
	uint32_t m_serverId;
	uint32_t m_clientId;

	NodeContainer m_nodeContainer;
	Ipv6InterfaceContainer m_v6Intf;
	Ipv4InterfaceContainer m_v4Intf;
	NetDeviceContainer m_netContainer;
	std::string m_keyProtocol;
	uint32_t m_k;
	uint32_t m_n;
	int32_t m_l;
	uint32_t m_roundThreshold;
	uint32_t m_maxUpdates;
	uint32_t m_updatesCnt;

	ECDH *m_ecdh;
	NC *m_nc;

	Ptr<ReceiverApplication> appRecv;
	Ptr<SenderApplication> appSender;

	void ReadPacket(Ptr<const Packet> packet);
	void UpdateECDH(std::string data);
	void UpdateNC(std::string data);
	void DecryptMSG(std::string data);
	void SendDataToNodeId(std::string data, uint32_t in_nodeId);
	double GetCurrentTime();
}; // Class definition end

#endif /* SERVER_H */
