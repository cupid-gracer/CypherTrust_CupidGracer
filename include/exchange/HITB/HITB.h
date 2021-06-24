// HITB.h
#ifndef HITB_H
#define HITB_H

#include <string>
#include <vector>

#include <json/value.h>

#include "exchange/HITB/API.h"
#include "exchange/HITB/Websock.h"
#include "exchange/CBPR/CBPR.h"


using namespace std;
using namespace rapidjson;

class HITB
{
public:
    HITB();
    HITB(API app_api, vector<string> symbols, string api_key, string secret_key, string uri, string wssURL, string redisURL, string connectorid, string redisConnectorChannel, string redisOrderBookChannel, bool *isExitApp);
    ~HITB();
    /* Declare Variables */
    bool *isExitApp;
    vector<string> CypherTrust_symbols;
    vector<string> HITB_symbols;
    vector<string> Offline_symbols;
    string  redisURL,
            wssURL,
            connectorID,
            redisConnectorChannel,
            redisOrderBookChannel;

    API app_api;
    HITBAPI api;
    HITBWebsock* sock;
    Util util;
    void run();

    thread th;
    Json::Value currency_data_format();

    void websock();
    void StartStopWebsock(); // to control(start or stop) for websocket 
    void exchangeMonitoring();
    void marketMonitoring();

};

#endif // HITB_H
