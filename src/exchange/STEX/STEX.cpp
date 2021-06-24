#include "exchange/STEX/STEX.h"
#include "exchange/STEX/API.h"
#include "Util.h"
#include "AC.h"

#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <sw/redis++/redis++.h>

using namespace rapidjson;
using namespace std;
using namespace sw::redis;

static Document doc_order;
static long seq = 0;
STEX::STEX(vector<string> coin_included, string access_token, string uri, string redisURL, string connectorid)
{
	util = Util();
	RedisUri = redisURL;
	ConnectorID = connectorid;
	api.uri = "https://" + uri;
	api.access_token = access_token;
	myCoinList = coin_included;
}

STEX::~STEX()
{
}

// currency_data_format for Bellman-Ford
Document STEX::currency_data_format()
{
	Document doc_tickers;
	doc_tickers.SetObject();
	rapidjson::Document::AllocatorType &allocator = doc_tickers.GetAllocator();

	try
	{
		string str_tickers = api.Get_ticker_list_currency_pairs();
		Document d_tickers;
		d_tickers.Parse(str_tickers.c_str());
		assert(d_tickers["data"].IsArray());

		const Value &c_tickers = d_tickers["data"];
		string str_tickers_all = "[";
		for (int i = 0; i < c_tickers.Size(); i++)
		{
			if (!c_tickers[i]["bid"].IsString() || !c_tickers[i]["ask"].IsString())
				continue;

			string bid = c_tickers[i]["bid"].GetString();
			string ask = c_tickers[i]["ask"].GetString();
			string c_code = c_tickers[i]["currency_code"].GetString();
			string m_code = c_tickers[i]["market_code"].GetString();

			if (bid == "" || ask == "")
				continue;
			if (stof(bid) <= 0 || stof(ask) <= 0)
				continue;

			Document doc_temp_ticker;
			doc_temp_ticker.SetObject();
			doc_temp_ticker.AddMember("c_code", Value().SetString(StringRef(c_code.c_str())), allocator);
			doc_temp_ticker.AddMember("m_code", Value().SetString(StringRef(m_code.c_str())), allocator);
			doc_temp_ticker.AddMember("bid", Value().SetString(StringRef(bid.c_str())), allocator);
			doc_temp_ticker.AddMember("ask", Value().SetString(StringRef(ask.c_str())), allocator);

			StringBuffer sb;
			Writer<StringBuffer> w(sb);
			doc_temp_ticker.Accept(w);
			str_tickers_all += sb.GetString();
			str_tickers_all += ",";
		}

		str_tickers_all.pop_back();
		str_tickers_all += "]";
		doc_tickers.Parse(str_tickers_all.c_str());
	}
	catch (exception e)
	{
		cerr << "error occur!" << e.what() << endl;
	}
	return doc_tickers;
}

void STEX::run()
{
	cout << "Stex run" << endl;
	int i = 0;
	while (1)
	{
		Publish_orderbook("USDT", "BTC");
		if (i++ > 10)
			break;
		this_thread::sleep_for(chrono::seconds(3));
	}
}

string STEX::Get_currencyPairId(string market_code, string currency_code)
{
	string str = api.Get_currency_pairs(market_code);
	Document d;
	d.Parse(str.c_str());
	Value &data = d["data"];
	int id = 0;

	for (int i = 0; i < data.Size(); ++i)
	{
		string c_code = data[i]["currency_code"].GetString();
		if (c_code == currency_code)
		{
			id = data[i]["id"].GetInt();
			break;
		}
	}
	return to_string(id);
}

long STEX::Get_serverTimestamp()
{
	string str = api.ping();
	Document d;
	d.Parse(str.c_str());
	string ts = d["data"]["server_datetime"].GetString();
	long timestamp = util.ConvertNanoseconds(ts);
	return timestamp;
}

void STEX::Publish_orderbook(string market_code, string currency_code)
{
	string curreycyPairId = Get_currencyPairId(market_code, currency_code);
	string str = api.Get_orderbook_currency_pair(curreycyPairId);
	long timestamp = Get_serverTimestamp();

	Document d;
	d.Parse(str.c_str());
	
	if(!d.HasMember("data")) return;

	if (d == doc_order)
		return;
	doc_order.Parse(str.c_str());


	Value &asks = d["data"]["ask"];
	extractOrderbook(asks, "ask", timestamp, market_code, currency_code);

	Value &bids = d["data"]["bid"];
	extractOrderbook(bids, "bid", timestamp, market_code, currency_code);
}

void STEX::extractOrderbook(Value &data, string type, long timestamp, string market_code, string currency_code)
{
	auto redis = Redis(RedisUri);
	string exchange = "stex";

	for (int i = 0; i < data.Size(); i++)
	{
		string price = data[i]["price"].GetString();
		string volume = data[i]["amount"].GetString();
		Document doc_bid;
		rapidjson::Document::AllocatorType &allocator = doc_bid.GetAllocator();

		doc_bid.SetObject();
		doc_bid.AddMember("connector", Value().SetString(StringRef(ConnectorID.c_str())), allocator);
		doc_bid.AddMember("exchange", Value().SetString(StringRef(exchange.c_str())), allocator);
		doc_bid.AddMember("ts", timestamp, allocator);
		doc_bid.AddMember("seq", seq++, allocator);
		doc_bid.AddMember("type", Value().SetString(StringRef(type.c_str())), allocator);
		doc_bid.AddMember("price", Value().SetString(StringRef(price.c_str())), allocator);
		doc_bid.AddMember("volume", Value().SetString(StringRef(volume.c_str())), allocator);
		doc_bid.AddMember("market", Value().SetString(StringRef((currency_code + "-" + market_code).c_str())), allocator);

		StringBuffer sb;
		Writer<StringBuffer> w(sb);
		doc_bid.Accept(w);
		redis.rpush(to_string(timestamp), sb.GetString());
	};
}