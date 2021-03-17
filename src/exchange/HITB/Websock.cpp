#include "exchange/HITB/Websock.h"
#include <iostream>
#include <algorithm>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <shared_mutex>
#include <thread>
#include <sw/redis++/redis++.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "Util.h"

using namespace rapidjson;
using namespace sw::redis;
using namespace std;
using namespace boost::posix_time;

static int count1 = 0;
void HITBWebsock::message_handler(web::websockets::client::websocket_incoming_message msg)
{
    try
    {
        string input = msg.extract_string().get();
        cout << input << endl << endl;
        cout << "------------------------------------------  " << count1++ << endl;
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

        if (d["params"].HasMember("sequence"))
        {
            ts = d["params"]["timestamp"].GetString();
            seq = d["params"]["sequence"].GetUint64();
            cout << "sequence :  " << d["params"]["sequence"].GetUint64() << endl;
            cout << "timestamp :  " << d["params"]["timestamp"].GetString() << endl;
        }
        Value &bids = d["params"]["bid"];
        redisPublish(bids, "bid", ts, seq);

        Value &asks = d["params"]["ask"];
        redisPublish(asks, "ask", ts, seq);
        
    }
    catch (exception e)
    {
        cout << "error: " << e.what() << endl;
    }
}

void HITBWebsock::redisPublish(Value &data, string type, string ts, uint64_t seq)
{
    long timestamp = util.GetMicroseconds(ts);
    auto redis = Redis(RedisUri);
    for (int i = 0; i < data.Size(); i++)
    {
        string price = data[i]["price"].GetString();
        string volume = data[i]["size"].GetString();
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
        redis.lpush(to_string(timestamp), sb.GetString());
    };
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

void HITBWebsock::Connect()
{
    
    client.set_message_handler([this](web::websockets::client::websocket_incoming_message msg) { message_handler(msg); });
    client.connect(Uri).wait();
    send_message(subscribeOrderbook(true));
    is_connected = true;
}

void HITBWebsock::Disconnect()
{
    send_message(subscribeOrderbook(false));
    client.close().wait();
    is_connected = false;
}

HITBWebsock::HITBWebsock(string basesymbol, string quotesymbol, string uri, string connectorID, string redisUri)
{
    util = Util();
    BaseSymbol = basesymbol;
    QuoteSymbol = quotesymbol;
    Uri = uri;
    ConnectorID = connectorID;
    RedisUri = redisUri;
}

HITBWebsock::~HITBWebsock()
{
    if (is_connected)
        Disconnect();
}
