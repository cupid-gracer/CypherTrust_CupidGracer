// CBPR.h
#ifndef CBPR_H
#define CBPR_H

#include "exchange/CBPR/API.h"
#include <string>
#include <vector>

using namespace std;

class CBPR
{
public:
    CBPR(vector<string> coin_included, string api_uri, string api_key, string secret, string passcode, string redisurl, string connectorID);
    ~CBPR();
    /* Declare Variables */
    string uri;
    string product_id;
    string redisURL;
    string ConnectorID;
    CBPRAPI api;

    vector<string> myCoinList;

    void run();

    void websock();


};

#endif // CBPR_H
