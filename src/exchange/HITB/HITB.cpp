#include "exchange/HITB/HITB.h"
#include "exchange/HITB/API.h"
#include "exchange/HITB/Websock.h"
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
#include <cstdlib>
#include <shared_mutex>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"


using namespace rapidjson;
using namespace std;

HITB::HITB( vector<string> coin_included, string api_key, string secret_key, string uri, string wssURL, string redisurl, string connectorid, string redisConnectorChannel, string redisOrderBookChannel)
{
	Util util;
	this->redisURL = redisurl;
	this->connectorID = connectorid;
	this->redisConnectorChannel = redisConnectorChannel;
	this->redisOrderBookChannel = redisOrderBookChannel;
	this->wssURL = wssURL;
	if (api_key == "" || secret_key == "")
	{
		cout << "api, secret key error!";
		return;
	}
	string str = api_key + ":" + secret_key;
	const char *data = str.c_str();
	int data_length = strlen(data);
	int encoded_data_length = util.Base64encode_len(data_length);
	char *base64_string = (char *)malloc(encoded_data_length);

	util.Base64encode(base64_string, data, data_length);


	api.uri = "https://" + uri;
	api.token = base64_string;
	api.addressID = connectorid;
	api.redisURL = redisURL;
	api.redisConnectorChannel = redisConnectorChannel;
	myCoinList = coin_included;

	thread th(&HITB::StartStopWebsock, this);
    th.detach();
	
}




Document HITB::currency_data_format()
{
	Document doc_tickers;
	doc_tickers.SetObject();
	rapidjson::Document::AllocatorType &allocator = doc_tickers.GetAllocator();

	try
	{
		string str_tickers = api.Get_list_tickers();
		string str_symbols = api.Get_list_symbols();

		Document d_tickers;
		Document d_symbols;
		d_tickers.Parse(str_tickers.c_str());
		d_symbols.Parse(str_symbols.c_str());
		const Value &c_tickers = d_tickers;
		const Value &c_symbols = d_symbols;

		string str_tickers_all = "[";

		for (int i = 0; i < c_tickers.Size(); i++)
		{
			if (!c_tickers[i]["bid"].IsString() || !c_tickers[i]["ask"].IsString())
			{
				continue;
			}
			string bid = c_tickers[i]["bid"].GetString();
			string ask = c_tickers[i]["ask"].GetString();
			string symbol = c_tickers[i]["symbol"].GetString();
			string c_code = "";
			string m_code = "";
			if (bid == "" || ask == "")
				continue;
			if (stof(bid) <= 0 || stof(ask) <= 0)
				continue;

			bool f = false;
			for (int j = 0; j < c_symbols.Size(); j++)
			{
				if (c_symbols[j]["id"].GetString() == symbol)
				{
					c_code = c_symbols[j]["baseCurrency"].GetString();
					m_code = c_symbols[j]["quoteCurrency"].GetString();
					f = true;
					break;
				}
			}
			if (!f)
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

void HITB::run()
{
	// AC ac = AC(myCoinList);
	// while(1)
	// ac.Get_arbitrage_opportunity(currency_data_format());
	websock();
}

void HITB::websock()
{
	cout << "web socket start !" << endl;

	try
	{
		string basesymbol = "ETH";
		string quotesymbol = "BTC";
		wssURL = "wss://api.hitbtc.com/api/2/ws";
		sock = HITBWebsock(basesymbol, quotesymbol, wssURL, connectorID, redisURL,redisOrderBookChannel, redisConnectorChannel);

		sock.Connect();
		cout << "web socket connected !" << endl;

		this_thread::sleep_for(chrono::seconds(2));
		int i = 0;
		while (1)
		{
			if (i++ > 10)
				break;
			this_thread::sleep_for(chrono::seconds(3));
		}
		
	}
	catch (exception e)
	{
		cout << "error occur: " << e.what() << endl;
	}
}


void HITB::StartStopWebsock()
{
	
}

HITB::HITB()
{
}


HITB::~HITB()
{
	cout << "~HITB() called  !!!!!!!!!!" << endl;
	sock.Disconnect();
}