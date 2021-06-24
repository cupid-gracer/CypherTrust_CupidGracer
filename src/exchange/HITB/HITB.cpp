
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

#include <json/json.h>
#include <json/value.h>

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

HITB::HITB(API app_api, vector<string> symbols, string api_key, string secret_key, string uri, string wssURL, string redisurl, string connectorid, string redisConnectorChannel, string redisOrderBookChannel, bool *isExitApp)
{
	util = Util();
	this->app_api = app_api;
	this->redisURL = redisurl;
	this->connectorID = connectorid;
	this->redisConnectorChannel = redisConnectorChannel;
	this->redisOrderBookChannel = redisOrderBookChannel;
	this->wssURL = wssURL;
    this->isExitApp = isExitApp;

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


	api.uri = uri;

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




Json::Value HITB::currency_data_format()
{
    Json::Value doc_tickers;

	try
	{
		string str_tickers = api.Get_list_tickers();
		string str_symbols = api.Get_list_symbols();

	    Json::Value d_tickers;
	    Json::Value d_symbols;

        Json::CharReaderBuilder charReaderBuilder;
        string errs;

        stringstream input1(str_tickers);
        bool isParse = parseFromStream(charReaderBuilder, input1, &d_tickers, &errs);
        if (!isParse) return doc_tickers; 

        stringstream input2(str_symbols);
		isParse = parseFromStream(charReaderBuilder, input2, &d_symbols, &errs);
        if (!isParse) return doc_tickers;


		string str_tickers_all = "[";

		for (int i = 0; i < d_tickers.size(); i++)
		{
			if (d_tickers[i]["bid"].asString() != "" || d_tickers[i]["ask"].asString() != "")
			{
				continue;
			}
			string bid = d_tickers[i]["bid"].asString();
			string ask = d_tickers[i]["ask"].asString();
			string symbol = d_tickers[i]["symbol"].asString();
			string c_code = "";
			string m_code = "";
			if (bid == "" || ask == "")
				continue;
			if (stof(bid) <= 0 || stof(ask) <= 0)
				continue;

			bool f = false;
			for (int j = 0; j < d_symbols.size(); j++)
			{
				if (d_symbols[j]["id"].asString() == symbol)
				{
					c_code = d_symbols[j]["baseCurrency"].asString();
					m_code = d_symbols[j]["quoteCurrency"].asString();
					f = true;
					break;
				}
			}
			if (!f)
				continue;

		    Json::Value doc_temp_ticker;
		    doc_temp_ticker["c_code"] = c_code;
		    doc_temp_ticker["m_code"] = m_code;
		    doc_temp_ticker["bid"] = bid;
		    doc_temp_ticker["ask"] = ask;

			Json::StreamWriterBuilder streamWriterBuilder;
		    streamWriterBuilder["indentation"] = "";

		    string out = writeString(streamWriterBuilder, doc_temp_ticker);
		    out.erase(remove(out.begin(), out.end(), '\n'), out.end());

			str_tickers_all += out;
			str_tickers_all += ",";
		}
		str_tickers_all.pop_back();
		str_tickers_all += "]";


		stringstream input3(str_tickers_all);
        isParse = parseFromStream(charReaderBuilder, input3, &doc_tickers, &errs);
	}
	catch (exception e)
	{
		cerr << "error occur!" << e.what() << endl;
	}
	return doc_tickers;
}

void HITB::run()
{
	websock();
}

void HITB::websock()
{
	try
	{
		wssURL = "wss://api.hitbtc.com/api/2/ws";
		sock = new HITBWebsock(app_api, CypherTrust_symbols, wssURL, connectorID, redisURL,redisOrderBookChannel, redisConnectorChannel);
		sock->Connect();
	}
	catch (exception e)
	{
        app_api.LoggingEvent(connectorID, "ERROR", "storage", "it occurs error while HITB websocket run : "  + string(e.what()));
	}
}

void HITB::exchangeMonitoring()
{
	try
	{

		while(isRunningHITB)
		{

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

		    Json::Value d;
		    Json::CharReaderBuilder charReaderBuilder;
	        string errs;

	        stringstream input1(res);
	        bool isParse = parseFromStream(charReaderBuilder, input1, &d, &errs);
		    if(!isParse)
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
		        app_api.LoggingEvent(connectorID, "INFO", "storage", "HITB exchange offline, so it will close all markets");
		    	app_api.del_address(connectorID);
		    	this->~HITB();
		        this_thread::sleep_for(chrono::seconds(5));
		    	break;
		    }

		    // market monitoring 
	        marketMonitoring();

	        this_thread::sleep_for(chrono::seconds(10));

		}
	}catch(exception e)	
	{
        app_api.LoggingEvent(connectorID, "ERROR", "storage", "it occurs error while HITB exchange monitoring : "  + string(e.what()));
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
    Json::Value d;
    Json::CharReaderBuilder charReaderBuilder;
    string errs;

    stringstream input1(res);
    bool isParse = parseFromStream(charReaderBuilder, input1, &d, &errs);
    if(!isParse)
    {
    	return;
    }
    else
    {
    	for(string symbol : HITB_symbols)
    	{
    		if(!d[symbol])
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
	sock->~HITBWebsock();
	for(string symbol : CypherTrust_symbols)
	{
		app_api.StopSession(connectorID, symbol);
	}
    *isExitApp = true;
}