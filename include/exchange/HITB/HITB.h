// HITB.h
#ifndef HITB_H
#define HITB_H

#include <string>
#include <vector>

#include "rapidjson/document.h"
#include "exchange/HITB/API.h"
#include "exchange/HITB/Websock.h"
#include "exchange/CBPR/CBPR.h"


using namespace std;
using namespace rapidjson;

class HITB
{
public:
    HITB();
    HITB( vector<string> coin_included, string api_key, string secret_key, string uri, string wssURL, string redisURL, string connectorid, string redisConnectorChannel, string redisOrderBookChannel);
    ~HITB();
    /* Declare Variables */
    vector<string> myCoinList;
    string  redisURL,
            wssURL,
            connectorID,
            redisConnectorChannel,
            redisOrderBookChannel;

    HITBAPI api;
    HITBWebsock sock;

    void run();

    Document currency_data_format();

    void websock();
    void StartStopWebsock(); // to control(start or stop) for websocket 


};

#endif // HITB_H
