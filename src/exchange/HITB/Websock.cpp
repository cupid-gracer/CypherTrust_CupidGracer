#include "exchange/HITB/Websock.h"
#include "Util.h"

#include <iostream>
#include <algorithm>
#include <shared_mutex>
#include <thread>
#include <cmath>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <json/json.h>
#include <json/value.h>

#include <sw/redis++/redis++.h>

using namespace sw::redis;
using namespace std;
using namespace boost::posix_time;

static int count1 = 0;
static int subscribe_id = 1;
static bool isStreamNow = false;
static bool isFirstSend = true;
static bool isFirstSendDone = false;
static long lastTimestamp = 0;
static long timeSub = 100000000000;  //100s
static vector<string> registeredMarkets;

static int seq = 0;

static int test_thread_num = 0;



void HITBWebsock::message_handler(web::websockets::client::websocket_incoming_message msg)
{
    thread th_message_process = thread(&HITBWebsock::message_process, this, msg);
    th_message_process.detach();
}

void HITBWebsock::message_process(web::websockets::client::websocket_incoming_message msg)
{
    try
    {
        string _msg = msg.extract_string().get();
        stringstream input(_msg);

        Json::CharReaderBuilder charReaderBuilder;
        Json::Value obj;
        string errs;
        bool isParse = parseFromStream(charReaderBuilder, input, &obj, &errs);

        if (!isParse)
        {
            app_api.LoggingEvent(ConnectorID, "WARNING", "stream", "HITB websocket stream data error");
            return;
        }

        if (obj["result"] || obj["error"] || !obj["params"])
        {
            return;
        }

        string ts;
        uint64_t seq;
        string market = CypherTrust_symbols[util.findIndex(HITB_symbols, obj["params"]["symbol"].asString())] ;
        string method = obj["method"].asString();

        //  orderbook data processing
        if ( method == "snapshotOrderbook" || method == "updateOrderbook" )
        {
            ts = obj["params"]["timestamp"].asString();
            lastTimestamp = util.ConvertNanoseconds(ts);
            seq = obj["params"]["sequence"].asInt64();

            const Json::Value bids = obj["params"]["bid"];
            const Json::Value asks = obj["params"]["ask"];

            Json::StreamWriterBuilder streamWriterBuilder;
            streamWriterBuilder["indentation"] = "";

            if( method == "snapshotOrderbook")
            {
                app_api.LoggingEvent(ConnectorID, "INFO", "stream", "Sending of a snapshot for orders.");
                string str_bids = writeString(streamWriterBuilder, bids);
                string str_asks = writeString(streamWriterBuilder, asks);
                boost::algorithm::replace_all(str_bids, "price", "");
                boost::algorithm::replace_all(str_bids, "size", "");
                boost::algorithm::replace_all(str_bids, ":", "");
                boost::algorithm::replace_all(str_bids, "{", "[");
                boost::algorithm::replace_all(str_bids, "}", "]");
                boost::algorithm::replace_all(str_bids, "\"", "");
                boost::algorithm::replace_all(str_asks, "price", "");
                boost::algorithm::replace_all(str_asks, "size", "");
                boost::algorithm::replace_all(str_asks, ":", "");
                boost::algorithm::replace_all(str_asks, "{", "[");
                boost::algorithm::replace_all(str_asks, "}", "]");
                boost::algorithm::replace_all(str_asks, "\"", "");
                snapshotMass("order", market,  str_bids, str_asks);
            }
            if( method == "updateOrderbook")
            {
                // long exchangeTimestamp = util.ConvertNanoseconds(obj["time"].asString());
                long connectorTimestamp = util.GetNowTimestamp();
                if(abs(connectorTimestamp - lastTimestamp) >= timeSub )
                {
                    return;
                }

                if(isFirstSend)
                {
                    isFirstSend = false;
                    app_api.LoggingEvent(ConnectorID, "INFO", "stream", "Starting of a market stream for orders");
                    app_api.LoggingEvent(ConnectorID, "INFO", "stream", "Starting of a market stream for trades");
                    sendToRedis("order", "",  "start",  "",  "", 1, 0);
                    sendToRedis("trade", "",  "start",  "",  "", 1, 0);
                    isFirstSendDone = true;
                }
                if(!isFirstSendDone)
                {
                    return;
                }

                if(!util.isValueInVector(registeredMarkets, "order"+market+"bid"))
                {
                    registeredMarkets.push_back("order"+market+"bid");
                    app_api.StartSession(ConnectorID, market, true, "order", "bid");
                }
                if(!util.isValueInVector(registeredMarkets, "order"+market+"ask"))
                {
                    registeredMarkets.push_back("order"+market+"ask");
                    app_api.StartSession(ConnectorID, market, true, "order", "ask");
                }

                for (int i = 0; i < bids.size(); i++)
                {
                    string price  = bids[i]["price"].asString();
                    string volume = bids[i]["size"].asString();
                    sendToRedis("order", market,  "bid",  price,  volume, seq, lastTimestamp);
                }

                for (int i = 0; i < asks.size(); i++)
                {
                    string price  = asks[i]["price"].asString();
                    string volume = asks[i]["size"].asString();
                    sendToRedis("order", market,  "ask",  price,  volume, seq, lastTimestamp);
                }
            }
        }

        //  trades data processing
        if (method == "snapshotTrades" || method == "updateTrades" )
        {
            bool isSnapshotTrades = false;
            if(method == "snapshotTrades") isSnapshotTrades = true;

            const Json::Value data = obj["params"]["data"];
            Json::Value buyTrade;
            Json::Value sellTrade;

            for (int i = 0; i < data.size(); i++)
            {
                string price = data[i]["price"].asString();
                string quantity = data[i]["quantity"].asString();
                string side = data[i]["side"].asString();
                long exchangeTimestamp = util.ConvertNanoseconds(data[i]["timestamp"].asString());

                //snapshort start 
                lastTimestamp = exchangeTimestamp;

                bool isFirst = false;
                // send snapshot
                if(!util.isValueInVector(registeredMarkets, "trade"+market+side))
                {
                    isFirst = true;
                    registeredMarkets.push_back("trade"+market+side);
                    app_api.StartSession(ConnectorID, market, true, "trade", side);
                }

                if(!isSnapshotTrades)
                {
                    sendToRedis("trade", market,  side,  price,  quantity, seq++, exchangeTimestamp);
                }
                else
                {
                    Json::Value temp;
                    temp.append(price);
                    temp.append(quantity);

                    if(side == "buy")
                    {
                        buyTrade.append(temp);
                    }
                    else
                    {
                        sellTrade.append(temp);
                    }

                }
            }

            if(isSnapshotTrades)
            {
                app_api.LoggingEvent(ConnectorID, "INFO", "stream", "Sending of a snapshot for trades.");

                Json::StreamWriterBuilder streamWriterBuilder;
                streamWriterBuilder["indentation"] = "";

                string outBuy = writeString(streamWriterBuilder, buyTrade);
                string outSell = writeString(streamWriterBuilder, sellTrade);

                outBuy.erase(remove(outBuy.begin(), outBuy.end(), '"'), outBuy.end());
                outSell.erase(remove(outSell.begin(), outSell.end(), '"'), outSell.end());
                snapshotMass("trade", market,  outBuy, outSell);
            }
        }
        
    }
    catch (exception e)
    {
        app_api.LoggingEvent(ConnectorID, "ERROR", "stream", e.what());
    }



}

