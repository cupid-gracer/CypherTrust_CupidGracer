// Auth.h
#ifndef AUTH_H
#define AUTH_H

#include <string>

using namespace std;

class Auth
{
public:
  string Key;
  string Secret;
  string Passphrase;
  string Sign(string time_stamp, string method, string path, string body);
  string GetTimestamp();

  Auth() {}
  Auth(string key, string secret, string passphrase);
};

#endif // AUTH_H
