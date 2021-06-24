#include "exchange/CBPR/Websock.h"
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <thread>

#include <shared_mutex>
#include <sw/redis++/redis++.h>

#include <json/json.h>
#include <json/value.h>

#include <fstream>

#include "Util.h"
#include "API.h"

// using namespace rapidjson;
using namespace std;
using namespace sw::redis;

static uint64_t seq = 1;
static bool isStreamNow = true;
static bool isSnapshotNow = false;
static bool isFirstSnapshot = true;
static bool isFirstSend = true;
static bool isFirstSendDone = false;
static long lastTimestamp = 0;
static long timeSub = 100000000000;  //100s
static long timeSpeedControl = 300000000;  //300ms
static vector<string> registeredMarkets;

static int treadNumber = 0;

void CBPRWebsock::message_handler(web::websockets::client::websocket_incoming_message msg)
{
    if(treadNumber >= 500){
        return;
    }
    else if(is_connected)
    {
        thread th_message_process = thread(&CBPRWebsock::message_process, this, msg);
        th_message_process.detach();
    }
}

void CBPRWebsock::message_process(web::websockets::client::websocket_incoming_message msg)
{
    bool isThreadProcess = false;
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
            app_api.LoggingEvent(ConnectorID, "WARNING", "stream", "CBPR websocket stream data error");
            return;
        }

        string type = obj["type"].asString();
        string market = obj["product_id"].asString();

        if (type == "snapshot")
        {
            isSnapshotNow = true;
            if(isFirstSnapshot)
            {
                isFirstSnapshot = false;
                app_api.LoggingEvent(ConnectorID, "INFO", "stream", "Sending of a snapshot for orders.");
                app_api.LoggingEvent(ConnectorID, "INFO", "stream", "Sending of a snapshot for trades.");
            }

            Json::StreamWriterBuilder streamWriterBuilder;
            streamWriterBuilder["indentation"] = "";
            string bids = writeString(streamWriterBuilder, obj["bids"]);
            string asks = writeString(streamWriterBuilder, obj["asks"]);
            bids.erase(remove(bids.begin(), bids.end(), '\n'), bids.end());
            bids.erase(remove(bids.begin(), bids.end(), '"'), bids.end());
            asks.erase(remove(asks.begin(), asks.end(), '\n'), asks.end());
            asks.erase(remove(asks.begin(), asks.end(), '"'), asks.end());
            snapshotMass("order", market,  bids, asks);
            isSnapshotNow = false;
        }
        else if (type == "last_match")
        {
            isSnapshotNow = true;
            string side     = obj["side"].asString();
            string price    = obj["price"].asString();
            string volume   = obj["size"].asString();
            sendToRedis("trade", market,  side,  price,  volume, 1, 0);
            isSnapshotNow = false;
        }
        else if (type == "l2update")
        {
            treadNumber++;
            isThreadProcess = true;

            if(isSnapshotNow)
            {
                treadNumber--;
                return;
            }

            long exchangeTimestamp = util.ConvertNanoseconds(obj["time"].asString());
            long connectorTimestamp = util.GetNowTimestamp();

            //snapshort start 
            lastTimestamp = exchangeTimestamp;
            if(!isStreamNow)
            {
                treadNumber--;
                return;
            }
            if(isFirstSend)
            {
                isFirstSend = false;
                sendToRedis("order", "",  "start",  "",  "", 1, 0);
                sendToRedis("trade", "",  "start",  "",  "", 1, 0);
                app_api.LoggingEvent(ConnectorID, "INFO", "stream", "Starting of a market stream for orders");
                app_api.LoggingEvent(ConnectorID, "INFO", "stream", "Starting of a market stream for trades");
                isFirstSendDone = true;
            }

            string exchange = "cbpr";
            
            const Json::Value changes = obj["changes"];

            for (int i = 0; i < changes.size(); i++)
            {
                string side = changes[i][0].asString();
                side = side == "buy"?  "bid" : "ask";
                string price = changes[i][1].asString();
                string volume = changes[i][2].asString();
                bool isFirst = false;
                // send snapshot
                if(!util.isValueInVector(registeredMarkets, "order"+market+side) )
                {
                    isFirst = true;
                    registeredMarkets.push_back("order"+market+side);
                    app_api.StartSession(ConnectorID, market, true, "order", side);
                }

                sendToRedis("order", market,  side,  price,  volume, seq++, exchangeTimestamp);
            }
        }
        else if (type == "match")
        {
            if(isSnapshotNow) return;
            long exchangeTimestamp = util.ConvertNanoseconds(obj["time"].asString());
            if(abs(exchangeTimestamp - lastTimestamp) <= timeSpeedControl )
            {
                return;
            }
            long connectorTimestamp = util.GetNowTimestamp();

            //snapshort start 
            lastTimestamp = exchangeTimestamp;
            if(!isStreamNow)
            {
                return;
            }
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

            string exchange = "cbpr";

            string side         = obj["side"].asString();
            string price        = obj["price"].asString();
            string volume       = obj["size"].asString();
            uint64_t sequence   = obj["sequence"].asInt64();

            if(!util.isValueInVector(registeredMarkets, "trade"+market+side))
            {
                registeredMarkets.push_back("trade"+market+side);
                app_api.StartSession(ConnectorID, market, true, "trade", side);
            }
            
            sendToRedis("trade", market,  side,  price,  volume, sequence, exchangeTimestamp);
        }
        else if(type == "status")
        {
            const Json::Value products = obj["products"];
            for (string symbol : Product_ids)
            {
                bool f = false;
                for (int i = 0; i < products.size(); i++)
                {
                    if(symbol == products[i]["id"].asString())
                    {
                        f = true;
                        string status = products[i]["status"].asString();
                        if(status == "offline")
                        {
                            if(!util.isValueInVector(Offline_symbols, symbol))
                            {
                                string object = "market." + symbol;
                                Offline_symbols.push_back(symbol);
                                util.publishLatency(redisURL, redisConnectorChannel, object, ConnectorID, status, util.GetNowTimestamp(), 0);
                            }
                        }
                        else if(status == "online")
                        {
                            if(util.isValueInVector(Offline_symbols, symbol))
                            {
                                string object = "market." + symbol;
                                Offline_symbols.erase(remove(Offline_symbols.begin(), Offline_symbols.end(), symbol), Offline_symbols.end());
                                util.publishLatency(redisURL, redisConnectorChannel, object, ConnectorID, status, util.GetNowTimestamp(), 0);
                            }
                        }
                        break;
                    }
                }

                if(!f)
                {
                    if(!util.isValueInVector(Offline_symbols, symbol))
                    {
                        string object = "market." + symbol;
                        Offline_symbols.push_back(symbol);
                        util.publishLatency(redisURL, redisConnectorChannel, object, ConnectorID, "offline", util.GetNowTimestamp(), 0);
                    }
                }
            }
        }
    }
    catch(exception e)
    {
        isSnapshotNow = false;

    }
    if (isThreadProcess) treadNumber--;
}

