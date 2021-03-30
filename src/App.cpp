
#include "API.h"
#include "App.h"
#include <vector>
#include <algorithm>
#include <iostream>
#include "curl/curl.h"
#include <sw/redis++/redis++.h>
#include <syslog.h>
#include <csignal>
#include <thread>

/* rapidjson */
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

/* Exchange */
// #include "exchange/BNUS/BNUS.h"
// #include "exchange/BTFX/BTFX.h"
#include "exchange/CBPR/CBPR.h"
#include "exchange/HITB/HITB.h"
// #include "exchange/HUOB/HUOB.h"
// #include "exchange/KRKN/KRKN.h"
// #include "exchange/PLNX/PLNX.h"
#include "exchange/STEX/STEX.h"

#include "Util.h"

using namespace rapidjson;
using namespace sw::redis;
using namespace std;



App::App()
{
    cout << "App() called!" << endl;
}

App::App(int* signal_status, string con_setting_str, string type, string scope, API api)
{
    cout << "App( ******** ) called!" << endl;
    try
    {
        this->signal_status = signal_status;
        this->util = Util();
        this->api = api;

        this->type = type;
        this->scope = scope;

        this->isAppRunning = true;
        /* set Global values*/
        cout << "setGlobalValue started!!!"  << endl;
        setGlobalValue(con_setting_str);
        cout << "setGlobalValue finished!!!"  << endl;
        /* set redis*/

        th_redisMan = thread(&App::redisMan, this);
        th_redisMan.detach();
    }
    catch (const Error &e)
    {
        cout << "\033[1;31mApp initial error occur!\033[0m => " << e.what() << endl;
        syslog(LOG_PERROR, "App initial error occur!");
    }
}

App::~App()
{
    cout << "~app ()  called!" << endl;
    isAppRunning = false;
    if(type == "HITB")
    {
        hitb->~HITB();
    }
    else if(type == "CBPR")
    {
        cbpr->~CBPR();
    }
}

void App::redisMan()
{
    cout << "=========   redisMan thread started   ======== :  " <<  endl;
    try
    {
        isPingReceived = false;
        auto redis = Redis(redisURL);

    cout << "=========   redisMan redis created   ======== :  " <<  endl;

        // Create a Subscriber.
        auto sub = redis.subscriber();
    cout << "=========   redisMan subscriber created   ======== :  " <<  endl;

        // Set callback functions.
        sub.on_message([this](string channel, string msg) {
            try{

                cout << msg << endl;

                Document d;
                
                ParseResult result = d.Parse(msg.c_str());
                if (!result)
                {
                    std::cerr << "JSON parse error: " << msg << endl;
                    return;
                }
                if(!d.HasMember("type")) return;
                string type = d["type"].GetString();
                if(type != "ping") return;

                isPingReceived = true;
                long seq = d["seq"].GetUint64();
                pong(seq);

            }catch(exception e){
                cout << "subscribe error occur :" << e.what() << endl;
            }
        });

        // Subscribe to channels and patterns.

        sub.subscribe(redisConnectorChannel);
      
        this_thread::sleep_for(chrono::seconds(10));
        if(!isPingReceived)
        {
            if(long seq = api.ping(addressID) == 0)
            {

                this->~App();
                api.del_address(addressID);
            }
            else
            {
                isPingReceived = true;
                pong(seq);
            }
        }

        // Consume messages in a loop.
        while (isAppRunning) {
            try {
                sub.consume();

            }
            catch (const Error& err) {
                cout << "\033[1;31mRedis subscribe error occur!\033[0m => " << err.what() << endl;
                syslog(LOG_PERROR, "Redis subscribe error occur!");
            }
        }
    cout << "=========   redisMan thread finished   ======== :  " <<  endl;

    }
    catch (const Error &e)
    {
        cout << "\033[1;31mRedis error occur!!!!\033[0m => " << e.what() << endl;
        syslog(LOG_PERROR, "Redis error occur!");
    }
}


