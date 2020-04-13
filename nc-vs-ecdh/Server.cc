#include "Server.h"
#include "Utils.h"
#include "EncryptDecrypt.h"
#include "Logger.h"

// Same definition as for server, since including through a header throws initialization errors
Eigen::IOFormat ServerFmt(Eigen::FullPrecision, Eigen::DontAlignCols, ";", ";", "", "", "", "");

Server::Server(uint32_t serverId, NodeContainer nodeContainer, Ipv4InterfaceContainer v4Intf,
			   std::string keyProtocol, uint32_t clientId)
{
	useIpv6 = false;
	m_serverId = serverId;
	m_nodeContainer = nodeContainer;
	m_v4Intf = v4Intf;
	m_keyProtocol = keyProtocol;
	m_clientId = clientId;
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

Server::Server(uint32_t serverId, NodeContainer nodeContainer, Ipv6InterfaceContainer v6Intf,
			   std::string keyProtocol, uint32_t clientId)
{
	useIpv6 = true;
	m_serverId = serverId;
	m_nodeContainer = nodeContainer;
	m_v6Intf = v6Intf;
	m_keyProtocol = keyProtocol;
	m_clientId = clientId;
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

Server::~Server()
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

void Server::Setup()
{

	appRecv = CreateObject<ReceiverApplication>();
	appSender = CreateObject<SenderApplication>();

	//SETUP SENDER
	appSender->Setup(m_serverId, m_nodeContainer);
	appSender->TraceConnectWithoutContext("AppTx", MakeBoundCallback(&Server::SendPacketCallBack, this));
	m_nodeContainer.Get(m_serverId)->AddApplication(appSender);
	appSender->SetStartTime(Seconds(Utils::GetStartTime()));
	appSender->SetStopTime(Seconds(Utils::GetStopTime()));

	//SETUP RECEIVER
	Ptr<Socket> socketRecvUdp = Socket::CreateSocket(m_nodeContainer.Get(m_serverId), UdpSocketFactory::GetTypeId());
	appRecv->TraceConnectWithoutContext("AppRx", MakeBoundCallback(&Server::ReceivePacketCallBack, this));

	if (useIpv6 == true)
	{
		Inet6SocketAddress recvAddress = Inet6SocketAddress(m_v6Intf.GetAddress(m_serverId, 1), Utils::GetServerPort());
		socketRecvUdp->Bind(recvAddress);
	}
	else
	{
		InetSocketAddress recvAddress = InetSocketAddress(m_v4Intf.GetAddress(m_serverId), Utils::GetServerPort());
		socketRecvUdp->Bind(recvAddress);
	}

	m_nodeContainer.Get(m_serverId)->AddApplication(appRecv);
	appRecv->Setup(socketRecvUdp, m_serverId);
	appRecv->SetStartTime(Seconds(Utils::GetStartTime()));
	appRecv->SetStopTime(Seconds(Utils::GetStopTime()));
}

void Server::SendPacketCallBack(Server *server, Ptr<const Packet> packet)
{
	//MEASUREMENT
	Logger::GetInstance()->TraceTransmitPacket(packet, server->GetServerId());
}

void Server::ReceivePacketCallBack(Server *server, Ptr<const Packet> packet)
{
	//MEASUREMENT
	Logger::GetInstance()->TraceReceivePacket(packet, server->GetServerId());
	server->ReadPacket(packet);
}

uint32_t Server::GetServerId()
{
	return m_serverId;
}

void Server::ReadPacket(Ptr<const Packet> packet)
{
	std::cout << "Server got packet at time:" << GetCurrentTime() << "\n";

	if (packet->GetSize() <= 0)
	{
		return;
	}

	std::stringstream packetData;
	packet->CopyData(&packetData, packet->GetSize());

	// Extract first field from Dataframe format
	std::string str_purpose;
	std::getline(packetData, str_purpose, ','); // Separates the Purpose from Data

	// Based on the fields value either update ECDH, update NC, or decrypt the MSG
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
		std::getline(packetData, str_data); // Gets the remaining data
		DecryptMSG(str_data);
	}
	else
	{
		std::cout << "Message purpose unknown: " << str_purpose << "\n";
	}
}

void Server::UpdateECDH(std::string data)
{
	std::cout << "Server UpdateECDH: " << data << "\n";

	// We use delimiter '.', since somehow we sometimes read garbage otherwise...
	boost::char_separator<char> sep(",");
	boost::tokenizer<boost::char_separator<char>> tokens(data, sep);
	boost::tokenizer<boost::char_separator<char>>::iterator beg = tokens.begin();
	std::string str_state = *beg;

	if (str_state.compare("BEGIN") != 0)
	{
		std::cout << "Unknown ECDH state: " << str_state << "\n";
		return;
	}

	beg++;
	std::string str_publicKey = *beg;
	CryptoPP::Integer *publicKeySender = new CryptoPP::Integer(str_publicKey.c_str());

	if (m_ecdh != NULL)
	{
		delete m_ecdh;
	}

	m_ecdh = new ECDH();
	m_ecdh->GenerateKeyPair();
	m_ecdh->GenerateSecret(publicKeySender);

	// Once the secret is generated the publicKeySender is not needed anymore
	delete publicKeySender;

	// Log the key generated by the client
	Logger::GetInstance()->LogKey(GetServerId(), m_ecdh->GetSecret());

	std::ostringstream oss;
	oss.str("");
	oss << "ECDH,FINAL," << m_ecdh->GetPublicKey() << ",";
	SendDataToNodeId(oss.str(), m_clientId);
}

void Server::UpdateNC(std::string data)
{
	//std::cout << "Server UpdateNC: " << data << "\n";

	// We use delimiter ',', since somehow we sometimes read garbage otherwise...
	boost::char_separator<char> sep(",");
	boost::tokenizer<boost::char_separator<char>> tokens(data, sep);
	boost::tokenizer<boost::char_separator<char>>::iterator beg = tokens.begin();
	std::string str_state = *beg;

	if (str_state.compare("BEGIN") == 0)
	{
		if (m_nc != NULL)
		{
			delete m_nc;
		}

		m_nc = new NC(m_k, m_n, m_l, m_roundThreshold, std::string("hebbian"));

		beg++;
		std::string str_inVector = *beg;
		// Transform the string into the Eigen::Vector
		Eigen::VectorXi inVector = m_nc->VectorFromString(str_inVector);

		beg++;
		int8_t partnerTau = std::stoi(*beg);

		// Calculate the tau for the given input vector
		int8_t lastTau = m_nc->CalculateOutput(inVector);

		// Update the weights using the partnerTau
		m_nc->UpdateWeights(partnerTau);

		// Generate a new input and calculate output bit
		inVector = m_nc->GenerateInputVector();
		int8_t tau = m_nc->CalculateOutput(inVector);

		// Send the input vector, the output bit and the output bit for the last input to the partner TPM
		std::ostringstream oss;
		oss << "NC,UPDATE," << inVector.format(ServerFmt) << "," << int(tau) << "," << int(lastTau); // conversion to print properly
		//std::cout << "Server will send: " << oss.str() << "\n";
		SendDataToNodeId(oss.str(), m_clientId);
	}
	else if (str_state.compare("UPDATE") == 0)
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
			oss << "NC,UPDATE," << inVector.format(ServerFmt) << "," << int(tau) << "," << int(lastTau); // conversion to print properly
			//std::cout << "Server will send: " << oss.str() << "\n";
			SendDataToNodeId(oss.str(), m_clientId);
		}
		// Otherwise create a final package
		else
		{
			// Generate a zero vector
			inVector = Eigen::VectorXi::Zero(m_n * m_k);

			// Send the zero vector, and the output bit for the last input to the partner TPM
			std::ostringstream oss;
			oss << "NC,FINAL," << inVector.format(ServerFmt) << "," << int(lastTau) << "," << int(lastTau); // conversion to print properly
			std::cout << "Server will send: " << oss.str() << "\n";
			SendDataToNodeId(oss.str(), m_clientId);

			// Generate the key that will be used to decrypt and encrypt a message
			m_nc->GenerateKey();
			// Log the key generated by the server
			Logger::GetInstance()->LogKey(GetServerId(), m_nc->GetSecret());
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
		// Log the key generated by the server
		Logger::GetInstance()->LogKey(GetServerId(), m_nc->GetSecret());
	}
	else
	{
		std::cout << "Unknown NC state: " << str_state << "\n";
		return;
	}
}

