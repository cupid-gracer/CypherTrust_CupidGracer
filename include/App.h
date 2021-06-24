// App.h
#ifndef APP_H
#define APP_H

#include <string>
#include <vector>
#include "API.h"
#include "Util.h"
#include "exchange/HITB/HITB.h"
#include "exchange/BNUS/BNUS.h"
#include "exchange/EXMO/EXMO.h"
#include <thread>

using namespace std;

class App
{

public:

    App();
    App(int* signal_status, string con_setting_str, string type, string scope, API api, bool isLAN, string interface, bool *isExitApp);
    ~App();

    API api;
    string uri;
    Util util;
    int* signal_status;
    bool* isExitApp;

    HITB* hitb;
    CBPR* cbpr;
    BNUS* bnus;
    EXMO* exmo;

    thread th_redisMan;
    /* Variable Declare */
    // bootstrap variables
    string  inbound,
            outbound,
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
            redisChannel,
            interface;
    int redisPort, logPort;
    bool    walletEnabled,
            isAppRunning,
            isLAN,
            isPingReceived;
    vector<string> symbols;
    string type;
    void auth(string user, string password, string type);

    void setGlobalValue(string res);

    long pingTrigger();
    
    void redisMan();

    void run(bool StartOrStop);

    void pong(long seq);

};



#endif // APP_H