void HITBWebsock::sendToRedis(string context, string market, string sideclass, string price, string volume, uint64_t sequence, long exchangeTimestamp)
{
    if(!is_connected) return;
    string exchange = "hitb";
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

void HITBWebsock::snapshotMass(string context, string market, string bids, string asks)
{
    if(!is_connected) return;
    string exchange = "hitb";
    long connectorTimestamp = util.GetNowTimestamp();

    auto redis = Redis(RedisUri);

    Json::Value obj;

    obj["context"]      = context;
    obj["class"]        = "snapshot";
    obj["connectorts"]  = connectorTimestamp;
    obj["exchange"]     = exchange;
    obj["market"]       = market;
    if(context == "order")
    {
        obj["bids"]     = bids;
        obj["asks"]     = asks;
    }
    else
    {
        obj["buy"]      = bids;
        obj["sell"]     = asks;   
    }

    Json::StreamWriterBuilder streamWriterBuilder;
    streamWriterBuilder["indentation"] = "";

    string out = writeString(streamWriterBuilder, obj);
    out.erase(remove(out.begin(), out.end(), '\n'), out.end());

    using Attrs = vector<pair<string, string>>;
    Attrs attrs = { {market, out }};
    redis.xadd(redisOrderBookChannel, "*", attrs.begin(), attrs.end());
}

void HITBWebsock::redisPublishStartOrStop(string type)
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
    if(isStreamNow)
    {
        if(type == "start") return;
        else if(type == "stop")
        {
            isStreamNow = false;
        }
    }
    else
    {
        isStreamNow = true;
    }

    sendToRedis("", "",  type,  "",  "", 1, nowTimestamp);
}

