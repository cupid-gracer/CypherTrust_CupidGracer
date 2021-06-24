// EXMO.h
#ifndef EXMO_H
#define EXMO_H

#include <string>
#include <vector>

#include "rapidjson/document.h"
#include "exchange/EXMO/API.h"
#include "exchange/EXMO/Websock.h"

using namespace std;
using namespace rapidjson;

class EXMO
{
public:
    EXMO();
    EXMO(API app_api, vector<string> symbols, string api_key, string secret_key, string uri, string wssURL, string redisURL, string connectorid, string redisConnectorChannel, string redisOrderBookChannel, bool *isExitApp);
    ~EXMO();
    /* Declare Variables */
    vector<string> CypherTrust_symbols;
    vector<string> EXMO_symbols;
    vector<string> Offline_symbols;

    bool *isExitApp;
    string  redisURL,
            wssURL,
            connectorID,
            redisConnectorChannel,
            redisOrderBookChannel;

    API app_api;
    EXMOAPI api;
    EXMOWebsock* sock;
    Util util;
    void run();

    thread th;
    Document currency_data_format();

    void websock();
    void StartStopWebsock(); // to control(start or stop) for websocket 
    void exchangeMonitoring();
    void marketMonitoring();

};

#endif // EXMO_H
