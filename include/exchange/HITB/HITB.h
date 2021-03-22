// HITB.h
#ifndef HITB_H
#define HITB_H

#include <string>
#include <vector>

#include "rapidjson/document.h"
#include "exchange/HITB/API.h"

using namespace std;
using namespace rapidjson;

class HITB
{
public:
    HITB(vector<string> coin_included, string api_key, string secret_key, string uri, string wssURL, string redisURL, string connectorid, string redisConnectorChannel, string redisOrderBookChannel);
    ~HITB();
    /* Declare Variables */
    vector<string> myCoinList;
    string  redisURL,
            wssURL,
            connectorID,
            redisConnectorChannel,
            redisOrderBookChannel;

    HITBAPI api;

    void run();

    Document currency_data_format();

    void websock();

};

#endif // HITB_H
