// Auth.cpp
#include "exchange/CBPR/Auth.h"
#include <iostream>
#include "cryptopp/cryptlib.h"
#include "cryptopp/hmac.h"
#include "cryptopp/sha.h"
#include "cryptopp/base64.h"
#include "cryptopp/filters.h"


using namespace std;
using CryptoPP::Exception;
using CryptoPP::HMAC;
using CryptoPP::SHA256;
using CryptoPP::Base64Decoder;
using CryptoPP::Base64Encoder;
using CryptoPP::StringSink;
using CryptoPP::StringSource;
using CryptoPP::HashFilter;

string Auth::Sign(string time_stamp, string method, string path, string body)
{
  string mac, encoded, key;
  string plain = time_stamp + method + path + body;
  StringSource(Secret, true,
		  new Base64Decoder(
			  new StringSink(key)));
  try
  {
	  HMAC<SHA256> hmac((unsigned char*)key.c_str(), key.length());
	  StringSource(plain, true,
			  new HashFilter(hmac,
				  new StringSink(mac)));
  }
  catch(const CryptoPP::Exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
  StringSource(mac, true,
		  new Base64Encoder(
			  new StringSink(encoded)));
  encoded.erase(44, 1);
  return encoded;
}

string Auth::GetTimestamp()
{
  time_t t = time(0);
  return std::to_string(t);
}

Auth::Auth(string key, string secret, string passphrase)
{ Key = key; Secret = secret; Passphrase = passphrase; }
