#ifndef NC_H
#define NC_H

#include <bits/stdc++.h>


#include "ns3/applications-module.h"
#include "ns3/internet-module.h"

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

#include "Eigen/Dense"

using namespace ns3;

class NC
{
public:
	NC(uint32_t k, uint32_t n, int32_t l, uint32_t roundThreshold, std::string learnRule);
	Eigen::VectorXi GenerateInputVector();
	Eigen::VectorXi VectorFromString(std::string &str_Vector);
	int8_t CalculateOutput(Eigen::VectorXi &inVector); // Calculates the output of the network
	void UpdateWeights(int8_t partnerTau);
	uint32_t GetCurrentRound();
	void GenerateKey(); // maybe not needed as public function
	CryptoPP::SecByteBlock GetSecret();

private:
	uint32_t m_k;
	uint32_t m_n;
	int32_t m_l;
	uint32_t m_roundThreshold;
	std::string m_learnRule;
	Eigen::VectorXi m_inVector;
	Eigen::VectorXi m_weights;
	Eigen::VectorXi m_sigmas;
	uint32_t m_round;
	int8_t m_tau;

	std::mt19937 gen;
	std::uniform_int_distribution<int> uni;

	CryptoPP::SecByteBlock m_secret;

	// Function pointer used to speed up UpdateWeights function
	// Otherwise would need several string compares
	void (NC::*UpdateRule)();

	bool Theta(int tau1, int tau2);
	void EnforceBounds();
	void Hebbian();
	void AntiHebbian();
	void RandomWalk();
};

#endif /* NC_H */
