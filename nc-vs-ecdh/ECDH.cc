#include "ECDH.h"

ECDH::ECDH()
{
	CryptoPP::OID CURVE = CryptoPP::ASN1::secp256r1();
	CryptoPP::ECDH<CryptoPP::ECP>::Domain ecdh(CURVE);
	m_ecdh = ecdh;
	CryptoPP::SecByteBlock priv(m_ecdh.PrivateKeyLength());
	CryptoPP::SecByteBlock pub(m_ecdh.PublicKeyLength());
	m_privateKey = priv;
	m_publicKey = pub;
}

void ECDH::GenerateKeyPair()
{
	CryptoPP::AutoSeededRandomPool rng;
	m_ecdh.GenerateKeyPair(rng, m_privateKey, m_publicKey);
}

CryptoPP::Integer ECDH::GetPublicKey()
{
	CryptoPP::Integer publicKey;
	publicKey.Decode(m_publicKey.BytePtr(), m_publicKey.SizeInBytes());
	return publicKey;
}

void ECDH::GenerateSecret(CryptoPP::Integer *pub)
{
	CryptoPP::SecByteBlock secPub(pub->MinEncodedSize());
	pub->Encode(secPub, secPub.size());
	CryptoPP::SecByteBlock secret(m_ecdh.AgreedValueLength());
	m_ecdh.Agree(secret, m_privateKey, secPub);
	m_secret = secret;
	std::cout << "Secret size: " << m_secret.size() << "\n";
}

CryptoPP::SecByteBlock ECDH::GetSecret()
{
	return m_secret;
}
