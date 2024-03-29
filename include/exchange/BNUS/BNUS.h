// BNUS.h
#ifndef BNUS_H
#define BNUS_H

#include <string>
#include <vector>

#include "rapidjson/document.h"
#include "exchange/BNUS/API.h"
#include "exchange/BNUS/Websock.h"
#include "exchange/CBPR/CBPR.h"


using namespace std;
using namespace rapidjson;

class BNUS
{
public:
    BNUS();
    BNUS(API app_api, vector<string> symbols, string api_key, string secret_key, string uri, string wssURL, string redisURL, string connectorid, string redisConnectorChannel, string redisOrderBookChannel, bool *isExitApp);
    ~BNUS();
    /* Declare Variables */
    bool *isExitApp;
    vector<string> CypherTrust_symbols;
    vector<string> BNUS_symbols;
    vector<string> Offline_symbols;
    string  redisURL,
            wssURL,
            connectorID,
            redisConnectorChannel,
            redisOrderBookChannel;

    API app_api;
    BNUSAPI api;
    BNUSWebsock* sock;
    Util util;
    void run();

    thread th;
    Document currency_data_format();

    void websock();
    void StartStopWebsock(); // to control(start or stop) for websocket 
    void exchangeMonitoring();
    void marketMonitoring();

};

#endif // BNUS_H
