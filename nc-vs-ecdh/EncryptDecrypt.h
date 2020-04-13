#ifndef ENCRYPTDECRYPT_H
#define ENCRYPTDECRYPT_H

#include <iostream> 
#include <deque>
#include <bits/stdc++.h>

#include "ns3/applications-module.h"

#include "cryptopp/nbtheory.h"
#include "cryptopp/cryptlib.h"
#include "cryptopp/integer.h"
#include "cryptopp/osrng.h"
#include "cryptopp/eccrypto.h"
#include "cryptopp/secblock.h"
#include "cryptopp/oids.h"
#include "cryptopp/asn.h"
#include "cryptopp/hex.h"
#include "cryptopp/aes.h"
#include "cryptopp/base64.h"
#include "cryptopp/filters.h"
#include "cryptopp/modes.h"


using namespace ns3;

class AES {
public:
	static AES* GetInstance();
	std::string EncryptMessage(CryptoPP::SecByteBlock key, std::string data);
	std::string DecryptMessage(CryptoPP::SecByteBlock key, std::string data);
private:
	AES();
	static AES* aes;
};

#endif /* ENCRYPTDECRYPT_H */

