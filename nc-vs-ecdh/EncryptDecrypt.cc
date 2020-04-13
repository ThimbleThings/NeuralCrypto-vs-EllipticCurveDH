#include "EncryptDecrypt.h"

AES* AES::aes = NULL;
AES* AES::GetInstance() {
	if (!aes) {
		aes = new AES();
	}
	return aes;
}

AES::AES() {
}

std::string AES::EncryptMessage(CryptoPP::SecByteBlock key, std::string data) {
	CryptoPP::AutoSeededRandomPool rnd;
	byte iv[CryptoPP::AES::BLOCKSIZE];
	rnd.GenerateBlock(iv, CryptoPP::AES::BLOCKSIZE);

	std::string cipher;
	CryptoPP::StringSink* sink = new CryptoPP::StringSink(cipher);
	CryptoPP::Base64Encoder* base64_enc = new CryptoPP::Base64Encoder(sink);
	CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption cbc_enc(key, key.size(), iv);
	CryptoPP::StreamTransformationFilter* enc = new CryptoPP::StreamTransformationFilter(cbc_enc, base64_enc);
	CryptoPP::StringSource source(data, true, enc);

	std::ostringstream ss;

	CryptoPP::Integer* i_iv = new CryptoPP::Integer(iv, CryptoPP::AES::BLOCKSIZE);
	ss << *i_iv << "," << cipher << ",t";
	return ss.str();
}

std::string AES::DecryptMessage(CryptoPP::SecByteBlock key, std::string ivData) {
	CryptoPP::AutoSeededRandomPool rnd;
	std::istringstream iss(ivData);

	std::string str_iv;
	getline(iss, str_iv, ',');
	CryptoPP::Integer* i_iv = new CryptoPP::Integer(str_iv.c_str());

	std::string str_data;
	getline(iss, str_data, ',');
	std::string cipher = str_data;

	byte iv[CryptoPP::AES::BLOCKSIZE];
	i_iv->Encode(iv, CryptoPP::AES::BLOCKSIZE);

	std::string data;
	CryptoPP::StringSink* sink = new CryptoPP::StringSink(data);
	CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption cbc_dec(key, key.size(), iv);
	CryptoPP::StreamTransformationFilter* dec = new CryptoPP::StreamTransformationFilter(cbc_dec, sink);
	CryptoPP::Base64Decoder* base64_dec = new CryptoPP::Base64Decoder(dec);
	CryptoPP::StringSource source(cipher, true, base64_dec);
	return data;
}
