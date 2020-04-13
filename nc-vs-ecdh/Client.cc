#include "Client.h"
#include "Utils.h"
#include "EncryptDecrypt.h"
#include "Logger.h"

// Same definition as for server, since including through a header throws initialization errors
Eigen::IOFormat ClientFmt(Eigen::FullPrecision, Eigen::DontAlignCols, ";", ";", "", "", "", "");

Client::Client(uint32_t clientId, NodeContainer nodeContainer, Ipv4InterfaceContainer v4Intf,
			   std::string keyProtocol, uint32_t maxUpdates, uint32_t serverId)
{
	useIpv6 = false;
	m_clientId = clientId;
	m_nodeContainer = nodeContainer;
	m_v4Intf = v4Intf;
	m_keyProtocol = keyProtocol;
	m_maxUpdates = maxUpdates;
	m_serverId = serverId;
	m_updatesCnt = 0;
	m_ecdh = NULL;
	m_nc = NULL;

	if (m_keyProtocol.compare("nc") == 0)
	{
		m_k = 4;
		m_n = 6;
		m_l = 4;
		m_roundThreshold = 600;
	}
}

Client::Client(uint32_t clientId, NodeContainer nodeContainer, Ipv6InterfaceContainer v6Intf,
			   std::string keyProtocol, uint32_t maxUpdates, uint32_t serverId)
{
	useIpv6 = true;
	m_clientId = clientId;
	m_nodeContainer = nodeContainer;
	m_v6Intf = v6Intf;
	m_keyProtocol = keyProtocol;
	m_maxUpdates = maxUpdates;
	m_serverId = serverId;
	m_updatesCnt = 0;
	m_ecdh = NULL;
	m_nc = NULL;

	if (m_keyProtocol.compare("nc") == 0)
	{
		m_k = 4;
		m_n = 6;
		m_l = 4;
		m_roundThreshold = 600;
	}
}

Client::~Client()
{
	if (m_ecdh != NULL)
	{
		delete m_ecdh;
	}
	if (m_nc != NULL)
	{
		delete m_nc;
	}
}

void Client::Setup()
{
	appSend = CreateObject<SenderApplication>();
	appRecv = CreateObject<ReceiverApplication>();

	ScheduleUpdateKey(Utils::GetRefreshTime());

	//SETUP SENDER
	appSend->Setup(m_clientId, m_nodeContainer);
	//TRACE
	appSend->TraceConnectWithoutContext("AppTx", MakeBoundCallback(&Client::SendPacketCallBack, this));
	m_nodeContainer.Get(m_clientId)->AddApplication(appSend);
	appSend->SetStartTime(Seconds(Utils::GetStartTime()));
	appSend->SetStopTime(Seconds(Utils::GetStopTime()));

	//SETTING UP RECEIVER APP
	Ptr<Socket> socketRecvUdp = Socket::CreateSocket(m_nodeContainer.Get(m_clientId), UdpSocketFactory::GetTypeId());
	//TRACE
	appRecv->TraceConnectWithoutContext("AppRx", MakeBoundCallback(&Client::ReceivePacketCallBack, this));

	if (useIpv6 == true)
	{
		Inet6SocketAddress recvAddress = Inet6SocketAddress(m_v6Intf.GetAddress(m_clientId, 1), Utils::GetClientPort());
		socketRecvUdp->Bind(recvAddress);
	}
	else
	{
		InetSocketAddress recvAddress = InetSocketAddress(m_v4Intf.GetAddress(m_clientId, 0), Utils::GetClientPort());
		socketRecvUdp->Bind(recvAddress);
	}

	m_nodeContainer.Get(m_clientId)->AddApplication(appRecv);
	appRecv->Setup(socketRecvUdp, m_clientId);
	appRecv->SetStartTime(Seconds(Utils::GetStartTime()));
	appRecv->SetStopTime(Seconds(Utils::GetStopTime()));
}

