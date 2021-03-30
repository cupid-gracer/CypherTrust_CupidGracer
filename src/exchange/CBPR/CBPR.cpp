#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>

#include <iostream>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "API.h"
#include "Util.h"

#include "exchange/CBPR/CBPR.h"
#include "exchange/CBPR/Auth.h"
#include "exchange/CBPR/API.h"
#include "exchange/CBPR/Websock.h"

using namespace rapidjson;
using namespace std;

static vector<int> res_check (5, 0);
static bool isRunningCBPR = false;
CBPR::CBPR(API app_api, vector<string> symbols, string api_uri, string api_key, string secret, string passcode, string exchangeWsUrl, string redisurl, string connectorID, string  redisConnectorChannel, string redisOrderBookChannel)
{
	util = Util();

	this->app_api = app_api;
	this->redisURL = redisurl;
	this->ConnectorID = connectorID;
	this->redisConnectorChannel = redisConnectorChannel;
	this->redisOrderBookChannel = redisOrderBookChannel;
	this->wssURL = exchangeWsUrl;
	
	string uri = "https://" + api_uri;
	string product_id = "BTC-USD";


	Auth auth(api_key, secret, passcode);

	api.uri = uri;
	api.product_id = product_id;
	api.auth = auth;

	this->CypherTrust_symbols = symbols;

	isRunningCBPR = true;
	th = thread(&CBPR::exchangeMonitoring, this);
    th.detach();

}

CBPR::CBPR()
{
}

CBPR::~CBPR()
{
	isRunningCBPR = false;

	sock->~CBPRWebsock();
}

void CBPR::run()
{
	websock();
}


void CBPR::websock()
{
	try
	{
		vector<string> channels = {"level2", "status"};
		wssURL = "wss://ws-feed.pro.coinbase.com";
		// CypherTrust_symbols = {"MATIC-BTC", "SUSHI-EUR" , "ETH-BTC"};
		sock = new CBPRWebsock(channels, CypherTrust_symbols, wssURL, redisURL, ConnectorID, redisOrderBookChannel, redisConnectorChannel);
		sock->Connect();
		cout << "web socket connected !" << endl;

		for(string symbol : CypherTrust_symbols)
		{
			app_api.StartSession(ConnectorID, symbol);
		}
	}
	catch (exception e)
	{
		cout << "CBPR websock init occur: " << e.what() << endl;
	}
}


void CBPR::exchangeMonitoring()
{
	while(isRunningCBPR)
	{
		// delay for 10s


		//set start timestamp before call REST API
	    util.setStartTimestamp();
	    //get data on REST API for monitoring
		string res = api.Get_List_Accounts();
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
	    util.publishLatency(redisURL, redisConnectorChannel, "exchange", ConnectorID, status, util.finishTimestamp, latency);
	    if(status == "offline")
	    {

	    	this->~CBPR();
	    	app_api.del_address(ConnectorID);
	    	break;
	    }
        this_thread::sleep_for(chrono::seconds(10));
	    
	}
}

