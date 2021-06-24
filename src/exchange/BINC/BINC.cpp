
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

#include "exchange/BINC/BINC.h"
#include "exchange/BINC/API.h"
#include "exchange/BINC/Websock.h"



using namespace rapidjson;
using namespace std;

static vector<int> res_check (5, 0);
static bool isRunningBINC = false;

BINC::BINC(API app_api, vector<string> symbols, string api_key, string secret_key, string uri, string wssURL, string redisurl, string connectorid, string redisConnectorChannel, string redisOrderBookChannel)
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


	api.uri = uri;
	api.api_key = api_key;
	api.secret_key = secret_key;
	api.addressID = connectorid;
	api.redisURL = redisURL;
	api.redisConnectorChannel = redisConnectorChannel;

	this->CypherTrust_symbols = symbols;
	for(string symbol : CypherTrust_symbols)
    {
        symbol.erase(remove(symbol.begin(), symbol.end(), '-'), symbol.end());
        BINC_symbols.push_back(symbol);
    }

    isRunningBINC = true;

	// th = thread(&BINC::exchangeMonitoring, this);
 //    th.detach();
}


void BINC::run()
{
	// AC ac = AC(symbols);
	// while(1)
	// ac.Get_arbitrage_opportunity(currency_data_format());
	websock();
}

void BINC::websock()
{
	cout << "web socket start !" << endl;

	try
	{
		string basesymbol = "ETH";
		string quotesymbol = "BTC";
		wssURL = "wss://stream.binance.com:9443/ws";
		sock = new BINCWebsock(CypherTrust_symbols, wssURL, connectorID, redisURL,redisOrderBookChannel, redisConnectorChannel);

		sock->Connect();
		cout << "web socket connected !" << endl;

		for(string symbol : CypherTrust_symbols)
		{
			app_api.StartSession(connectorID, symbol);
		}
	}
	catch (exception e)
	{
		cout << "BINC websock start error occur: " << e.what() << endl;
	}
}

void BINC::exchangeMonitoring()
{
	while(isRunningBINC)
	{
		// delay for 10s

        // market monitoring 
        // marketMonitoring();

		//set start timestamp before call REST API
	    util.setStartTimestamp();
	    //get data on REST API for monitoring
		// string res = api.Get_main_account_balance();
		string res = "";
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
	    	this->~BINC();
	    	app_api.del_address(connectorID);
	    	break;
	    }
        this_thread::sleep_for(chrono::seconds(10));

	}
}

void BINC::marketMonitoring()
{
	//set start timestamp before call REST API
    string api_symbols = "";
    for(string symbol : BINC_symbols)
    {
    	api_symbols += symbol + ",";
    }
    util.setStartTimestamp();
    //get data on REST API for monitoring
	// string res = api.Get_list_orderbook(api_symbols, "1");
	string res = "";
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
    	for(string symbol : BINC_symbols)
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
    		string object = "market." + CypherTrust_symbols[util.findIndex(BINC_symbols, o_s)];
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
    		string object = "market." + CypherTrust_symbols[util.findIndex(BINC_symbols, n_s)];
		    util.publishLatency(redisURL, redisConnectorChannel, object, connectorID, status, util.finishTimestamp, latency);
    	}
    }

	Offline_symbols = noSymbols;
}



BINC::BINC()
{
}


BINC::~BINC()
{
	isRunningBINC = false;
	cout << "~BINC() called  !!!!!!!!!!" << endl;
	sock->~BINCWebsock();
	for(string symbol : CypherTrust_symbols)
	{
		app_api.StopSession(connectorID, symbol);
	}
}