#include "Utils.h"

uint32_t Utils::m_refreshTime = 0;
uint32_t Utils::m_startTime = 0;
uint32_t Utils::m_stopTime = 0;
uint32_t Utils::m_maxRound = 0;
uint32_t Utils::m_port = 0;
uint32_t Utils::m_msgTimeOut = 3;
uint32_t Utils::m_rngSeed = 0;
std::string Utils::m_keyProtocol = "";
std::string Utils::m_dataRate = "";
std::string Utils::m_delay = "";

Utils::Utils() {}

CryptoPP::Integer Utils::GetRandom(CryptoPP::Integer min, CryptoPP::Integer max)
{
	CryptoPP::Integer g;
	CryptoPP::AutoSeededRandomPool *rng = new CryptoPP::AutoSeededRandomPool();
	g.Randomize(*rng, min, max);
	return g;
}

void Utils::SetRngSeed(uint32_t rngSeed)
{
	m_rngSeed = rngSeed;
}

uint32_t Utils::GetRngSeed()
{
	return m_rngSeed;
}

void Utils::SetRefreshTime(uint32_t refreshTime)
{
	m_refreshTime = refreshTime;
}

uint32_t Utils::GetRefreshTime()
{
	return m_refreshTime;
}

void Utils::SetStartTime(uint32_t startTime)
{
	m_startTime = startTime;
}

uint32_t Utils::GetStartTime()
{
	return m_startTime;
}

void Utils::SetStopTime(uint32_t stopTime)
{
	m_stopTime = stopTime;
}

uint32_t Utils::GetStopTime()
{
	return m_stopTime;
}

void Utils::SetMaxRound(uint32_t in_maxRound)
{
	m_maxRound = in_maxRound;
}

uint32_t Utils::GetMaxRound()
{
	return m_maxRound;
}

void Utils::SetPort(uint32_t port)
{
	m_port = port;
}
uint32_t Utils::GetPort()
{
	return m_port;
}

uint32_t Utils::GetClientPort()
{
	return (m_port + 1);
}

uint32_t Utils::GetServerPort()
{
	return m_port;
}

uint32_t Utils::GetMsgTimeOut()
{
	return m_msgTimeOut;
}

void Utils::SetKeyProtocol(std::string keyProtocol)
{
	m_keyProtocol = keyProtocol;
}

std::string Utils::GetKeyProtocol()
{
	return m_keyProtocol;
}

void Utils::SetDataRate(std::string dataRate)
{
	m_dataRate = dataRate;
}

std::string Utils::GetDataRate()
{
	return m_dataRate;
}

void Utils::SetDelay(std::string delay)
{
	m_delay = delay;
}

std::string Utils::GetDelay()
{
	return m_delay;
}