void App::setGlobalValue(string res)
{
    //  get address id
    size_t pos = res.find("\"connector\"");

    string st = res.substr(pos, 40);
    vector<string> st_array;
    util.split(st, '"', st_array);
    addressID = st_array[2];

    cout << "addressID  "  << addressID  << endl;

    string pre_channel = "cyphertrust_database_";

    Document d;
    d.Parse(res.c_str());

    // get redis info
    redisHost = d["bootstrap"]["redisHost"].GetString();
    redisPort = d["bootstrap"]["redisPort"].GetInt();
    redisPassword = d["bootstrap"]["redisPassword"].GetString();

    Value& val_connector = d["connector"][Value().SetString(StringRef(addressID.c_str()))];

    redisConnectorChannel = pre_channel + val_connector["channel"]["connector"].GetString();
    redisOrderBookChannel = pre_channel + val_connector["channel"]["orderbook"].GetString();
    

    walletName      = val_connector["wallet"]["walletName"].GetString();
    // walletEnabled   = val_connector["wallet"]["walletEnabled"].GetBool();
    exchangeSecret  = val_connector["wallet"]["exchangeSecret"].GetString();
    // exchangePassword = val_connector["wallet"]["exchangePassword"].GetString();
    exchangeKey     = val_connector["wallet"]["exchangeKey"].GetString();
    exchangeApiUrl  = val_connector["wallet"]["exchangeApiUrl"].GetString();
    exchangeWsUrl   = val_connector["wallet"]["exchangeWsUrl"].GetString();
    // exchangeRedisOrderChannel = val_connector["wallet"]["exchangeRedisOrderChannel"].GetString();
    // portfolioName   = val_connector["wallet"]["portfolioName"].GetString();

    
    // walletName = "Coinbase Pro Sandbox Dev";
    // walletEnabled = true;
    // exchangeSecret = "dOYGUFftjS5i--H8rNSoyu8rDRmIDWtH";
    // exchangePassword = "k7on2nlkl1s";
    // exchangeKey = "wcvsKvAvq5Ta8ZRfk0R_bBi6nReTbMCb";
    // exchangeApiUrl = "api.hitbtc.com/api/2";
    // exchangeWsUrl = "ws-feed.pro.coinbase.com";
    // exchangeRedisOrderChannel = "orders.cb.aldenburgh";
    // portfolioName = "Aldenburgh";

    const Value &e = val_connector["market"]["full"];
    for (int i = 0; i < e.Size(); i++)
    {
        string symbol = e[i].GetString();
        transform(symbol.begin(), symbol.end(), symbol.begin(), ::toupper);
        symbols.push_back(symbol);
    }
    redisURL = "tcp://" + redisPassword + "@" + redisHost + ":" + to_string(redisPort);
    // cout << "redis URL : "  << redisURL << endl;
    redisChannel = "cyphertrust_database_" + addressID;
}


void App::pong(long seq)
{
    auto redis = Redis(redisURL);
    Document d_pong;
    rapidjson::Document::AllocatorType &allocator = d_pong.GetAllocator();
    d_pong.SetObject();

    d_pong.AddMember("type", "pong", allocator);
    d_pong.AddMember("object", "connector", allocator);
    d_pong.AddMember("address", Value().SetString(StringRef(addressID.c_str())), allocator);
    d_pong.AddMember("status", "online", allocator);
    d_pong.AddMember("ts", util.GetNowTimestamp(), allocator);
    d_pong.AddMember("seq", seq, allocator);
    d_pong.AddMember("latency", NULL, allocator);

    StringBuffer sb;
    Writer<StringBuffer> w(sb);
    d_pong.Accept(w);
    redis.publish(redisConnectorChannel, sb.GetString());
}