void Client::SendPacketCallBack(Client *client, Ptr<const Packet> packet)
{
	Logger::GetInstance()->TraceTransmitPacket(packet, client->GetClientId());
}

void Client::ReceivePacketCallBack(Client *client, Ptr<const Packet> packet)
{
	Logger::GetInstance()->TraceReceivePacket(packet, client->GetClientId());
	client->ReadPacket(packet);
}

uint32_t Client::GetClientId()
{
	return m_clientId;
}

void Client::UpdateKey()
{
	if (m_keyProtocol.compare("ecdh") == 0)
	{
		InitECDH();									// Initializes the key negotiation with the server
		ScheduleUpdateKey(Utils::GetRefreshTime()); // Schedules the next negotiation
	}
	else if (m_keyProtocol.compare("nc") == 0)
	{
		InitNC();									// Initializes the key negotiation with the server
		ScheduleUpdateKey(Utils::GetRefreshTime()); // Schedules the next negotiation
	}
	else
	{
		std::cout << "m_keyProtocol is unknown: " << m_keyProtocol << "\n";
		// Don't need to schedule when protocol unknown.
		// Simulation will terminate because no more events are scheduled.
	}
}

void Client::InitECDH()
{
	// Clean up if object from previous negotiation still exists
	if (m_ecdh != NULL)
	{
		delete m_ecdh;
	}

	// Generate the ECDH key pair
	m_ecdh = new ECDH();
	m_ecdh->GenerateKeyPair();

	// Send the public key to the server
	std::ostringstream oss;
	oss.str("");
	oss << "ECDH,BEGIN," << m_ecdh->GetPublicKey() << ",";
	std::cout << "Client will send: " << oss.str() << "\n";
	SendDataToNodeId(oss.str(), m_serverId);
}

void Client::InitNC()
{
	// Clean up if object from previous negotiation still exists
	if (m_nc != NULL)
	{
		delete m_nc;
	}

	// Create TPM, generate first input and calculate output bit
	m_nc = new NC(m_k, m_n, m_l, m_roundThreshold, std::string("hebbian"));
	Eigen::VectorXi inVector = m_nc->GenerateInputVector();
	int8_t tau = m_nc->CalculateOutput(inVector);

	m_round = 0;

	// Send the input vector and the output bit to the partner TPM
	std::ostringstream oss;
	oss << "NC,BEGIN," << inVector.format(ClientFmt) << "," << int(tau) << "," << int(tau); // second tau to be consistent, conversion to print properly
	std::cout << "Client will send: " << oss.str() << "\n";
	SendDataToNodeId(oss.str(), m_serverId);
}

void Client::ReadPacket(Ptr<const Packet> packet)
{
	std::cout << "Client got packet at time:" << GetCurrentTime() << "\n";

	if (packet->GetSize() <= 0)
	{
		return;
	}

	std::stringstream packetData;
	packet->CopyData(&packetData, packet->GetSize());

	// Extract first field from Dataframe format
	std::string str_purpose;
	std::getline(packetData, str_purpose, ','); // Separates the purpose field from data

	if (str_purpose.compare("ECDH") == 0)
	{
		std::string str_data;
		std::getline(packetData, str_data); // Gets the remaining data
		UpdateECDH(str_data);
	}
	else if (str_purpose.compare("NC") == 0)
	{
		std::string str_data;
		std::getline(packetData, str_data); // Gets the remaining data
		UpdateNC(str_data);
	}
	else if (str_purpose.compare("MSG") == 0)
	{
		std::string str_data;
		std::getline(packetData, str_data, ','); // Remove empty State
		std::getline(packetData, str_data);		 // Gets the remaining data
		DecryptMSG(str_data);
	}
	else
	{
		std::cout << "Message purpose unknown: " << str_purpose << "\n";
	}
}