void HITBWebsock::send_message(string to_send)
{
    web::websockets::client::websocket_outgoing_message out_msg;
    out_msg.set_utf8_message(to_send);
    client.send(out_msg).wait();
}

string HITBWebsock::subscribeOrderbook(string subscribeType, string symbol, bool sub)
{
    Json::Value obj;

    if (sub)
        obj["method"] = subscribeType;
    else
        obj["method"] = "un" + subscribeType;

    Json::Value params;
    params["symbol"] = symbol;
    if(subscribeType == "subscribeTrades")
    {
        params["limit"] = 10;
    }

    obj["params"] = params;
    obj["id"] = subscribe_id++;

    Json::StreamWriterBuilder streamWriterBuilder;
    streamWriterBuilder["indentation"] = "";

    string out = writeString(streamWriterBuilder, obj);
    out.erase(remove(out.begin(), out.end(), '\n'), out.end());
    return out;
}

void HITBWebsock::StopOrder()
{
    while(is_connected)
    {
        if(isStreamNow)
        {
            long nowTimestamp = util.GetNowTimestamp();
            if(abs(nowTimestamp - lastTimestamp) >= timeSub) 
            {
                redisPublishStartOrStop("stop");
            }
            this_thread::sleep_for(chrono::seconds(5));
        }
    }
}

void HITBWebsock::Connect()
{
    try{
        client.set_message_handler([this](web::websockets::client::websocket_incoming_message msg) { message_handler(msg); });
        client.set_close_handler(
            [this](web::websockets::client::websocket_close_status close_status, const utility::string_t &reason, const std::error_code &error) 
            {
                app_api.LoggingEvent(ConnectorID, "INFO", "stream", "Stopping of a market stream for orders");
                app_api.LoggingEvent(ConnectorID, "INFO", "stream", "Stopping of a market stream for trades");
            }
        );

        client.connect(wssURL.c_str()).wait();
        cout << util.Green << " - Daemonizing connectord: Done" << util.ResetColor << endl;
        for(string symbol : HITB_symbols)
        {
            send_message(subscribeOrderbook("subscribeOrderbook", symbol, true));
            send_message(subscribeOrderbook("subscribeTrades", symbol, true));
        }
        lastTimestamp = util.GetNowTimestamp();
        is_connected = true;
    }catch(exception e){
    }

    sendToRedis("order", "",  "start",  "",  "", 1, 0);
    sendToRedis("trade", "",  "start",  "",  "", 1, 0);

    thread th(&HITBWebsock::StopOrder, this);
    th.detach();
}

void HITBWebsock::Disconnect()
{
    if(is_connected){
        for(string symbol : HITB_symbols)
        {
            send_message(subscribeOrderbook("subscribeOrderbook",symbol, false));
            send_message(subscribeOrderbook("subscribeTrades",symbol, false));
        }
        client.close().wait();
        is_connected = false;
        redisPublishStartOrStop("stop");
        this_thread::sleep_for(chrono::seconds(5));
    }
}

HITBWebsock::HITBWebsock( API app_api, vector<string> cypherTrust_symbols, string wssURL, string connectorID, string redisUri, string redisOrderBookChannel, string redisConnectorChannel)
{
    util = Util();
    this->app_api = app_api;
    this->CypherTrust_symbols = cypherTrust_symbols;
    this->wssURL = wssURL;
    this->ConnectorID = connectorID;
    this->RedisUri = redisUri;
    this->redisConnectorChannel = redisConnectorChannel;
    this->redisOrderBookChannel = redisOrderBookChannel;

    for(string symbol : cypherTrust_symbols)
    {
        symbol.erase(remove(symbol.begin(), symbol.end(), '-'), symbol.end());
        HITB_symbols.push_back(symbol);
    }
}

HITBWebsock::~HITBWebsock()
{
    if (is_connected)
        Disconnect();
}