void Server::DecryptMSG(std::string data)
{
	//std::cout << "Server DecryptMSG: " << data << "\n";
	// We use delimiter '.', since somehow otherwise we sometimes read garbage...

	std::ostringstream oss;

	try
	{
		if (m_keyProtocol.compare("ecdh") == 0)
		{
			std::cout << "Decrypted Message: " << AES::GetInstance()->DecryptMessage(m_ecdh->GetSecret(), data) << "\n";

			oss << "MSG,," << AES::GetInstance()->EncryptMessage(m_ecdh->GetSecret(), std::string("SERVER_TEST_ENCRYPT"));
		}
		else if (m_keyProtocol.compare("nc") == 0)
		{
			std::cout << "Decrypted Message: " << AES::GetInstance()->DecryptMessage(m_nc->GetSecret(), data) << "\n";

			oss << "MSG,," << AES::GetInstance()->EncryptMessage(m_nc->GetSecret(), std::string("SERVER_TEST_ENCRYPT"));
		}

		// Do not need to send message to client if this decryption already fails
		SendDataToNodeId(oss.str(), m_clientId);
	}
	catch (CryptoPP::InvalidCiphertext const &e)
	{
		// Most certainly the synchronization was not successful within the given number of rounds.
		// Thus the generated keys are different
		std::cout << "MSG from Client could not be decrypted. Wrong key. " << e.what() << '\n';
	}
	catch (CryptoPP::InvalidKeyLength const &e)
	{
		// Most certainly the final packet was lost and we do not resend.
		// Thus no key was generated.
		std::cout << "MSG from Client could not be decrypted. Key too short. " << e.what() << '\n';
	}
}

void Server::SendDataToNodeId(std::string data, uint32_t in_nodeId)
{
	if (useIpv6 == true)
	{
		Inet6SocketAddress sendAddress = Inet6SocketAddress(m_v6Intf.GetAddress(in_nodeId, 1), Utils::GetClientPort());
		appSender->SendPacket(data, sendAddress);
	}
	else
	{
		InetSocketAddress sendAddress = InetSocketAddress(m_v4Intf.GetAddress(in_nodeId), Utils::GetClientPort());
		appSender->SendPacket(data, sendAddress);
	}
}

double Server::GetCurrentTime()
{
	return Simulator::Now().GetSeconds();
}