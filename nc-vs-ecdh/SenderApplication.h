#ifndef SENDERAPPLICATION_H
#define SENDERAPPLICATION_H

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/olsr-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/error-model.h"


using namespace ns3;


class SenderApplication: public Application {
public:
	SenderApplication();
	virtual ~SenderApplication();
	static TypeId GetTypeId(void);
	void Setup(uint32_t nodeId, NodeContainer nodeContainer);
	void SendPacket(std::string data, Address recvAddress);
	//void SendPacketForDiscard(std::string data, Address recvAddress, NetDeviceContainer devices);

private:
	virtual void StartApplication(void);
	virtual void StopApplication(void);
	Ptr<Socket> BindToAddress(Address recvAddress);
	bool m_running;
	NodeContainer m_nodeContainer;
	uint32_t m_nodeId;
	TracedCallback<Ptr<const Packet> > m_txPacket;
};

#endif /* SENDERAPPLICATION_H */

