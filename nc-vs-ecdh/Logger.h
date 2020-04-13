#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>

#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>

#include "cryptopp/integer.h"

#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/olsr-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/olsr-routing-protocol.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-routing-table-entry.h"

#include <bits/stdc++.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>

using namespace ns3;

class Logger
{
public:
	static Logger *GetInstance();
	void End();
	void TraceReceivePacket(Ptr<const Packet> packet, uint32_t nodeId);
	void TraceTransmitPacket(Ptr<const Packet> packet, uint32_t nodeId);
	void LogKey(uint32_t nodeId, CryptoPP::SecByteBlock key);

private:
	enum
	{
		CREATE_DATA_PACKET = 1,
		CONSUME_DATA_PACKET = 2
	};

	struct PacketEventParameters
	{
		uint32_t packet_id;
		uint32_t event_type;
		uint16_t node_id;
		std::string packetType;
		std::string packetState;
		std::string comment;
		uint32_t size;
	};
	struct InsertParameters
	{
		double time;
		PacketEventParameters packet_event;
	};

	static constexpr char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7',
									  '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

	static Logger *log;
	std::deque<InsertParameters> insertsRxTx;
	boost::iostreams::filtering_ostream packetTxRxFile;
	boost::iostreams::filtering_ostream keyExchangeFile;

	Logger();
	InsertParameters MakePacketParameters(Ptr<const Packet> packet, InsertParameters params, uint32_t nodeId, std::string comment);
	void DoDatabaseInsert(const InsertParameters &params);
	std::string hexStr(unsigned char *data, int len);
};

#endif /* LOGGER_H */
