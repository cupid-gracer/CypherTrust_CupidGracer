// App.h
#ifndef APP_H
#define APP_H

#include <string>
#include <vector>
#include "API.h"

using namespace std;

class App
{

public:
    App(string con_setting_str, string type);
    ~App();

    API api;
    string uri;
    /* Variable Declare */
    // bootstrap variables
    string  redisManagementChannel,
            redisHeartbeatChannel,
            redisOrderBookChannel,
            redisHost,
            logHost,
            walletName,
            exchangeSecret,
            exchangePassword,
            exchangeKey,
            exchangeApiUrl,
            exchangeWsUrl,
            exchangeRedisOrderChannel,
            portfolioName,
            connectorID,
            expiration,
            redisURL,
            address_id,
            scope;
    int redisPort, logPort;
    bool walletEnabled;
    vector<string> coin_included;
    string type;

    void auth(string user, string password, string type);

    void redisMan();
    void setGlobalValue(string res);

    void run();

};



#endif // APP_H
