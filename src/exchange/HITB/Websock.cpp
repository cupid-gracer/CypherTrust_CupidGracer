#include "exchange/HITB/Websock.h"
#include "Util.h"

#include <iostream>
#include <algorithm>
#include <shared_mutex>
#include <thread>
#include <cmath>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include <sw/redis++/redis++.h>

using namespace rapidjson;
using namespace sw::redis;
using namespace std;
using namespace boost::posix_time;

static int count1 = 0;
static int subscribe_id = 1;
static bool f_start_order = false;
static bool f_stop_order = false;
static long lastTimestamp = 0;
static long timeSub = 100000000000;  //100s

void HITBWebsock::message_handler(web::websockets::client::websocket_incoming_message msg)
{
    try
    {   
        string input = msg.extract_string().get();
        Document d;
        ParseResult result = d.Parse(input.c_str());
        if (!result)
        {
            std::cerr << "JSON parse error: " << input << endl;
            return;
        }
        if (d.HasMember("result") || d.HasMember("error") || !d.HasMember("params"))
            return;

        string ts;
        uint64_t seq;
        string connector = ConnectorID;
        string exchange = "hitb";
        string market = CypherTrust_symbols[util.findIndex(HITB_symbols, d["params"]["symbol"].GetString())] ;

        cout << "------------------------------------------  " << count1++ << endl;
        if (d["params"].HasMember("sequence"))
        {
            ts = d["params"]["timestamp"].GetString();
            seq = d["params"]["sequence"].GetUint64();
            cout << "market number :  " << util.findIndex(HITB_symbols, d["params"]["symbol"].GetString()) << endl;
            cout << "market :  " << market << "  " << d["params"]["symbol"].GetString() << endl;
            cout << "sequence :  " << d["params"]["sequence"].GetUint64() << endl;
            cout << "timestamp :  " << d["params"]["timestamp"].GetString() << endl;
        }


        Value &bids = d["params"]["bid"];
        redisPublishOrder(bids, "bid", ts, seq, market);

        Value &asks = d["params"]["ask"];
        redisPublishOrder(asks, "ask", ts, seq, market);
        
    }
    catch (exception e)
    {
        cout << "error: " << e.what() << endl;
    }
}

void HITBWebsock::redisPublishOrder(Value &data, string type, string ts, uint64_t seq, string market)
{
    lastTimestamp = util.ConvertNanoseconds(ts);

    long nowTimestamp = util.GetNowTimestamp();
    if(abs(nowTimestamp - lastTimestamp) >= timeSub) // 10s
    {
        return;
    }

    // when start order, run
    // if(!f_start_order)
    // {
    //     redisPublishStartOrStop("start");
    //     f_start_order = true;
    //     f_stop_order = false;
    // }


    long timestamp = util.ConvertNanoseconds(ts);
    auto redis = Redis(RedisUri);
    for (int i = 0; i < data.Size(); i++)
    {
        string price = data[i]["price"].GetString();
        string volume = data[i]["size"].GetString();
        if(volume == "0") continue;
        string exchange = "hitb";
        Document doc_bid;
        rapidjson::Document::AllocatorType &allocator = doc_bid.GetAllocator();

        doc_bid.SetObject();
        doc_bid.AddMember("connector", Value().SetString(StringRef(ConnectorID.c_str())), allocator);
        doc_bid.AddMember("exchange", Value().SetString(StringRef(exchange.c_str())), allocator);
        doc_bid.AddMember("ts", timestamp, allocator);
        doc_bid.AddMember("seq", seq, allocator);
        doc_bid.AddMember("type", Value().SetString(StringRef(type.c_str())), allocator);
        doc_bid.AddMember("price", Value().SetString(StringRef(price.c_str())), allocator);
        doc_bid.AddMember("volume", Value().SetString(StringRef(volume.c_str())), allocator);
        doc_bid.AddMember("market", Value().SetString(StringRef(market.c_str())), allocator);

        StringBuffer sb;
        Writer<StringBuffer> w(sb);
        doc_bid.Accept(w);
        // if(!f_stop_order){
            redis.publish(redisOrderBookChannel, sb.GetString());
        // }
        break;
    };
}


void HITBWebsock::redisPublishStartOrStop(string type)
{
    auto redis = Redis(RedisUri);
    string exchange = "hitb";
    long timestamp = util.GetNowTimestamp();

    Document d;
    rapidjson::Document::AllocatorType &allocator = d.GetAllocator();

    d.SetObject();
    d.AddMember("connector", Value().SetString(StringRef(ConnectorID.c_str())), allocator);
    d.AddMember("exchange", Value().SetString(StringRef(exchange.c_str())), allocator);
    d.AddMember("ts", timestamp, allocator);
    d.AddMember("seq", 0, allocator);
    d.AddMember("type", Value().SetString(StringRef(type.c_str())), allocator);
    d.AddMember("price", "", allocator);
    d.AddMember("volume", "", allocator);
    d.AddMember("market", "", allocator);

    StringBuffer sb;
    Writer<StringBuffer> w(sb);
    d.Accept(w);

    redis.publish(redisOrderBookChannel, sb.GetString());
}

void HITBWebsock::send_message(string to_send)
{
    web::websockets::client::websocket_outgoing_message out_msg;
    out_msg.set_utf8_message(to_send);
    client.send(out_msg).wait();
}

string HITBWebsock::subscribeOrderbook(string symbol, bool sub)
{
    Document d;
    d.SetObject();
    rapidjson::Document::AllocatorType &allocator = d.GetAllocator();
    if (sub)
        d.AddMember("method", "subscribeOrderbook", allocator);
    else
        d.AddMember("method", "unsubscribeOrderbook", allocator);
    Document params;
    params.SetObject();
    params.AddMember("symbol", StringRef(symbol.c_str()), allocator);
    d.AddMember("params", params, allocator);
    d.AddMember("id", subscribe_id++, allocator);
    StringBuffer strbuf;
    Writer<StringBuffer> writer(strbuf);
    d.Accept(writer);
    return strbuf.GetString();  
}

void HITBWebsock::StopOrder()
{
    while(is_connected)
    {
        if(f_start_order && !f_stop_order)
        {
            long nowTimestamp = util.GetNowTimestamp();
            if(abs(nowTimestamp - lastTimestamp) >= 10000000000) // 10s
            {
                f_stop_order = true;
                f_start_order = false;
                redisPublishStartOrStop("stop");
            }
        }
    }
}


void HITBWebsock::Connect()
{
    
    client.set_message_handler([this](web::websockets::client::websocket_incoming_message msg) { message_handler(msg); });
    client.connect(wssURL).wait();
    for(string symbol : HITB_symbols)
    {
        send_message(subscribeOrderbook(symbol, true));
        this_thread::sleep_for(chrono::seconds(10));
    }
    is_connected = true;

    // thread th(&HITBWebsock::StopOrder, this);
    // th.detach();
}

void HITBWebsock::Disconnect()
{
    if(is_connected){
        cout << "web socket disconnect!!!" << endl;
        for(string symbol : HITB_symbols)
        {
            send_message(subscribeOrderbook(symbol, false));
        }
        client.close().wait();
        is_connected = false;
        f_start_order = false;
        f_stop_order = false;
        // redisPublishStartOrStop("stop");
    }
}

HITBWebsock::HITBWebsock( vector<string> cypherTrust_symbols, string wssURL, string connectorID, string redisUri, string redisOrderBookChannel, string redisConnectorChannel)
{
    util = Util();
    
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
