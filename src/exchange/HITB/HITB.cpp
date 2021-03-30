
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
#include <thread>
#include <cmath>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "API.h"
#include "Util.h"
#include "AC.h"

#include "exchange/HITB/HITB.h"
#include "exchange/HITB/API.h"
#include "exchange/HITB/Websock.h"



using namespace rapidjson;
using namespace std;

static vector<int> res_check (5, 0);
static bool isRunningHITB = false;

HITB::HITB(API app_api, vector<string> symbols, string api_key, string secret_key, string uri, string wssURL, string redisurl, string connectorid, string redisConnectorChannel, string redisOrderBookChannel)
{
	util = Util();
	this->app_api = app_api;
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

	this->CypherTrust_symbols = symbols;
	for(string symbol : CypherTrust_symbols)
    {
        symbol.erase(remove(symbol.begin(), symbol.end(), '-'), symbol.end());
        HITB_symbols.push_back(symbol);
    }

    isRunningHITB = true;

	th = thread(&HITB::exchangeMonitoring, this);
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
	// AC ac = AC(symbols);
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
		sock = new HITBWebsock(CypherTrust_symbols, wssURL, connectorID, redisURL,redisOrderBookChannel, redisConnectorChannel);

		sock->Connect();
		cout << "web socket connected !" << endl;

		for(string symbol : CypherTrust_symbols)
		{
			app_api.StartSession(connectorID, symbol);
		}
	}
	catch (exception e)
	{
		cout << "error occur: " << e.what() << endl;
	}
}

void HITB::exchangeMonitoring()
{
	while(isRunningHITB)
	{
		// delay for 10s

        // market monitoring 
        marketMonitoring();

		//set start timestamp before call REST API
	    util.setStartTimestamp();
	    //get data on REST API for monitoring
		string res = api.Get_main_account_balance();
		//set finish timestamp before call REST API
	    util.setFinishTimestamp();

	    long latencyTime = abs(util.finishTimestamp - util.startTimestamp);
	    size_t json_size = res.size();
	    long latency = latencyTime/json_size;

	    string status = "online";

	    Document d;
	    ParseResult result = d.Parse(res.c_str());
	    if(!result)
	    {
	    	status = "offline";
	    	latency = 0;
	    	res_check.push_back(1);
	    }
	    else
	    {
	    	if(latencyTime < 5000000000 ) // latencyTime < 5s
		    {
		    	status = "online";
		    	res_check.push_back(0);

		    }
		    else if(latencyTime >= 5000000000 && latencyTime < 30000000000) // 5s <= latencyTime < 30s
		    {
		    	status = "degraded";
		    	res_check.push_back(0);

		    }
		    else if(latencyTime >= 30000000000)
		    {
		    	status = "offline";
		    	latency = 0;
		    	res_check.push_back(1);
		    }
	    }
	    
	    int sum = accumulate(res_check.begin(), res_check.end(), 0);
	    if(sum == 1)
	    {
	    	status = "degraded";
	    }
	    else if(sum >= 2)
	    {
	    	status = "offline";
	    	latency = 0;
	    }

	    //publish latency through redis heartbeat channel
	    util.publishLatency(redisURL, redisConnectorChannel, "exchange", connectorID, status, util.finishTimestamp, latency);

	    if(status == "offline")
	    {
	    	this->~HITB();
	    	app_api.del_address(connectorID);
	    	break;
	    }
        this_thread::sleep_for(chrono::seconds(10));

	}
}

void HITB::marketMonitoring()
{
	//set start timestamp before call REST API
    string api_symbols = "";
    for(string symbol : HITB_symbols)
    {
    	api_symbols += symbol + ",";
    }
    util.setStartTimestamp();
    //get data on REST API for monitoring
	string res = api.Get_list_orderbook(api_symbols, "1");
	//set finish timestamp before call REST API
    util.setFinishTimestamp();

    long latencyTime = abs(util.finishTimestamp - util.startTimestamp);
    size_t json_size = res.size();
    long latency = latencyTime/json_size;

    string status = "online";

    vector<string> noSymbols;
    Document d;
    ParseResult result = d.Parse(res.c_str());
    if(!result)
    {
    	return;
    }
    else
    {
    	for(string symbol : HITB_symbols)
    	{
    		if(!d.HasMember(symbol.c_str()))
    		{
    			noSymbols.push_back(symbol);
    		}
    	}
    }

    for(string o_s : Offline_symbols)
    {
    	bool f = false;
    	for(string n_s : noSymbols)
    	{
    		if(o_s == n_s) f = true;
    	}
    	if(!f)
    	{
    		status = "online";
    		string object = "market." + CypherTrust_symbols[util.findIndex(HITB_symbols, o_s)];
		    util.publishLatency(redisURL, redisConnectorChannel, object, connectorID, status, util.finishTimestamp, latency);
    	}
    }

	for(string n_s : noSymbols)
    {
    	bool f = false;
	    for(string o_s : Offline_symbols)
    	{
    		if(o_s == n_s) f = true;
    	}
    	if(!f)
    	{
    		status = "offline";
    		string object = "market." + CypherTrust_symbols[util.findIndex(HITB_symbols, n_s)];
		    util.publishLatency(redisURL, redisConnectorChannel, object, connectorID, status, util.finishTimestamp, latency);
    	}
    }

	Offline_symbols = noSymbols;
}



HITB::HITB()
{
}


HITB::~HITB()
{
	isRunningHITB = false;
	cout << "~HITB() called  !!!!!!!!!!" << endl;
	sock->~HITBWebsock();
	for(string symbol : CypherTrust_symbols)
	{
		app_api.StopSession(connectorID, symbol);
	}
}