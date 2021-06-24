#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>

#include <iostream>

#include <json/json.h>
#include <json/value.h>


#include "API.h"
#include "Util.h"

#include "exchange/CBPR/CBPR.h"
#include "exchange/CBPR/Auth.h"
#include "exchange/CBPR/API.h"
#include "exchange/CBPR/Websock.h"

using namespace std;

static vector<int> res_check (5, 0);
static bool isRunningCBPR = false;
CBPR::CBPR(API app_api, vector<string> symbols, string api_uri, string api_key, string secret, string passcode, string exchangeWsUrl, string redisurl, string connectorID, string  redisConnectorChannel, string redisOrderBookChannel,  bool *isExitApp)
{
	util = Util();

	this->app_api = app_api;
	this->redisURL = redisurl;
	this->ConnectorID = connectorID;
	this->redisConnectorChannel = redisConnectorChannel;
	this->redisOrderBookChannel = redisOrderBookChannel;
	this->wssURL = exchangeWsUrl;
    this->isExitApp = isExitApp;
	
	string uri =  api_uri;
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

    *isExitApp = true;

}

void CBPR::run()
{
	websock();
}


void CBPR::websock()
{
	try
	{
		vector<string> channels = {"level2", "status", "matches"};
		sock = new CBPRWebsock(app_api, channels, CypherTrust_symbols, wssURL, redisURL, ConnectorID, redisOrderBookChannel, redisConnectorChannel);
		sock->Connect();
		sock->Run();
	}
	catch (exception e)
	{
        app_api.LoggingEvent(ConnectorID, "ERROR", "storage", "it occurs error while CBPR websocket run : "  + string(e.what()));
	}
}


void CBPR::exchangeMonitoring()
{
	while(isRunningCBPR)
	{
		// delay for 10s
		this_thread::sleep_for(chrono::seconds(10));	
		try{
			//set start timestamp before call REST API
		    util.setStartTimestamp();
		    //get data on REST API for monitoring
			string res = api.Get_List_Accounts();
			//set finish timestamp before call REST API
		    util.setFinishTimestamp();
		    long latencyTime = abs(util.finishTimestamp - util.startTimestamp);
		    size_t json_size = res.size();

			if(json_size <= 0) continue;
		    long latency = latencyTime/json_size;

		    string status = "online";

			Json::CharReaderBuilder charReaderBuilder;
	        Json::Value obj;
	        stringstream input(res);
	        string errs;
	        bool isParse = parseFromStream(charReaderBuilder, input, &obj, &errs);

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
		    util.publishLatency(redisURL, redisConnectorChannel, "exchange", ConnectorID, status, util.finishTimestamp, latency);
		    if(status == "offline")
		    {
		        app_api.LoggingEvent(ConnectorID, "INFO", "storage", "CBPR exchange offline, so it will close all markets");
		    	this->~CBPR();
		    	app_api.del_address(ConnectorID);
		    	break;
		    }
		}
		catch(exception e)
		{
	        app_api.LoggingEvent(ConnectorID, "ERROR", "storage", "it occurs error while CBPR exchange monitoring : "  + string(e.what()));
		}
	}
}