void App::run(bool StartOrStop)
{
    cout << "------------    app  run   ----------------    "<<  StartOrStop  << endl;
    //test
    type = "HITB";

    if (type == "BNUS")
    {
        // BNUS bnus = BNUS();
        // bnus.run();
    }
    else if (type == "BTFX")
    {
        // BTFX btfx = BTFX();
        // btfx.run();
    }
    else if (type == "CBPR")
    {
        string api_uri = "api-public.sandbox.pro.coinbase.com";
        string api_key = "11dfbdd50299d30a03ea46736da2cb73";
        string secret_key = exchangeSecret;
               secret_key = "Aw4LtSL8CPTciYXVqV7s1ZZyiFdWgIu0nDeHWuGqK5LvUgAi1ACPPyiJY4uN65+7DgF9D0QzAVGFp4FaVHmWxw==";
        string passcode = "k7on2nlkl1s";
        
        cout << "-----  CBPR init in app.cpp:    ------" << endl;
        cbpr = new CBPR(api, symbols, api_uri, api_key, secret_key, passcode, exchangeWsUrl, redisURL, addressID, redisConnectorChannel, redisOrderBookChannel);
        cbpr->run();
    }
    else if (type == "HITB")
    {
        string api_key = exchangeKey;
        string secret_key = exchangeSecret;
        string uri = exchangeApiUrl;
        
        cout << "-----  HITB init in app.cpp:    ------" << endl;
        hitb = new HITB(api, symbols, api_key, secret_key, uri, exchangeWsUrl, redisURL, addressID, redisConnectorChannel, redisOrderBookChannel);
        hitb->run();
    }
    else if (type == "HUOB")
    {
        // HUOB huob = HUOB();
        // huob.run();
    }
    else if (type == "KRKN")
    {
        // KRKN krkn = KRKN();
        // krkn.run();
    }
    else if (type == "PLNX")
    {
        // PLNX plnx = PLNX();
        // plnx.run();
    }
    else if (type == "STEX")
    {
        string uri = "api3.stex.com";
        string access_token = "eyJ0eXAiOiJKV1QiLCJhbGciOiJSUzI1NiJ9.eyJhdWQiOiIxIiwianRpIjoiYjE5ZmJmNzUwNTg1ZjgyNzgxN2M4NTMxNzc5ZDY1ODIyNGVhYmVkZTExNjc1NWQ1MTc5ODFmYTQxMjZkZTJiNDRhYjZkMmNlNDUwZGQ1ZjkiLCJpYXQiOjE2MTIyNTE0OTAsIm5iZiI6MTYxMjI1MTQ5MCwiZXhwIjoxNjQzNzg3NDkwLCJzdWIiOiI0MjcyODAiLCJzY29wZXMiOlsicHJvZmlsZSIsInRyYWRlIiwid2l0aGRyYXdhbCIsInJlcG9ydHMiLCJwdXNoIiwic2V0dGluZ3MiXX0.SdzP9kwOb73vEyVGr7hcnF39eWTSbTLjr5Bv1UrwUCfP5sG-_haaK-_wF_OPt2fOhMbSi_JQ9y50LCefBX-TsRzcX38P0O4eoRqFhQpTNQwSDJ4VISpdL4MvMybDXaHTsDwgSvHRS-f8Ji3JjfO2aCqEmFOkKM-s4Cc5AaiIUM7-UVvGGv_5hkmQsr0Q1ssO21v7qGa1nHagn4CJsSBVjEK6yKls0IaargV8MbkMVgRErbcVyPdxi6bBa_eOh2sqGEyh_ghdE60hDjapaYqT0PA7WLNCULdt23q65bUEIgo3yLehvpM4kUdGMwEadB3d4FZj5Dcgu1Kc7R7InrpWGA4rd3sWLWssx7o_oT1UgD0q_G_AlAB70foBSYa77W9W64ZOge0Gqnq-gvkBRj33V0Nd3eJ6-JbWXhUEguf6xcxqmnVE_sdTAtEw-BV--pbAK14dgcs83MUREIaVhKqgp-AkIYtgU3-nZf0Rip2Q7wfkBYhvZaD2CTcBkKJjL8oxIry1kIGaV2R3mTuB4b4UhI_KsATbTnGem7jEQXU3U6R2W98hoAaqZazBc44hgtJ7rSs4s6LZJ4CZ8j6wT260-gK-ahwMpO1d03GMx9LlIC5aqXIgOB5kIf1_63xMOu5NS0QnuHpJAscpAHZEwTCBkPwM4gjQFSnVtOPnsn68m_U";
        STEX stex = STEX(symbols, access_token, uri, redisURL, addressID);
        stex.run();
    }
}
