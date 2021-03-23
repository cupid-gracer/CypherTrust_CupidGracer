#include "exchange/CBPR/Websock.h"
#include <iostream>
#include <algorithm>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <shared_mutex>
#include <sw/redis++/redis++.h>

#include "Util.h"

using namespace rapidjson;
using namespace std;
using namespace sw::redis;

static uint64_t seq = 0;

void CBPRWebsock::message_handler(web::websockets::client::websocket_incoming_message msg)
{
    try{
    auto redis = Redis(redisURL);
    string input = msg.extract_string().get();
    Document d;
    d.Parse(input.c_str());
    string type = d["type"].GetString();
    if (type == "snapshot")
    {
        return;
        const Value &bids = d["bids"];
        const Value &asks = d["asks"];
        for (SizeType i = 0; i < bids.Size(); i++)
        {
            buy_prices.push_back(stod(bids[i][0].GetString()));
        }
        for (SizeType i = 0; i < asks.Size(); i++)
        {
            sell_prices.push_back(stod(asks[i][0].GetString()));
        }
    }
    else if (type == "l2update")
    {
        long timestamp = util.ConvertNanoseconds(d["time"].GetString());
        string market = d["product_id"].GetString();
        string exchange = "cbpr";
        cout << "-------------------  " << seq << endl << market << endl;

        const Value &changes = d["changes"];

        assert(changes.IsArray());
        for (SizeType i = 0; i < changes.Size(); i++)
        {
            assert(changes[i].IsArray());
            string price = changes[i][1].GetString();
            string side = changes[i][0].GetString();
            string volume = changes[i][2].GetString();

            Document doc_bid;
            rapidjson::Document::AllocatorType &allocator = doc_bid.GetAllocator();
            doc_bid.SetObject();
            doc_bid.AddMember("connector", Value().SetString(StringRef(ConnectorID.c_str())), allocator);
            doc_bid.AddMember("exchange", Value().SetString(StringRef(exchange.c_str())), allocator);
            doc_bid.AddMember("ts", timestamp, allocator);
            doc_bid.AddMember("seq", seq++, allocator);
            doc_bid.AddMember("type", Value().SetString(StringRef(side.c_str())), allocator);
            doc_bid.AddMember("price", Value().SetString(StringRef(price.c_str())), allocator);
            doc_bid.AddMember("volume", Value().SetString(StringRef(volume.c_str())), allocator);
            doc_bid.AddMember("market", Value().SetString(StringRef(market.c_str())), allocator);

            StringBuffer sb;
            Writer<StringBuffer> w(sb);
            doc_bid.Accept(w);
            redis.lpush(to_string(timestamp), sb.GetString());
        }
    }}catch(exception e)
    {
        cout << "CBPR error occur: " << e.what();
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
    Document d;
    d.SetObject();
    rapidjson::Document::AllocatorType &allocator = d.GetAllocator();
    if (sub)
        d.AddMember("type", "subscribe", allocator);
    else
        d.AddMember("type", "unsubscribe", allocator);
    Value product_ids(kArrayType);
    for (string &product_id : Product_ids)
        product_ids.PushBack(Value().SetString(StringRef(product_id.c_str())), allocator);
    d.AddMember("product_ids", product_ids, allocator);
    Value channels(kArrayType);
    for (string &channel : Channels)
        channels.PushBack(Value().SetString(StringRef(channel.c_str())), allocator);
    d.AddMember("channels", channels, allocator);
    StringBuffer strbuf;
    Writer<StringBuffer> writer(strbuf);
    d.Accept(writer);
    return strbuf.GetString();
}

double CBPRWebsock::Best_Buy_Price()
{
    shared_lock<shared_mutex> lock(buy_mut);
    auto biggest = max_element(begin(buy_prices), end(buy_prices));
    return *biggest;
}

double CBPRWebsock::Best_Sell_Price()
{
    shared_lock<shared_mutex> lock(sell_mut);
    auto smallest = min_element(begin(sell_prices), end(sell_prices));
    return *smallest;
}

double CBPRWebsock::MidMarket_Price()
{
    return (Best_Buy_Price() + Best_Sell_Price()) / 2;
}

void CBPRWebsock::Connect()
{
    client.set_message_handler([this](web::websockets::client::websocket_incoming_message msg) { message_handler(msg); });
#ifdef _WIN32
    client.connect(web::uri(utility::conversions::to_string_t(Uri))).wait();
#else
    client.connect(Uri).wait();
#endif
    send_message(subscribe(true));
    is_connected = true;
}

void CBPRWebsock::Disconnect()
{
    send_message(subscribe(false));
    client.close().wait();
    is_connected = false;
}

CBPRWebsock::CBPRWebsock(vector<string> channels, vector<string> product_ids, string uri, string redisurl, string connectorID)
{

    Channels = channels;
    Product_ids = product_ids;
    Uri = uri;
    ConnectorID = connectorID;
    redisURL = redisurl;
    util = Util();
}
CBPRWebsock::~CBPRWebsock()
{
    if (is_connected)
        Disconnect();
}