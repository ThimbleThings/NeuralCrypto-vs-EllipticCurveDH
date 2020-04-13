#include "SenderApplication.h"

TypeId SenderApplication::GetTypeId(void)
{
	static TypeId tid = TypeId("SenderApplication").SetParent(Object::GetTypeId()).AddConstructor<SenderApplication>().AddTraceSource("AppTx", "When Sending a packet", MakeTraceSourceAccessor(&SenderApplication::m_txPacket));
	return tid;
}

SenderApplication::SenderApplication() {}

SenderApplication::~SenderApplication() {}

void SenderApplication::Setup(uint32_t nodeId, NodeContainer nodeContainer)
{
	m_nodeId = nodeId;
	m_nodeContainer = nodeContainer;
}

void SenderApplication::StartApplication(void)
{
	m_running = true;
}
void SenderApplication::StopApplication(void)
{
	m_running = false;
}

Ptr<Socket> SenderApplication::BindToAddress(Address recvAddress)
{
	Ptr<Socket> socketSenderUdp = Socket::CreateSocket(m_nodeContainer.Get(m_nodeId), UdpSocketFactory::GetTypeId());
	int bind = socketSenderUdp->Bind();
	int connect = socketSenderUdp->Connect(recvAddress);
	//m_running = false;
	return socketSenderUdp;
}

void SenderApplication::SendPacket(std::string data, Address recvAddress)
{
	std::cout<<"SEND PACKET::"<<recvAddress<<"\n";
	Ptr<Socket> socketSenderUdp = BindToAddress(recvAddress);
	uint16_t packetSize = data.size();
	std::cout << "SEND PACKET SIZE::" << packetSize << "\n";
	Ptr<Packet> packet = Create<Packet>((uint8_t *)data.c_str(), packetSize);
	std::cout << "SEND PACKET UID::" << packet->GetUid() << "\n";
	socketSenderUdp->Send(packet);
	m_txPacket(packet);
	socketSenderUdp->Close();
}