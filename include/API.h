/* CypherTrust */

// Api.h
#ifndef API_H
#define API_H

#include <string>
#include <vector>

using namespace std;

class API
{
private:
    string Call(string method, bool authed, string path, string body);

public:
    API();
    ~API();
    string url;
    string token;

    string user;
    string key;
    string password;

    string auth();
    bool del_address(string address_id);
    long ping(string address_id);
    void StartSession(string address_id, string market);
    void StopSession(string address_id, string market);


};



#endif // API_H
