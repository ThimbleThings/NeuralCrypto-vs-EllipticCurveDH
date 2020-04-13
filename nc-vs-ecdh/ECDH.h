#ifndef ECDH_H
#define ECDH_H

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

using namespace ns3;

class ECDH
{
public:
	ECDH();
	void GenerateKeyPair();
	CryptoPP::Integer GetPublicKey();
	void GenerateSecret(CryptoPP::Integer *pub);
	CryptoPP::SecByteBlock GetSecret();

private:
	CryptoPP::SecByteBlock m_publicKey;
	CryptoPP::SecByteBlock m_privateKey;
	CryptoPP::ECDH<CryptoPP::ECP>::Domain m_ecdh;
	CryptoPP::SecByteBlock m_secret;
	uint32_t m_gen_round;
};

#endif /* ECDH_H */
