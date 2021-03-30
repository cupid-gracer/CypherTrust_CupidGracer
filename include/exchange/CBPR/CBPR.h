// CBPR.h
#ifndef CBPR_H
#define CBPR_H

#include <string>
#include <vector>
#include "exchange/CBPR/API.h"
#include "exchange/CBPR/Websock.h"

using namespace std;

class CBPR
{
public:
    CBPR();
    CBPR(API app_api, vector<string> symbols, string api_uri, string api_key, string secret, string passcode, string exchangeWsUrl, string redisurl, string connectorID, string  redisConnectorChannel, string redisOrderBookChannel);
    ~CBPR();
    /* Declare Variables */
    string uri;
    string product_id;
    string redisURL;
    string ConnectorID,
           wssURL,
           redisConnectorChannel, 
           redisOrderBookChannel;

    vector<string> CypherTrust_symbols;
    vector<string> Offline_symbols;

    API app_api;
    CBPRAPI api;
    Util util;
    CBPRWebsock* sock;
    thread th ;

    vector<string> symbols;

    void run();

    void websock();
    void StartStopWebsock(); // to control(start or stop) for websocket 
    void exchangeMonitoring();
    void marketMonitoring();



};

#endif // CBPR_H
