#include "exchange/BNUS/Websock.h"
#include "Util.h"

#include <iostream>
#include <algorithm>
#include <shared_mutex>
#include <thread>
#include <cassert>
#include <cmath>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <json/json.h>
#include <json/value.h>

#include <sw/redis++/redis++.h>
// Use (void) to silent unused warnings.
#define assertm(exp, msg) assert(((void)msg, exp))
 

using namespace rapidjson;
using namespace sw::redis;
using namespace std;
using namespace boost::posix_time;

static int count1 = 0;
static int subscribe_id = 1;

static bool isStreamNow = false;
static bool isFirstSend = true;
static bool isFirstSendDone = false;
static long lastTimestamp = 0;
static long timeSub = 100000000000; //100s
static long timeSpeedControl = 300000000;  //300ms
static vector<string> registeredMarkets;

void BNUSWebsock::message_handler(web::websockets::client::websocket_incoming_message msg)
{
    thread th_message_process = thread(&BNUSWebsock::message_process, this, msg);
    th_message_process.detach();
}

void BNUSWebsock::message_process(web::websockets::client::websocket_incoming_message msg)
{
    try
    {
        string _msg = msg.extract_string().get();
        if(_msg.size() <= 20  || _msg.find("{") == std::string::npos) return;

        stringstream input(_msg);

        Json::CharReaderBuilder charReaderBuilder;
        Json::Value obj;
        string errs;
        bool isParse = parseFromStream(charReaderBuilder, input, &obj, &errs);

        if (!isParse)
        {
            app_api.LoggingEvent(ConnectorID, "WARNING", "stream", "BNUS websocket stream data error");
            return;
        }
        if (!obj["e"]) return;

        string eventType = obj["e"].asString();
        if (!(eventType == "depthUpdate" || eventType == "trade"))
            return;

        long ts;
        uint64_t seq;
        string connector = ConnectorID;
        string exchange = "BNUS";
        string market = CypherTrust_symbols[util.findIndex(BNUS_symbols, obj["s"].asString())];
        ts = obj["E"].asInt64() * 1000000;

        if(isFirstSend)
        {
            isFirstSend = false;
            sendToRedis("order", "",  "start",  "",  "", 1, 0);
            sendToRedis("trade", "",  "start",  "",  "", 1, 0);

            app_api.LoggingEvent(ConnectorID, "INFO", "stream", "Starting of a market stream for orders");
            app_api.LoggingEvent(ConnectorID, "INFO", "stream", "Starting of a market stream for trades");
            isFirstSendDone = true;
        }
        if(!isFirstSendDone) return;

        if (eventType == "depthUpdate")
        {
            seq = obj["U"].asInt64();

            Json::Value bids = obj["b"];
            Json::Value asks = obj["a"];


            if(!util.isValueInVector(registeredMarkets, "order"+market))
            {
                registeredMarkets.push_back("order"+market);
                app_api.StartSession(ConnectorID, market, true, "order", "bid");
                app_api.StartSession(ConnectorID, market, true, "order", "ask");
            }

            for (int i = 0; i < bids.size(); i++)
            {
                if(bids[i].size() < 2) continue;
                string price  = bids[i][0].asString();
                string volume = bids[i][1].asString();
                sendToRedis("order", market,  "bid",  price,  volume, seq, ts);
            }

            for (int i = 0; i < asks.size(); i++)
            {
                if(bids[i].size() < 2) continue;
                string price  = asks[i][0].asString();
                string volume = asks[i][1].asString();
                sendToRedis("order", market,  "bid",  price,  volume, seq, ts);
            }
        }
        else if (eventType == "trade")
        {
            if(!util.isValueInVector(registeredMarkets, "trade"+market))
            {
                registeredMarkets.push_back("trade"+market);
                app_api.StartSession(ConnectorID, market, true, "trade", "buy");
                app_api.StartSession(ConnectorID, market, true, "trade", "sell");
            }
            string price = obj["p"].asString();
            string volume = obj["q"].asString();
            uint64_t seq = obj["t"].asInt64();
            string type = "sell";
            if(obj["m"].asBool()) type = "buy";

            sendToRedis("trade", market,  type,  price,  volume, seq, ts);
        }
    }
    catch (exception e)
    {
        app_api.LoggingEvent(ConnectorID, "ERROR", "stream", e.what());
    }
}

