
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

/* Json */
#include <json/json.h>
#include <json/value.h>

/* Exchange */
#include "exchange/BNUS/BNUS.h"
// #include "exchange/BTFX/BTFX.h"
#include "exchange/CBPR/CBPR.h"
#include "exchange/EXMO/EXMO.h"
#include "exchange/HITB/HITB.h"
// #include "exchange/HUOB/HUOB.h"
// #include "exchange/KRKN/KRKN.h"
// #include "exchange/PLNX/PLNX.h"
#include "exchange/STEX/STEX.h"

#include "Util.h"

using namespace sw::redis;
using namespace std;




static long lastSeenTime = 0;
static long timeSub = 10000000000;  //10s



App::App()
{
}

App::App(int *signal_status, string con_setting_str, string type, string scope, API api, bool isLAN, string interface, bool *isExitApp)
{
    try
    {
        this->signal_status = signal_status;
        this->isExitApp = isExitApp;
        this->util = Util();
        this->api = api;

        this->type = type;
        this->scope = scope;

        this->isAppRunning = true;
        this->isLAN = isLAN;
        this->interface = interface;

        /* set Global values*/
        setGlobalValue(con_setting_str);

        /* set redis*/
        th_redisMan = thread(&App::redisMan, this);
        th_redisMan.detach();

        pingTrigger();
    }
    catch (const Error &e)
    {
        api.LoggingEvent(addressID, "ERROR", "storage", "it occurs error while init app, will do again.");
    }
}

App::~App()
{
    api.LoggingEvent(addressID, "INFO", "storage", type + " sending data is finished");
    isAppRunning = false;
    if (type == "HITB")
    {
        hitb->~HITB();
    }
    else if (type == "CBPR")
    {
        cbpr->~CBPR();
    }
    else if (type == "BNUS")
    {
        bnus->~BNUS();
    }
    else if (type == "EXMO")
    {
        exmo->~EXMO();
    }
}

long App::pingTrigger()
{
    string res = api.ping(addressID);
    if(res == "")
    {
        api.LoggingEvent(addressID, "CRITICAL", "storage", "ping response result is empty. Can't get sequence number.");
        return 0;
    }
    stringstream input(res);

    Json::CharReaderBuilder charReaderBuilder;
    Json::Value obj;
    string errs;
    bool isParse = parseFromStream(charReaderBuilder, input, &obj, &errs);
    if(!isParse) 
    {
        api.del_address(addressID);
        api.LoggingEvent(addressID, "INFO", "storage", "connector stopped because connoctor is offline");
        this->~App();
        *isExitApp = true;
    };

    long seq = obj[addressID]["seq"].asInt64();
    return seq;
}

void App::redisMan()
{
    cout << util.Green << " - Setting up heartbeats: Done" << util.ResetColor << endl;
    api.LoggingEvent(addressID, "INFO", "storage", "Setting up heartbeats: Done.");
    while (isAppRunning)
    {
        try
        {
            long nowTimeStamp = util.GetNowTimestamp();
            if( abs(nowTimeStamp - lastSeenTime ) >= timeSub )
            {
                long seq1 = pingTrigger();
                pong(seq1);
                continue;
            }


            auto redis = Redis(redisURL);

            using Attrs = vector<pair<string, string>>;

            // Each item is assigned with an id: pair<id, attributes>.
            using Item = pair<string, Attrs>;
            using ItemStream = vector<Item>;

            std::unordered_map<string, ItemStream> result;
            string ddd;

            redis.xread(inbound, "$", std::chrono::seconds(1), std::inserter(result, result.end()));

            string msg = "";
            for (const auto &stream : result)
            {
                auto &it = stream.second[0];
                auto &attr = it.second[0];
                msg = attr.second;
            }

            if(msg == "") continue;
            
            stringstream input(msg);
            Json::CharReaderBuilder charReaderBuilder;
            Json::Value obj;
            string errs;
            bool isParse = parseFromStream(charReaderBuilder, input, &obj, &errs);
        
            if (!isParse)
            {
                std::cerr << "JSON parse error: " << msg << endl;
                continue;
            }

            if (!obj["type"])
                continue;
            string type = obj["type"].asString();
            if (type != "ping")
            {
                continue;
            }

            isPingReceived = true;
            long seq = obj["seq"].asInt64();
            pong(seq);
        }
        catch (const Error &e)
        {
            api.LoggingEvent(addressID, "WARNING", "storage", "it occurs error while connector monitoring : "  + string(e.what()));
        }
    }
}