void CBPRWebsock::sendToRedis(string context, string market, string sideclass, string price, string volume, uint64_t sequence, long exchangeTimestamp)
{
    if(!is_connected) return;
    string exchange = "cbpr";
    long connectorTimestamp = util.GetNowTimestamp();
    Redis redis = Redis(redisURL);

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

    try
    {
        redis.xadd(redisOrderBookChannel, "*", attrs.begin(), attrs.end());
    }
    catch (const ReplyError &err) {
        // WRONGTYPE Operation against a key holding the wrong kind of value
        app_api.LoggingEvent(ConnectorID, "ERROR", "stream", err.what());
    } catch (const TimeoutError &err) {
        // reading or writing timeout
        app_api.LoggingEvent(ConnectorID, "ERROR", "stream", err.what());
    } catch (const ClosedError &err) {
        // the connection has been closed.
        app_api.LoggingEvent(ConnectorID, "ERROR", "stream", err.what());
    } catch (const IoError &err) {
        // there's an IO error on the connection.
        app_api.LoggingEvent(ConnectorID, "ERROR", "stream", err.what());
    } catch (const Error &err) {
       // other errors
        app_api.LoggingEvent(ConnectorID, "ERROR", "stream", err.what());
    }
}

void CBPRWebsock::snapshotMass(string context, string market, string bids, string asks)
{
    if(!is_connected) return;
    string exchange = "cbpr";
    long connectorTimestamp = util.GetNowTimestamp();

    auto redis = Redis(redisURL);

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

void CBPRWebsock::redisPublishStartOrStop(string type)
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

void CBPRWebsock::StopOrder()
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

void CBPRWebsock::send_message(string to_send)
{
    web::websockets::client::websocket_outgoing_message out_msg;
    out_msg.set_utf8_message(to_send);
    client.send(out_msg).wait();
}

string CBPRWebsock::subscribe(bool sub)
{

    Json::Value obj;

    if(sub) obj["type"] = "subscribe";
    else
    {
        obj["type"] = "unsubscribe";
    }    


    Json::Value product_ids;
    for (string product_id : Product_ids)
    {
        product_ids.append(product_id);
    }
    obj["product_ids"] = product_ids;

    Json::Value channels;
    for (string channel : Channels)
        channels.append(channel);

    obj["channels"] = channels;

    Json::StreamWriterBuilder streamWriterBuilder;
    streamWriterBuilder["indentation"] = "";
    string out = writeString(streamWriterBuilder, obj);
    out.erase(remove(out.begin(), out.end(), '\n'), out.end());
    return out;
}

void CBPRWebsock::Connect()
{
    client.set_message_handler([this](web::websockets::client::websocket_incoming_message msg) { message_handler(msg); });
    client.set_close_handler(
        [this](web::websockets::client::websocket_close_status close_status, const utility::string_t &reason, const std::error_code &error) 
        {
            app_api.LoggingEvent(ConnectorID, "INFO", "stream", "Stopping of a market stream for orders");
            app_api.LoggingEvent(ConnectorID, "INFO", "stream", "Stopping of a market stream for trades");
        }
    );

    client.connect(wssURL.c_str()).wait();
}

void CBPRWebsock::Run()
{
    cout << util.Green << " - Daemonizing connectord: Done" << util.ResetColor << endl;
    send_message(subscribe(true));
    is_connected = true;
    lastTimestamp = util.GetNowTimestamp();

    sendToRedis("order", "",  "start",  "",  "", 1, 0);
    sendToRedis("trade", "",  "start",  "",  "", 1, 0);
}

void CBPRWebsock::Disconnect()
{
    send_message(subscribe(false));
    this_thread::sleep_for(chrono::seconds(2));
    client.close().wait();
    is_connected = false;
    this_thread::sleep_for(chrono::seconds(5));
}

CBPRWebsock::CBPRWebsock(API app_api, vector<string> channels, vector<string> product_ids, string wssURL, string redisurl, string connectorID, string redisOrderBookChannel, string redisConnectorChannel)
{
    util = Util();
    this->app_api = app_api;
    this->Product_ids = product_ids;
    this->Channels = channels;
    this->Product_ids = product_ids;
    this->wssURL = wssURL;
    this->ConnectorID = connectorID;
    this->redisURL = redisurl;
    this->redisConnectorChannel = redisConnectorChannel;
    this->redisOrderBookChannel = redisOrderBookChannel;
}

CBPRWebsock::~CBPRWebsock()
{
    if (is_connected)
        Disconnect();
}