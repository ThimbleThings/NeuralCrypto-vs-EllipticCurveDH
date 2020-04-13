#ifndef CLIENT_H
#define CLIENT_H

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

#include "Eigen/Dense"

class Client
{
public:
	Client(uint32_t clientId, NodeContainer nodeContainer, Ipv4InterfaceContainer v4Intf,
		   std::string keyProtocol, uint32_t maxUpdates, uint32_t serverId);
	Client(uint32_t clientId, NodeContainer nodeContainer, Ipv6InterfaceContainer v6Intf,
		   std::string keyProtocol, uint32_t maxUpdates, uint32_t serverId);
	~Client();
	void Setup();
	static void SendPacketCallBack(Client *client, Ptr<const Packet> packet);
	static void ReceivePacketCallBack(Client *client, Ptr<const Packet> packet);
	uint32_t GetClientId();

private:
	bool useIpv6;
	uint32_t m_clientId;
	uint32_t m_serverId;
	NodeContainer m_nodeContainer;
	Ipv4InterfaceContainer m_v4Intf;
	Ipv6InterfaceContainer m_v6Intf;
	std::string m_keyProtocol;
	uint32_t m_k;
	uint32_t m_n;
	int32_t m_l;
	uint32_t m_roundThreshold;
	uint32_t m_round;
	uint32_t m_maxUpdates;
	uint32_t m_updatesCnt;

	ECDH *m_ecdh;
	NC *m_nc;

	Ptr<SenderApplication> appSend;
	Ptr<ReceiverApplication> appRecv;

	std::map<uint32_t, ECDH> m_ecdhMap;
	std::map<uint32_t, uint32_t> m_receivedECDHMap;
	std::map<uint32_t, uint32_t> m_receivedECDHMapConf;
	std::map<uint32_t, uint32_t> m_receivedDataMap;
	std::map<uint32_t, uint32_t> m_maxTryMap;
	std::map<uint32_t, EventId> m_DhTimeoutMap;
	std::map<uint32_t, EventId> m_KeyUpdTimeoutMap;

	void UpdateKey();
	void InitECDH();
	void InitNC();

	void ReadPacket(Ptr<const Packet> packet);
	void UpdateECDH(std::string data);
	void UpdateNC(std::string data);
	void DecryptMSG(std::string data);

	void ScheduleUpdateKey(uint32_t in_refreshTime);

	void SendEncryptedMessage();
	void SendDataToNodeId(std::string data, uint32_t in_nodeId);
	double GetCurrentTime();

}; // Class definition end

#endif /* CLIENT_H */
