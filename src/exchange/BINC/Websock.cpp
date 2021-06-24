#include "exchange/BINC/Websock.h"
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

void BINCWebsock::message_handler(web::websockets::client::websocket_incoming_message msg)
{
    thread th_message_process = thread(&BINCWebsock::message_process, this, msg);
    th_message_process.detach();
}

void BINCWebsock::message_process(web::websockets::client::websocket_incoming_message msg)
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
        if (!d.HasMember("depthUpdate"))
            return;

        string ts;
        uint64_t seq;
        string connector = ConnectorID;
        string exchange = "BINC";
        string market = CypherTrust_symbols[util.findIndex(BINC_symbols, d["s"].GetString())] ;

        cout << "------------------------------------------  " << count1++ << endl;
        if (d["params"].HasMember("sequence"))
        {
            ts = d["E"].GetString();
            seq = d["U"].GetUint64();
            cout << "market number :  " << util.findIndex(BINC_symbols, d["params"]["symbol"].GetString()) << endl;
            cout << "market :  " << market << "  " << d["s"].GetString() << endl;
            cout << "sequence :  " << d["U"].GetUint64() << endl;
            cout << "timestamp :  " << d["E"].GetString() << endl;
        }


        Value &bids = d["b"];
        redisPublishOrder(bids, "bid", ts, seq, market);

        Value &asks = d["a"];
        redisPublishOrder(asks, "ask", ts, seq, market);
        
    }
    catch (exception e)
    {
        cout << "error: " << e.what() << endl;
    }
}

void BINCWebsock::redisPublishOrder(Value &data, string type, string ts, uint64_t seq, string market)
{
    // lastTimestamp = util.ConvertNanoseconds(ts);
    long exchangeTimestamp = util.ConvertNanoseconds(ts);

    long connectorTimestamp = util.GetNowTimestamp();

    auto redis = Redis(RedisUri);
    for (int i = 0; i < data.Size(); i++)
    {
        Value &d = data[i];
        string price = d[0].GetString();
        string volume = d[1].GetString();
        if(volume == "0") continue;
        string exchange = "BINC";
        Document doc_bid;
        rapidjson::Document::AllocatorType &allocator = doc_bid.GetAllocator();

        doc_bid.SetObject();
        doc_bid.AddMember("connector", Value().SetString(StringRef(ConnectorID.c_str())), allocator);
        doc_bid.AddMember("connectorts", connectorTimestamp, allocator);
        doc_bid.AddMember("exchange", Value().SetString(StringRef(exchange.c_str())), allocator);
        doc_bid.AddMember("exchangets", exchangeTimestamp, allocator);
        doc_bid.AddMember("seq", seq, allocator);
        doc_bid.AddMember("type", Value().SetString(StringRef(type.c_str())), allocator);
        doc_bid.AddMember("price", Value().SetString(StringRef(price.c_str())), allocator);
        doc_bid.AddMember("volume", Value().SetString(StringRef(volume.c_str())), allocator);
        doc_bid.AddMember("market", Value().SetString(StringRef(market.c_str())), allocator);

        StringBuffer sb;
        Writer<StringBuffer> w(sb);
        doc_bid.Accept(w);
        // if(!f_stop_order){
        using Attrs = vector<pair<string, string>>;
        Attrs attrs = { { market , sb.GetString()}};
        redis.xadd(redisOrderBookChannel, "*", attrs.begin(), attrs.end());
            // redis.publish(redisOrderBookChannel, sb.GetString());
        // }
        break;
    };
}


void BINCWebsock::redisPublishStartOrStop(string type)
{
    auto redis = Redis(RedisUri);
    string exchange = "BINC";
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
    // redis.publish(redisOrderBookChannel, sb.GetString());
}

void BINCWebsock::send_message(string to_send)
{
    web::websockets::client::websocket_outgoing_message out_msg;
    out_msg.set_utf8_message(to_send);
    client.send(out_msg).wait();
}

string BINCWebsock::subscribeOrderbook(bool sub)
{
    Document d;
    d.SetObject();
    rapidjson::Document::AllocatorType &allocator = d.GetAllocator();
    if (sub)
        d.AddMember("method", "SUBSCRIBE", allocator);
    else
        d.AddMember("method", "UNSUBSCRIBE", allocator);

    Value product_ids(kArrayType);
    for (string &product_id : BINC_symbols)
        product_ids.PushBack(Value().SetString(StringRef((product_id + "@depth").c_str())), allocator);

    d.AddMember("params", product_ids, allocator);
    d.AddMember("id", subscribe_id++, allocator);
    StringBuffer strbuf;
    Writer<StringBuffer> writer(strbuf);
    d.Accept(writer);
    return strbuf.GetString();  
}

void BINCWebsock::StopOrder()
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


void BINCWebsock::Connect()
{
    
    client.set_message_handler([this](web::websockets::client::websocket_incoming_message msg) { message_handler(msg); });
    client.connect(wssURL).wait();
    
    send_message(subscribeOrderbook(true));
    is_connected = true;

    // thread th(&BINCWebsock::StopOrder, this);
    // th.detach();
}

void BINCWebsock::Disconnect()
{
    if(is_connected){
        cout << "web socket disconnect!!!" << endl;
        send_message(subscribeOrderbook(false));
        client.close().wait();
        is_connected = false;
        f_start_order = false;
        f_stop_order = false;
        // redisPublishStartOrStop("stop");
    }
}

BINCWebsock::BINCWebsock( vector<string> cypherTrust_symbols, string wssURL, string connectorID, string redisUri, string redisOrderBookChannel, string redisConnectorChannel)
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
        BINC_symbols.push_back(symbol);
    }
}



BINCWebsock::~BINCWebsock()
{
    if (is_connected)
        Disconnect();
}
