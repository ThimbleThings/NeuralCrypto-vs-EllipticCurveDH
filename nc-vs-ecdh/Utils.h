#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <deque>
#include <string>
#include <bits/stdc++.h>
#include <vector>

#include "cryptopp/nbtheory.h"
#include "cryptopp/cryptlib.h"
#include "cryptopp/integer.h"
#include "cryptopp/osrng.h"
#include "cryptopp/eccrypto.h"
#include "cryptopp/secblock.h"
#include "cryptopp/oids.h"
#include "cryptopp/asn.h"
#include "cryptopp/hex.h"
#include "cryptopp/filters.h"
#include "cryptopp/modes.h"

#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-routing-table-entry.h"

using namespace ns3;

#define DEFAULT_PORT_NR 4477

class Utils
{
public:
	Utils();
	static CryptoPP::Integer GetRandom(CryptoPP::Integer min, CryptoPP::Integer max);
	static void SetRngSeed(uint32_t rngSeed);
	static uint32_t GetRngSeed();
	static void SetRefreshTime(uint32_t refreshTime);
	static uint32_t GetRefreshTime();
	static void SetStartTime(uint32_t startTime);
	static uint32_t GetStartTime();
	static void SetStopTime(uint32_t stopTime);
	static uint32_t GetStopTime();
	static void SetMaxRound(uint32_t in_maxRound);
	static uint32_t GetMaxRound();
	static void SetPort(uint32_t port);
	static uint32_t GetPort();
	static uint32_t GetClientPort();
	static uint32_t GetServerPort();
	static uint32_t GetMsgTimeOut();
	static void SetKeyProtocol(std::string keyProtocol);
	static std::string GetKeyProtocol();
	static void SetDataRate(std::string dataRate);
	static std::string GetDataRate();
	static void SetDelay(std::string delay);
	static std::string GetDelay();

private:
	static uint32_t m_refreshTime;
	static uint32_t m_startTime;
	static uint32_t m_stopTime;
	static uint32_t m_maxRound;
	static uint32_t m_port;
	static uint32_t m_msgTimeOut;
	static uint32_t m_rngSeed;
	static std::string m_keyProtocol;
	static std::string m_dataRate;
	static std::string m_delay;
};

#endif /* UTILS_H */
