#include "exchange/EXMO/Websock.h"
#include "Util.h"

#include <iostream>
#include <algorithm>
#include <shared_mutex>
#include <thread>
#include <cmath>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <sstream>

#include <json/json.h>
#include <json/value.h>

#include <sw/redis++/redis++.h>

using namespace rapidjson;
using namespace sw::redis;
using namespace std;
using namespace boost::posix_time;

static int count1 = 0;
static int subscribe_id = 1;
static uint64_t seq = 0;

static bool isStreamNow = false;
static bool isFirstSend = true;
static bool isFirstSendDone = false;
static long lastTimestamp = 0;
static long timeSub = 100000000000; //100s
static vector<string> registeredMarkets;

void EXMOWebsock::message_handler(web::websockets::client::websocket_incoming_message msg)
{
    thread th_message_process = thread(&EXMOWebsock::message_process, this, msg);
    th_message_process.detach();
}

void EXMOWebsock::message_process(web::websockets::client::websocket_incoming_message msg)
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
            app_api.LoggingEvent(ConnectorID, "WARNING", "stream", "EXMO websocket stream data error");
            return;
        }

        if (!obj["topic"] || !obj["data"])
        {
            return;
        }

        uint64_t exchangeTimestamp;
        string connector = ConnectorID;
        string exchange = "EXMO";

        exchangeTimestamp = obj["ts"].asInt64();
        string topic = obj["topic"].asString();
        vector<string> _temp1, _temp2;
        util.split(topic, ':', _temp1);
        string _symbol = _temp1[1];
        util.split(_temp1[0], '/', _temp2);
        string _subType = _temp2[1];
        string market = CypherTrust_symbols[util.findIndex(EXMO_symbols, _symbol)];

        string price, volume, type;

        if ( _subType == "order_book_snapshots" || _subType == "order_book_updates" )
        {
            bool isUpdate = _subType == "order_book_updates"? true : false;
            const Json::Value bids = obj["data"]["bid"];
            const Json::Value asks = obj["data"]["ask"];

            bool isFirst = false;
            // send snapshot
            if(!util.isValueInVector(registeredMarkets, _subType+market))
            {
                isFirst = true;
                registeredMarkets.push_back(_subType+market);
                app_api.StartSession(ConnectorID, market, true, "order", "bid");
                app_api.StartSession(ConnectorID, market, true, "order", "ask");
            }

            if(isUpdate)
            {
                if(isFirstSend)
                {
                    isFirstSend = false;
                    sendToRedis("order", "",  "start",  "",  "", 1, 0);
                    sendToRedis("trade", "",  "start",  "",  "", 1, 0);
                    
                    app_api.LoggingEvent(ConnectorID, "INFO", "stream", "Starting of a market stream for orders");
                    app_api.LoggingEvent(ConnectorID, "INFO", "stream", "Starting of a market stream for trades");
                    isFirstSendDone = true;
                }
                if(!isFirstSendDone) 
                {
                    return;
                }
            }

            Json::Value bidArray;
            Json::Value askArray;

            for (int i = 0; i < bids.size(); i++)
            {
                price  = bids[i][0].asString();
                volume = bids[i][1].asString();

                if(!isUpdate && isFirst) // in snapshot case
                {
                    Json::Value elementBid;
                    elementBid.append(price);
                    elementBid.append(volume);
                    bidArray.append(elementBid);
                }
                else
                {
                    sendToRedis("order", market,  "bid",  price,  volume, seq++, exchangeTimestamp);
                }
            }

            for (int i = 0; i < asks.size(); i++)
            {
                price  = asks[i][0].asString();
                volume = asks[i][1].asString();

                if(!isUpdate && isFirst) // in snapshot case
                {
                    Json::Value elementAsk;
                    elementAsk.append(price);
                    elementAsk.append(volume);
                    askArray.append(elementAsk);
                }
                else
                {
                    sendToRedis("order", market,  "bid",  price,  volume, seq++, exchangeTimestamp);
                }
            }

            if(!isUpdate && isFirst)
            {

                app_api.LoggingEvent(ConnectorID, "INFO", "stream", "Sending of a snapshot for orders.");
                app_api.LoggingEvent(ConnectorID, "INFO", "stream", "Sending of a snapshot for trades.");

                Json::StreamWriterBuilder streamWriterBuilder;
                streamWriterBuilder["indentation"] = "";

                string outBid = writeString(streamWriterBuilder, bidArray);
                string outAsk = writeString(streamWriterBuilder, askArray);

                outBid.erase(remove(outBid.begin(), outBid.end(), '"'), outBid.end());
                outAsk.erase(remove(outAsk.begin(), outAsk.end(), '"'), outAsk.end());
                snapshotMass("order", market,  outBid, outAsk);
            }
        }
        else if (_subType == "trades")
        {
            const Json::Value trades = obj["data"];
            for (int i = 0; i < trades.size(); i++)
            {
                price   = trades["price"].asString();
                volume  = trades["quantity"].asString();
                type    = trades["type"].asString();
                sendToRedis("trade", market,  type,  price,  volume, seq++, exchangeTimestamp);
            }
        }
    }
    catch (exception e)
    {
        app_api.LoggingEvent(ConnectorID, "ERROR", "stream", e.what());
    }

}