void Client::UpdateECDH(std::string data)
{
	if (m_ecdh == NULL)
	{
		std::cout << "ECDH is not initialized, ignoring package\n";
		return;
	}

	std::cout << "Client UpdateECDH: " << data << "\n";

	boost::char_separator<char> sep(",");
	boost::tokenizer<boost::char_separator<char>> tokens(data, sep);
	boost::tokenizer<boost::char_separator<char>>::iterator beg = tokens.begin();
	std::string str_state = *beg;

	if (str_state.compare("FINAL") != 0)
	{
		std::cout << "Unknown ECDH state: " << str_state << "\n";
		return;
	}

	beg++;
	std::string str_publicKey = *beg;
	CryptoPP::Integer *publicKeySender = new CryptoPP::Integer(str_publicKey.c_str());

	m_ecdh->GenerateSecret(publicKeySender);

	// Once the secret is generated the publicKeySender is not needed anymore
	delete publicKeySender;

	// Log the key generated by the client
	Logger::GetInstance()->LogKey(GetClientId(), m_ecdh->GetSecret());

	// Schedule sending of the encrypted message
	Simulator::Schedule(Seconds(1), &Client::SendEncryptedMessage, this);
}

void Client::UpdateNC(std::string data)
{
	//std::cout << "Client UpdateNC: " << data << "\n";
	// We use delimiter '.', since somehow we sometimes read garbage otherwise...
	boost::char_separator<char> sep(",");
	boost::tokenizer<boost::char_separator<char>> tokens(data, sep);
	boost::tokenizer<boost::char_separator<char>>::iterator beg = tokens.begin();
	std::string str_state = *beg;

	if (str_state.compare("UPDATE") == 0)
	{
		if (m_nc == NULL)
		{
			std::cout << "Received Update Package before Begin, ignoring\n";
			return;
		}

		beg++;
		std::string str_inVector = *beg;
		// Transform the string into the Eigen::Vector
		Eigen::VectorXi inVector = m_nc->VectorFromString(str_inVector);

		beg++;
		int8_t partnerTau = std::stoi(*beg);
		beg++;
		int8_t lastPartnerTau = std::stoi(*beg);

		// Update the weights using the lastPartnerTau for the last input sent by this server
		m_nc->UpdateWeights(lastPartnerTau);

		// Calculate the output bit for the received input from the client
		int8_t lastTau = m_nc->CalculateOutput(inVector);
		// Update the weights using the partnerTau
		m_nc->UpdateWeights(partnerTau);

		// Create new update package if we are below the roundThreshold
		if (m_nc->GetCurrentRound() < m_roundThreshold)
		{
			// Generate a new input and calculate output bit
			inVector = m_nc->GenerateInputVector();
			int8_t tau = m_nc->CalculateOutput(inVector);

			// Send the input vector, the output bit and the output bit for the last input to the partner TPM
			std::ostringstream oss;
			oss << "NC,UPDATE," << inVector.format(ClientFmt) << "," << int(tau) << "," << int(lastTau); // conversion to print properly
			//std::cout << "Client will send: " << oss.str() << "\n";
			SendDataToNodeId(oss.str(), m_serverId);
		}
		// Otherwise create a final package
		else
		{
			// Generate a zero vector
			inVector = Eigen::VectorXi::Zero(m_n * m_k);

			// Send the zero vector, and the output bit for the last input to the partner TPM
			std::ostringstream oss;
			oss << "NC,FINAL," << inVector.format(ClientFmt) << "," << int(lastTau) << "," << int(lastTau); // conversion to print properly
			//std::cout << "Client will send: " << oss.str() << "\n";
			SendDataToNodeId(oss.str(), m_serverId);

			// Generate the key that will be used to decrypt and encrypt a message
			m_nc->GenerateKey();
			// Log the key generated by the client
			Logger::GetInstance()->LogKey(GetClientId(), m_nc->GetSecret());

			// If sending FINAL package client will not receive a FINAL package
			// Schedule sending of the encrypted message
			Simulator::Schedule(Seconds(1), &Client::SendEncryptedMessage, this);
		}
	}
	else if (str_state.compare("FINAL") == 0)
	{
		if (m_nc == NULL)
		{
			std::cout << "Received Final Package before Begin, ignoring\n";
			return;
		}

		beg++; // Ignore the inVector, since it is zero anyway
		beg++;
		int8_t partnerTau = std::stoi(*beg);

		// Update the weights using the lastPartnerTau for the last input sent by this server
		m_nc->UpdateWeights(partnerTau);

		// Generate the key that will be used to decrypt and encrypt a message
		m_nc->GenerateKey();
		// Log the key generated by the client
		Logger::GetInstance()->LogKey(GetClientId(), m_nc->GetSecret());

		// FINAL package from server only if FINAL not sent by client
		// Schedule sending of the encrypted message
		Simulator::Schedule(Seconds(1), &Client::SendEncryptedMessage, this);
	}
	else
	{
		std::cout << "Unknown NC state: " << str_state << "\n";
		return;
	}
}