void BNUSWebsock::sendToRedis(string context, string market, string sideclass, string price, string volume, uint64_t sequence, long exchangeTimestamp)
{
    exchangeTimestamp = exchangeTimestamp * 1000000;
    if(!is_connected) return;
    string exchange = "bnus";
    long connectorTimestamp = util.GetNowTimestamp();

    Redis redis = Redis(RedisUri);

    Json::Value obj;

    obj["context"]      = context;
    obj["connector"]    = ConnectorID;
    obj["exchange"]     = exchange;
    obj["connectorts"]  = connectorTimestamp;
    obj["exchangets"]   = exchangeTimestamp;
    obj["seq"]          = sequence;
    obj["class"]        = sideclass;
    obj["price"]        = price;
    obj["volume"]       = volume;
    obj["market"]       = market;

    Json::StreamWriterBuilder streamWriterBuilder;
    streamWriterBuilder["indentation"] = "";

    string out = writeString(streamWriterBuilder, obj);
    out.erase(remove(out.begin(), out.end(), '\n'), out.end());

    using Attrs = vector<pair<string, string>>;
    Attrs attrs = { {market, out }};
    redis.xadd(redisOrderBookChannel, "*", attrs.begin(), attrs.end());
}

void BNUSWebsock::redisPublishStartOrStop(string type)
{
    long nowTimestamp = util.GetNowTimestamp();
    if (abs(nowTimestamp - lastTimestamp) >= timeSub && type == "start")
    {
        if (isStreamNow)
            type = "stop";
        else
            return;
    }

    // when start order, run
    if (isStreamNow)
    {
        if (type == "start")
            return;
        else if (type == "stop")
        {
            isStreamNow = false;
        }
    }
    else
    {
        isStreamNow = true;
    }

    auto redis = Redis(RedisUri);
    string exchange = "bnus";

    sendToRedis("", "",  type,  "",  "", 1, nowTimestamp);
}

void BNUSWebsock::send_message(string to_send)
{
    web::websockets::client::websocket_outgoing_message out_msg;
    out_msg.set_utf8_message(to_send);
    client.send(out_msg).wait();
}

string BNUSWebsock::subscribeOrderbook(bool sub)
{
    Json::Value obj;

    if (sub)
        obj["method"] = "SUBSCRIBE";
    else
        obj["method"] = "UNSUBSCRIBE";

    Json::Value product_ids;
    for (string product_id : BNUS_Subscribe_symbols)
    {
        transform(product_id.begin(), product_id.end(), product_id.begin(), ::tolower);
        product_ids.append(product_id);
    }

    obj["params"] = product_ids;
    obj["id"]     = subscribe_id++;

    Json::StreamWriterBuilder streamWriterBuilder;
    streamWriterBuilder["indentation"] = "";

    string out = writeString(streamWriterBuilder, obj);
    return out;
}

void BNUSWebsock::StopOrder()
{
    while (is_connected)
    {
        if (isStreamNow)
        {
            long nowTimestamp = util.GetNowTimestamp();
            if (abs(nowTimestamp - lastTimestamp) >= timeSub)
            {
                redisPublishStartOrStop("stop");
            }
            this_thread::sleep_for(chrono::seconds(5));
        }
    }
}

void BNUSWebsock::Connect()
{
    client.set_message_handler([this](web::websockets::client::websocket_incoming_message msg) { message_handler(msg); });
    client.set_close_handler(
        [this](web::websockets::client::websocket_close_status close_status, const utility::string_t &reason, const std::error_code &error) 
        {
            app_api.LoggingEvent(ConnectorID, "INFO", "stream", "Stopping of a market stream for orders");
            app_api.LoggingEvent(ConnectorID, "INFO", "stream", "Stopping of a market stream for trades");
        }
    );
    client.connect(wssURL).wait();
    cout << util.Green << " - Daemonizing connectord: Done" << util.ResetColor << endl;

    send_message(subscribeOrderbook(true));
    is_connected = true;
    lastTimestamp = util.GetNowTimestamp();
    thread th(&BNUSWebsock::StopOrder, this);
    th.detach();
}

void BNUSWebsock::Disconnect()
{
    if (is_connected)
    {
        send_message(subscribeOrderbook(false));
        client.close().wait();
        is_connected = false;
        redisPublishStartOrStop("stop");
    }
    this_thread::sleep_for(chrono::seconds(5));
}

BNUSWebsock::BNUSWebsock(API app_api, vector<string> cypherTrust_symbols, string wssURL, string connectorID, string redisUri, string redisOrderBookChannel, string redisConnectorChannel)
{
    util = Util();
    this->app_api = app_api;
    this->CypherTrust_symbols = cypherTrust_symbols;
    this->wssURL = wssURL;
    this->ConnectorID = connectorID;
    this->RedisUri = redisUri;
    this->redisConnectorChannel = redisConnectorChannel;
    this->redisOrderBookChannel = redisOrderBookChannel;

    for (string symbol : cypherTrust_symbols)
    {
        symbol.erase(remove(symbol.begin(), symbol.end(), '-'), symbol.end());
        BNUS_symbols.push_back(symbol);
        BNUS_Subscribe_symbols.push_back(symbol + "@depth");
        BNUS_Subscribe_symbols.push_back(symbol + "@trade");
    }
}

BNUSWebsock::~BNUSWebsock()
{
    if (is_connected)
        Disconnect();
}
