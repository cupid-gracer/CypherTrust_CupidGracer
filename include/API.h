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
    string auth_url;

    string auth(string user, string password, string key);
    


};



#endif // API_H
