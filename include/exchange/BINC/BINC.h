// BINC.h
#ifndef BINC_H
#define BINC_H

#include <string>
#include <vector>

#include "rapidjson/document.h"
#include "exchange/BINC/API.h"
#include "exchange/BINC/Websock.h"
#include "exchange/CBPR/CBPR.h"


using namespace std;
using namespace rapidjson;

class BINC
{
public:
    BINC();
    BINC(API app_api, vector<string> symbols, string api_key, string secret_key, string uri, string wssURL, string redisURL, string connectorid, string redisConnectorChannel, string redisOrderBookChannel);
    ~BINC();
    /* Declare Variables */
    vector<string> CypherTrust_symbols;
    vector<string> BINC_symbols;
    vector<string> Offline_symbols;
    string  redisURL,
            wssURL,
            connectorID,
            redisConnectorChannel,
            redisOrderBookChannel;

    API app_api;
    BINCAPI api;
    BINCWebsock* sock;
    Util util;
    void run();

    thread th;
    Document currency_data_format();

    void websock();
    void StartStopWebsock(); // to control(start or stop) for websocket 
    void exchangeMonitoring();
    void marketMonitoring();

};

#endif // BINC_H
