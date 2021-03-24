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
static bool f_start_order = false;
static bool f_stop_order = false;
static long lastTimestamp = 0;
static long timeSub = 60000000000;

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
        string ts;
        uint64_t seq;
        string connector = ConnectorID;
        string exchange = "hitb";
        string market = BaseSymbol + QuoteSymbol;
        if (d.HasMember("result") || d.HasMember("error") || !d.HasMember("params"))
            return;

        


        cout << "------------------------------------------  " << count1++ << endl;
        if (d["params"].HasMember("sequence"))
        {
            ts = d["params"]["timestamp"].GetString();
            seq = d["params"]["sequence"].GetUint64();
            cout << "sequence :  " << d["params"]["sequence"].GetUint64() << endl;
            cout << "timestamp :  " << d["params"]["timestamp"].GetString() << endl;
        }


        Value &bids = d["params"]["bid"];
        redisPublishOrder(bids, "bid", ts, seq);

        Value &asks = d["params"]["ask"];
        redisPublishOrder(asks, "ask", ts, seq);
        
    }
    catch (exception e)
    {
        cout << "error: " << e.what() << endl;
    }
}

void HITBWebsock::redisPublishOrder(Value &data, string type, string ts, uint64_t seq)
{
    lastTimestamp = util.ConvertNanoseconds(ts);

    long nowTimestamp = util.GetNowTimestamp();
    if(abs(nowTimestamp - lastTimestamp) >= timeSub) // 10s
    {
        cout << "time sub :  " << abs(nowTimestamp - lastTimestamp) << endl;
        return;
    }

    // when start order, run
    if(!f_start_order)
    {
        redisPublishStartOrStop("start");
        f_start_order = true;
        f_stop_order = false;
    }


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
        doc_bid.AddMember("market", Value().SetString(StringRef((BaseSymbol + "-" + QuoteSymbol).c_str())), allocator);

        StringBuffer sb;
        Writer<StringBuffer> w(sb);
        doc_bid.Accept(w);
        if(!f_stop_order){
            redis.publish(redisOrderBookChannel, sb.GetString());
        }
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
    cout << to_send << endl;
    web::websockets::client::websocket_outgoing_message out_msg;
    out_msg.set_utf8_message(to_send);
    client.send(out_msg).wait();
}

string HITBWebsock::subscribeOrderbook(bool sub)
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
    params.AddMember("symbol", StringRef((BaseSymbol + QuoteSymbol).c_str()), allocator);
    params.AddMember("limit", 1, allocator);
    d.AddMember("params", params, allocator);
    d.AddMember("id", 123, allocator);
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
                redisPublishStartOrStop("stop");
                f_stop_order = true;
                f_start_order = false;
            }
        }
    }
}

void HITBWebsock::Connect()
{
    
    client.set_message_handler([this](web::websockets::client::websocket_incoming_message msg) { message_handler(msg); });
    client.connect(wssURL).wait();
    send_message(subscribeOrderbook(true));
    is_connected = true;

    thread th(&HITBWebsock::StopOrder, this);
    th.detach();
}

void HITBWebsock::Disconnect()
{
    if(is_connected){
        cout << "web socket disconnect!!!" << endl;
        send_message(subscribeOrderbook(false));
        client.close().wait();
        is_connected = false;
        f_start_order = false;
        f_stop_order = false;
        redisPublishStartOrStop("stop");
    }
}

HITBWebsock::HITBWebsock( string basesymbol, string quotesymbol, string wssURL, string connectorID, string redisUri, string redisOrderBookChannel, string redisConnectorChannel)
{
    util = Util();
    BaseSymbol = basesymbol;
    QuoteSymbol = quotesymbol;
    this->wssURL = wssURL;
    ConnectorID = connectorID;
    RedisUri = redisUri;
    this->signal_status = signal_status;
    this->redisConnectorChannel = redisConnectorChannel;
    this->redisOrderBookChannel = redisOrderBookChannel;
}



HITBWebsock::~HITBWebsock()
{
    if (is_connected)
        Disconnect();
}
