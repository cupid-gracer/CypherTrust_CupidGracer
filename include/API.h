/* CypherTrust */

// Api.h
#ifndef API_H
#define API_H

#include <string>
#include <vector>
#include "Util.h"

using namespace std;

class API
{
private:
    string Call(string method, bool authed, string path, string body);

public:
    API();
    ~API();
    Util util;
    
    string url;
    string token;

    string user;
    string key;
    string password;
    string type;

    string auth();
    bool del_address(string address_id);
    string ping(string address_id);
    void StartSession(string address_id, string market, bool snapshot, string context, string classType);
    void StopSession(string address_id, string market);
    void LoggingEvent(string address_id, string clss, string context, string text);


};



#endif // API_H
