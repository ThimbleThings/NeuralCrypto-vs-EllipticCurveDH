#include "ReceiverApplication.h"

TypeId ReceiverApplication::GetTypeId(void)
{
	static TypeId tid = TypeId("ReceiverApplication").SetParent(Object::GetTypeId()).AddConstructor<ReceiverApplication>().AddTraceSource("AppRx", "When Receiving a packet", MakeTraceSourceAccessor(&ReceiverApplication::m_rxPacket));
	return tid;
}

ReceiverApplication::ReceiverApplication()
{
	m_counter = 0;
	m_nodeId = 0;
}
ReceiverApplication::~ReceiverApplication()
{
	m_counter = 0;
	m_nodeId = 0;
}

void ReceiverApplication::Setup(Ptr<Socket> socket, uint32_t id)
{
	m_socket = socket;
	m_nodeId = id;
}
void ReceiverApplication::StartApplication()
{
	if (m_socket == 0)
	{
		Ptr<SocketFactory> socketFactory = GetNode()->GetObject<SocketFactory>(UdpSocketFactory::GetTypeId());
		m_socket = socketFactory->CreateSocket();
		m_socket->Bind();
	}
	m_socket->SetRecvCallback(MakeCallback(&ReceiverApplication::Receive, this));
}

void ReceiverApplication::Receive(Ptr<Socket> socket)
{
	Ptr<Packet> packet;
	Address from;
	while (packet = socket->RecvFrom(from))
	{
		std::cout << "RECEIVE PACKET UID::" << packet->GetUid() << "\n";
		m_rxPacket(packet);
	}
}

void ReceiverApplication::StopApplication(void)
{
	m_socket->Close();
}