void Client::DecryptMSG(std::string data)
{
	std::cout << "Client DecryptMSG: " << data << "\n";

	try
	{
		if (m_keyProtocol.compare("ecdh") == 0)
		{
			std::cout << "Decrypted Message: " << AES::GetInstance()->DecryptMessage(m_ecdh->GetSecret(), data) << "\n";
		}
		else if (m_keyProtocol.compare("nc") == 0)
		{
			std::cout << "Decrypted Message: " << AES::GetInstance()->DecryptMessage(m_nc->GetSecret(), data) << "\n";
		}
	}
	catch (CryptoPP::InvalidCiphertext const &e)
	{
		// Most certainly the synchronization was not successful within the given number of rounds.
		// Thus the generated keys are different
		std::cout << "MSG from Server could not be decrypted. Wrong key. " << e.what() << '\n';
	}
	catch (CryptoPP::InvalidKeyLength const &e)
	{
		// Most certainly the final packet was lost and we do not resend.
		// Thus no key was generated.
		std::cout << "MSG from Server could not be decrypted. Key too short. " << e.what() << '\n';
	}
}

void Client::ScheduleUpdateKey(uint32_t in_refreshTime)
{
	Simulator::Schedule(Seconds(in_refreshTime), &Client::UpdateKey, this);
}

void Client::SendEncryptedMessage()
{
	std::ostringstream oss;

	try
	{
		if (m_keyProtocol.compare("ecdh") == 0)
		{
			oss << "MSG,," << AES::GetInstance()->EncryptMessage(m_ecdh->GetSecret(), std::string("CLIENT_TEST_ENCRYPT"));
		}
		else if (m_keyProtocol.compare("nc") == 0)
		{
			oss << "MSG,," << AES::GetInstance()->EncryptMessage(m_nc->GetSecret(), std::string("CLIENT_TEST_ENCRYPT"));
		}
	}
	catch (CryptoPP::InvalidKeyLength const &e)
	{
		// Most certainly the final packet was lost and we do not resend.
		// Thus no key was generated.
		std::cout << "MSG to Server could not be encrypted. Key too short. " << e.what() << '\n';
	}

	SendDataToNodeId(oss.str(), m_serverId);
}

void Client::SendDataToNodeId(std::string data, uint32_t in_nodeId)
{
	if (useIpv6 == true)
	{
		Inet6SocketAddress sendAddress = Inet6SocketAddress(m_v6Intf.GetAddress(in_nodeId, 1), Utils::GetServerPort());
		appSend->SendPacket(data, sendAddress);
	}
	else
	{
		InetSocketAddress sendAddress = InetSocketAddress(m_v4Intf.GetAddress(in_nodeId), Utils::GetServerPort());
		appSend->SendPacket(data, sendAddress);
	}
}

double Client::GetCurrentTime()
{
	return Simulator::Now().GetSeconds();
}