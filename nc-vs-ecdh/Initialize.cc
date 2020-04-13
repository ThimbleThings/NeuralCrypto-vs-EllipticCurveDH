#include <sys/time.h>

#include "Server.h"
#include "Client.h"
#include "Initialize.h"
#include "Utils.h"

Initialize::Initialize(NodeContainer nodeContainer, Ipv6InterfaceContainer v6Intf,
					   NetDeviceContainer in_devices, uint32_t startTime, uint32_t stopTime,
					   uint32_t refreshTime, uint32_t maxRounds, std::string keyProtocol,
					   std::string dataRate, std::string delay)
{
	useIpv6 = true;
	m_startTime = startTime;
	m_stopTime = stopTime;
	m_nodeContainer = nodeContainer;
	m_v6Intf = v6Intf;
	m_netContainer = in_devices;
	m_refreshTime = refreshTime;
	m_maxRounds = maxRounds;
	m_keyProtocol = keyProtocol;
	Utils::SetRefreshTime(m_refreshTime);
	Utils::SetStartTime(m_startTime);
	Utils::SetStopTime(m_stopTime);
	Utils::SetPort(DEFAULT_PORT_NR);
	Utils::SetKeyProtocol(keyProtocol);
	Utils::SetDataRate(dataRate);
	Utils::SetDelay(delay);
}

Initialize::Initialize(NodeContainer nodeContainer, Ipv4InterfaceContainer v4Intf,
					   NetDeviceContainer in_devices, uint32_t startTime, uint32_t stopTime,
					   uint32_t refreshTime, uint32_t maxRounds, std::string keyProtocol,
					   std::string dataRate, std::string delay)
{
	useIpv6 = false;
	m_startTime = startTime;
	m_stopTime = stopTime;
	m_nodeContainer = nodeContainer;
	m_v4Intf = v4Intf;
	m_netContainer = in_devices;
	m_refreshTime = refreshTime;
	m_maxRounds = maxRounds;
	m_keyProtocol = keyProtocol;
	Utils::SetRefreshTime(m_refreshTime);
	Utils::SetStartTime(m_startTime);
	Utils::SetStopTime(m_stopTime);
	Utils::SetPort(DEFAULT_PORT_NR);
	Utils::SetKeyProtocol(keyProtocol);
	Utils::SetDataRate(dataRate);
	Utils::SetDelay(delay);
}

Initialize::~Initialize()
{
	delete client;
	delete server;
}

void Initialize::Setup()
{
	uint32_t clientId = 0;
	uint32_t serverId = 1;

	time_t seconds = time(NULL);
	Utils::SetRngSeed((unsigned int)seconds);
	srand(Utils::GetRngSeed());

	if (useIpv6 == true)
	{
		client = new Client(clientId, m_nodeContainer, m_v6Intf, m_keyProtocol, m_maxRounds, serverId);
		client->Setup();
		server = new Server(serverId, m_nodeContainer, m_v6Intf, m_keyProtocol, clientId);
		server->Setup();
	}
	else
	{
		client = new Client(clientId, m_nodeContainer, m_v4Intf, m_keyProtocol,  m_maxRounds, serverId);
		client->Setup();
		server = new Server(serverId, m_nodeContainer, m_v4Intf, m_keyProtocol, clientId);
		server->Setup();
	}
}