void App::setGlobalValue(string res)
{
    try
    {
        //  get address id
        addressID = util.getAddressId(res, interface);

        string wss = "wss://";
        string https = "https://";

        stringstream input(res);
        Json::CharReaderBuilder charReaderBuilder;
        Json::Value obj;
        string errs;
        bool isParse = parseFromStream(charReaderBuilder, input, &obj, &errs);

        if (!isParse) return;


        // get redis info
        redisHost = obj["bootstrap"]["redisHost"].asString();
        cout << util.Green << " - Registering the Redis network: "<< redisHost << util.ResetColor << endl;

        redisPort = obj["bootstrap"]["redisPort"].asInt();
        redisPassword = obj["bootstrap"]["redisPassword"].asString();
        string redisPrefix = obj["bootstrap"]["redisPrefix"].asString();

        Json::Value val_connector = obj["connector"];

        for(int i = 0; i < val_connector.size(); i++)
        {
            if(val_connector[i]["id"] != addressID) continue;

            inbound = redisPrefix + val_connector[i]["streaming"]["inbound"].asString();
            outbound = redisPrefix + val_connector[i]["streaming"]["outbound"].asString();


            walletName = val_connector[i]["wallet"]["walletName"].asString();
            exchangeSecret = val_connector[i]["wallet"]["exchangeSecret"].asString();
            if (val_connector[i]["wallet"]["exchangePassword"])
            {
                exchangePassword = val_connector[i]["wallet"]["exchangePassword"].asString();
            }
            exchangeKey = val_connector[i]["wallet"]["exchangeKey"].asString();
            exchangeApiUrl = https + val_connector[i]["wallet"]["exchangeApiUrl"].asString();
            exchangeWsUrl = wss + val_connector[i]["wallet"]["exchangeWsUrl"].asString();
           
            const Json::Value e = val_connector[i]["market"];
            for (int j = 0; j < e.size(); j++)
            {
                if(j >= 5) break;
                string symbol = e[j].asString();
                transform(symbol.begin(), symbol.end(), symbol.begin(), ::toupper);
                symbols.push_back(symbol);
            }
        }

        redisURL = "tcp://" + redisPassword + "@" + redisHost + ":" + to_string(redisPort);
        if(isLAN)
        {
            redisURL = "tcp://127.0.0.1";
        }
        redisChannel = "cyphertrust_database_" + addressID;
    }
    catch(const Error &e)
    {
        api.LoggingEvent(addressID, "ERROR", "storage", "it occurs error while init variables : " + string(e.what()));

    }
}

void App::pong(long seq)
{
    auto redis = Redis(redisURL);

    lastSeenTime = util.GetNowTimestamp();

    Json::Value d_pong;

    d_pong["type"]      = "pong";
    d_pong["object"]    = "connector";
    d_pong["address"]   = addressID;
    d_pong["status"]    = "online";
    d_pong["seq"]       = seq;
    d_pong["ts"]        = lastSeenTime;
    d_pong["latency"]   = "";

    Json::StreamWriterBuilder streamWriterBuilder;
    streamWriterBuilder["indentation"] = "";

    string out = writeString(streamWriterBuilder, d_pong);
    out.erase(remove(out.begin(), out.end(), '\n'), out.end());

    using Attrs = vector<pair<string, string>>;
    Attrs attrs = {{"heartbeat", out}};
    redis.xadd(inbound, "*", attrs.begin(), attrs.end());
}

void App::run(bool StartOrStop)
{

    cout << util.Green << " - Setting up exchange interfaces and streams: Done" << util.ResetColor << endl;
    api.LoggingEvent(addressID, "INFO", "storage", "Setting up exchange interfaces and streams: Done");

    if (type == "BNUS")
    {
        string api_key = exchangeKey;
        string secret_key = exchangeSecret;
        string uri = exchangeApiUrl;
        exchangeWsUrl = exchangeWsUrl + "/ws";

        bnus = new BNUS(api, symbols, api_key, secret_key, uri, exchangeWsUrl, redisURL, addressID, inbound, outbound, isExitApp);
        bnus->run();
    }
    else if (type == "EXMO")
    {
        string api_key = exchangeKey;
        string secret_key = exchangeSecret;
        string uri = exchangeApiUrl;
        exchangeWsUrl = exchangeWsUrl + ":443/v1/public";
        exmo = new EXMO(api, symbols, api_key, secret_key, uri, exchangeWsUrl, redisURL, addressID, inbound, outbound, isExitApp);
        exmo->run();
    }
    else if (type == "BTFX")
    {
        // BTFX btfx = BTFX();
        // btfx.run();
    }
    else if (type == "CBPR")
    {

        string api_uri = exchangeApiUrl;
        string api_key = exchangeKey;
        string secret_key = exchangeSecret;
        string passcode = exchangePassword;
        cbpr = new CBPR(api, symbols, api_uri, api_key, secret_key, passcode, exchangeWsUrl, redisURL, addressID, inbound, outbound, isExitApp);
        cbpr->run();
    }
    else if (type == "HITB")
    {
        string api_key = exchangeKey;
        string secret_key = exchangeSecret;
        string uri = exchangeApiUrl;
        hitb = new HITB(api, symbols, api_key, secret_key, uri, exchangeWsUrl, redisURL, addressID, inbound, outbound, isExitApp);
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

    *signal_status = 1;
}