void EXMOWebsock::sendToRedis(string context, string market, string sideclass, string price, string volume, uint64_t sequence, long exchangeTimestamp)
{
    exchangeTimestamp = exchangeTimestamp * 1000000;
    if(!is_connected) return;
    string exchange = "exmo";
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

void EXMOWebsock::snapshotMass(string context, string market, string bids, string asks)
{
    if(!is_connected) return;
    string exchange = "exmo";
    long connectorTimestamp = util.GetNowTimestamp();

    auto redis = Redis(RedisUri);

    Json::Value obj;

    obj["context"]      = context;
    obj["class"]        = "snapshot";
    obj["connectorts"]  = connectorTimestamp;
    obj["exchange"]     = exchange;
    obj["market"]       = market;
    obj["bids"]         = bids;
    obj["asks"]         = asks;

    Json::StreamWriterBuilder streamWriterBuilder;
    streamWriterBuilder["indentation"] = "";

    string out = writeString(streamWriterBuilder, obj);
    out.erase(remove(out.begin(), out.end(), '\n'), out.end());

    using Attrs = vector<pair<string, string>>;
    Attrs attrs = { {market, out }};
    redis.xadd(redisOrderBookChannel, "*", attrs.begin(), attrs.end());
}

void EXMOWebsock::redisPublishStartOrStop(string type)
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

    sendToRedis("", "",  type,  "",  "", seq++, nowTimestamp);
}

void EXMOWebsock::send_message(string to_send)
{
    web::websockets::client::websocket_outgoing_message out_msg;
    out_msg.set_utf8_message(to_send);
    client.send(out_msg).wait();
}

string EXMOWebsock::subscribeOrderbook(bool sub, string sub_symbol)
{
    Json::Value obj;

    if (sub)
        obj["method"] = "subscribe";
    else
        obj["method"] = "unsubscribe";

    Json::Value product_ids;
        product_ids.append(sub_symbol);
    obj["topics"] = product_ids;

    Json::StreamWriterBuilder streamWriterBuilder;
    streamWriterBuilder["indentation"] = "";

    string out = writeString(streamWriterBuilder, obj);
    out.erase(remove(out.begin(), out.end(), '\n'), out.end());
    return out;
}

void EXMOWebsock::StopOrder()
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

void EXMOWebsock::Connect()
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
    for (string sub_symbol : EXMO_subscribe_symbols)
        send_message(subscribeOrderbook(true, sub_symbol));
    lastTimestamp = util.GetNowTimestamp();
    is_connected = true;

    sendToRedis("order", "",  "start",  "",  "", 1, 0);
    sendToRedis("trade", "",  "start",  "",  "", 1, 0);

    thread th(&EXMOWebsock::StopOrder, this);
    th.detach();
}

void EXMOWebsock::Disconnect()
{
    if (is_connected)
    {
        for (string sub_symbol : EXMO_subscribe_symbols)
            send_message(subscribeOrderbook(false, sub_symbol));

        client.close().wait();
        is_connected = false;
        redisPublishStartOrStop("stop");
        this_thread::sleep_for(chrono::seconds(5));
    }
}

EXMOWebsock::EXMOWebsock(API app_api, vector<string> cypherTrust_symbols, string wssURL, string connectorID, string redisUri, string redisOrderBookChannel, string redisConnectorChannel)
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
        replace(symbol.begin(), symbol.end(), '-', '_');
        string orderbook = "spot/order_book_snapshots:" + symbol;
        string updates = "spot/order_book_updates:" + symbol;
        string trade = "spot/trades:" + symbol;
        EXMO_subscribe_symbols.push_back(orderbook);
        EXMO_subscribe_symbols.push_back(updates);
        EXMO_subscribe_symbols.push_back(trade);
        EXMO_symbols.push_back(symbol);
    }
}

EXMOWebsock::~EXMOWebsock()
{
    if (is_connected)
        Disconnect();
}
