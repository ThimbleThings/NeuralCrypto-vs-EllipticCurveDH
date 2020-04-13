#include "Logger.h"
#include "Utils.h"
#include <boost/algorithm/string.hpp>

Logger *Logger::log = NULL;

constexpr char Logger::hexmap[];

Logger *Logger::GetInstance()
{
	if (!log)
	{
		log = new Logger();
	}
	return log;
}

Logger::Logger()
{
	std::ostringstream ss;
	std::ostringstream ss_dir;
	ss << Utils::GetKeyProtocol() << "_DataRate" << Utils::GetDataRate()
	   << "_delay" << Utils::GetDelay() << "_Refr" << Utils::GetRefreshTime() << "_Seed" << Utils::GetRngSeed();
	ss_dir << Utils::GetKeyProtocol() << "_DataRate" << Utils::GetDataRate()
		   << "_delay" << Utils::GetDelay() << "_Refr" << Utils::GetRefreshTime();

	boost::filesystem::create_directory("Simulations/" + ss_dir.str());
	//packetTxRxFile.push(boost::iostreams::gzip_compressor());
	packetTxRxFile.push(boost::iostreams::file_sink("Simulations/" + ss_dir.str() + "/PacketTxRx_" + ss.str() + ".csv", std::ios::binary));
	// Write the CSV header
	packetTxRxFile << "Time,PacketID,NodeID,EventType,Size,PacketType,PacketState,Comment\n";

	keyExchangeFile.push(boost::iostreams::file_sink("Simulations/" + ss_dir.str() + "/KeyExchange_" + ss.str() + ".csv", std::ios::binary));
	// Write the CSV header
	keyExchangeFile << "Time,NodeID,Key\n";
}

void Logger::TraceReceivePacket(Ptr<const Packet> packet, uint32_t nodeId)
{
	InsertParameters params;
	params = MakePacketParameters(packet, params, nodeId, "receive");
	params.packet_event.event_type = CONSUME_DATA_PACKET;
	DoDatabaseInsert(params);
}

void Logger::TraceTransmitPacket(Ptr<const Packet> packet, uint32_t nodeId)
{
	InsertParameters params;
	params = MakePacketParameters(packet, params, nodeId, "transmit");
	params.packet_event.event_type = CREATE_DATA_PACKET;
	DoDatabaseInsert(params);
}

Logger::InsertParameters Logger::MakePacketParameters(Ptr<const Packet> packet, InsertParameters params, uint32_t nodeId, std::string comment)
{
	params.time = Simulator::Now().GetSeconds();
	params.packet_event.packet_id = packet->GetUid();
	params.packet_event.node_id = nodeId;
	params.packet_event.size = packet->GetSize();
	std::stringstream packetData;
	packet->CopyData(&packetData, packet->GetSize());

	std::vector<std::string> strs;
	std::string test = packetData.str();
	boost::split(strs, test, boost::is_any_of(","));

	params.packet_event.packetType = strs[0];
	params.packet_event.packetState = strs[1];
	params.packet_event.comment = comment;
	return params;
}

void Logger::DoDatabaseInsert(const InsertParameters &params)
{
	std::stringstream dateStringStream;
	dateStringStream.str("");
	dateStringStream << params.time;
	std::string timeStr = dateStringStream.str();

	std::string packetIdStr = boost::lexical_cast<std::string>(params.packet_event.packet_id);
	std::string eventTypeStr = boost::lexical_cast<std::string>(params.packet_event.event_type);
	std::string nodeIdStr = boost::lexical_cast<std::string>(params.packet_event.node_id);
	std::string sizeStr = boost::lexical_cast<std::string>(params.packet_event.size);

	packetTxRxFile << timeStr << "," << packetIdStr << "," << nodeIdStr << "," << eventTypeStr << ","
					<< sizeStr << "," << params.packet_event.packetType << "," << params.packet_event.packetState << ",";

	if (params.packet_event.comment.size())
	{
		packetTxRxFile << params.packet_event.comment << "\n";
	}
	else
	{
		packetTxRxFile << "\\N"
						<< "\n";
	}
}

void Logger::LogKey(uint32_t nodeId, CryptoPP::SecByteBlock key)
{
	std::stringstream dateStringStream;
	dateStringStream.str("");
	dateStringStream << Simulator::Now().GetSeconds();
	std::string timeStr = dateStringStream.str();

	keyExchangeFile << timeStr << "," << boost::lexical_cast<std::string>(nodeId) << ","
					<< hexStr(key.data(), key.size()) << std::endl;
}

std::string Logger::hexStr(unsigned char *data, int len)
{
	std::string s(len * 2, ' ');
	for (int i = 0; i < len; ++i)
	{
		s[2 * i] = hexmap[(data[i] & 0xF0) >> 4];
		s[2 * i + 1] = hexmap[data[i] & 0x0F];
	}
	return s;
}

void Logger::End()
{
	packetTxRxFile.pop();
}