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
    App(string con_setting_str);
    ~App();

    API api;
    string uri;
    /* Variable Declare */
    // bootstrap variables
    string  redisManagementChannel,
            redisHeartbeatChannel,
            redisOrderBookChannel,
            redisHost,
            walletName,
            exchangeSecret,
            exchangePassword,
            exchangeKey,
            exchangeApiUrl,
            exchangeWsUrl,
            exchangeRedisOrderChannel,
            portfolioName,
            address_id,
            expiration;
    int redisPort;
    bool walletEnabled;
    vector<string> coin_included;

    void auth(string user, string password, string type);

    void redisMan();
    void setGlobalValue(string res);


};



#endif // APP_H
