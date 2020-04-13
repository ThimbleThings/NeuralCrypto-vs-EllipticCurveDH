#ifndef RECEIVERAPPLICATION_H
#define RECEIVERAPPLICATION_H

#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/olsr-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/olsr-routing-protocol.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-routing-table-entry.h"

using namespace ns3;



class ReceiverApplication: public Application {
public:
	ReceiverApplication();
	virtual ~ReceiverApplication();
	void Setup(Ptr<Socket> socket, uint32_t id);
	static TypeId GetTypeId(void);
private:
	void StartApplication(void);
	void StopApplication(void);
	void Receive(Ptr<Socket> socket);
	Ptr<Socket> m_socket;
	uint32_t m_counter;
	uint32_t m_nodeId;
	TracedCallback<Ptr<const Packet> > m_rxPacket;
};

#endif /* RECEIVERAPPLICATION_H */

