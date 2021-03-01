#include "API.h"
#include "App.h"
#include <vector>
#include <iostream>
#include "curl/curl.h"
#include <sw/redis++/redis++.h>
#include <syslog.h>
#include <csignal>

/* rapidjson */
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;
using namespace sw::redis;
using namespace std;

App::App(string con_setting_str)
{
    try
    {
        /* set Global values*/
        setGlobalValue(con_setting_str);
        /* set redis*/
        redisMan();
    }
    catch (const Error &e)
    {
        cout << "\033[1;31mApp initial error occur!\033[0m => " << e.what() << endl;
        syslog(LOG_PERROR, "App initial error occur!");
    }
}

App::~App()
{
}

void App::setGlobalValue(string res)
{
    Document d;
    d.Parse(res.c_str());

    redisHeartbeatChannel = d["bootstrap"]["redisHeartbeatChannel"].GetString();
    redisOrderBookChannel = d["bootstrap"]["redisOrderBookChannel"].GetString();
    redisHost = d["bootstrap"]["redisHost"].GetString();
    redisPort = d["bootstrap"]["redisPort"].GetInt();

    const Value &c = d["connector"];

    redisManagementChannel = d["address"]["redisManagementChannel"].GetString();
    address_id = d["address"]["id"].GetString();
    expiration = d["address"]["expiration"].GetString();

    walletName      = c[0]["wallet"]["walletName"].GetString();
    walletEnabled   = c[0]["wallet"]["walletEnabled"].GetBool();
    exchangeSecret  = c[0]["wallet"]["exchangeSecret"].GetString();
    exchangePassword = c[0]["wallet"]["exchangePassword"].GetString();
    exchangeKey     = c[0]["wallet"]["exchangeKey"].GetString();
    exchangeApiUrl  = c[0]["wallet"]["exchangeApiUrl"].GetString();
    exchangeWsUrl   = c[0]["wallet"]["exchangeWsUrl"].GetString();
    exchangeRedisOrderChannel = c[0]["wallet"]["exchangeRedisOrderChannel"].GetString();
    portfolioName   = c[0]["wallet"]["portfolioName"].GetString();

    const Value &e = c[0]["market"]["included"];
    for (int i = 0; i < e.Size(); i++)
    {
        coin_included.push_back(e[i].GetString());
    }
}

void App::redisMan()
{
    try
    {

        cout << "tcp://" + redisHost + ":" + to_string(redisPort) << endl;
        string url = "tcp://" + redisHost + ":" + to_string(redisPort);

        auto redis = Redis("tcp://127.0.0.1:6379");
        cout << redis.ping() << endl;

        // auto redis = sw::redis::Redis(("tcp://" + redisHost + ":" + to_string(redisPort)).c_str());
        redis.set("key", "value");

        auto val = redis.get("key"); // val is of type OptionalString. See 'API Reference' section for details.

        if (val)
        {
            // Dereference val to get the returned value of std::string type.
            cout << *val << endl;
        } // else key doesn't exist.
        else
            cout << "key doesn't exist.";
        // auto val = redis.get("key");
        // cout << *val << endl;

        // Redis redis("tcp://127.0.0.1:6379");

        // // Create a Subscriber.
        // auto sub = redis.subscriber();

        // // Set callback functions.
        // sub.on_message([](std::string channel, std::string msg) {
        //     // Process message of MESSAGE type.
        // });

        // sub.on_pmessage([](std::string pattern, std::string channel, std::string msg) {
        //     // Process message of PMESSAGE type.
        // });

        // sub.on_meta([](Subscriber::MsgType type, OptionalString channel, long long num) {
        //     // Process message of META type.
        // });

        // // Subscribe to channels and patterns.
        // sub.subscribe("channel1");
        // // sub.subscribe({"channel2", "channel3"});

        // // sub.psubscribe("pattern1*");

        // // Consume messages in a loop.
        // while (true) {
        //     try {
        //         sub.consume();
        //     } catch (...) {
        //         // Handle exceptions.
        //     }
        // }
    }
    catch (const Error &e)
    {
        cout << "\033[1;31mRedis error occur!\033[0m => " << e.what() << endl;
        syslog(LOG_PERROR, "Redis error occur!");
    }
}



