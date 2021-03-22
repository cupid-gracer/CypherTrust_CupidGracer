// App.h
#ifndef APP_H
#define APP_H

#include <string>
#include <vector>
#include "API.h"
#include "Util.h"

using namespace std;

class App
{

public:

    App(string con_setting_str, string type, string scope);
    ~App();

    API api;
    string uri;
    Util util;
    /* Variable Declare */
    // bootstrap variables
    string  redisConnectorChannel,
            redisOrderBookChannel,
            redisHost,
            redisPassword,
            logHost,
            walletName,
            exchangeSecret,
            exchangePassword,
            exchangeKey,
            exchangeApiUrl,
            exchangeWsUrl,
            exchangeRedisOrderChannel,
            portfolioName,
            expiration,
            redisURL,
            addressID,
            scope,
            redisChannel;
    int redisPort, logPort;
    bool walletEnabled;
    vector<string> coin_included;
    string type;

    void auth(string user, string password, string type);

    void setGlobalValue(string res);

    void redisMan();

    void run();

};



#endif // APP_H
