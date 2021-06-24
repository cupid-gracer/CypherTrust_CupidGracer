
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

#include "exchange/EXMO/EXMO.h"
#include "exchange/EXMO/API.h"
#include "exchange/EXMO/Websock.h"

using namespace rapidjson;
using namespace std;

static vector<int> res_check(5, 0);
static bool isRunningEXMO = false;

EXMO::EXMO(API app_api, vector<string> symbols, string api_key, string secret_key, string uri, string wssURL, string redisurl, string connectorid, string redisConnectorChannel, string redisOrderBookChannel,  bool *isExitApp)
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
	api.api_key = api_key;
	api.secret_key = secret_key;
	api.addressID = connectorid;
	api.redisURL = redisURL;
	api.redisConnectorChannel = redisConnectorChannel;

	this->CypherTrust_symbols = symbols;
	for (string symbol : CypherTrust_symbols)
	{
        replace(symbol.begin(), symbol.end(), '-', '_');
		EXMO_symbols.push_back(symbol);
	}

	isRunningEXMO = true;

	th = thread(&EXMO::exchangeMonitoring, this);
	th.detach();
}

void EXMO::run()
{
	websock();
}

void EXMO::websock()
{
	try
	{
		sock = new EXMOWebsock(app_api, CypherTrust_symbols, wssURL, connectorID, redisURL, redisOrderBookChannel, redisConnectorChannel);
		sock->Connect();
	}
	catch (exception e)
	{
        app_api.LoggingEvent(connectorID, "ERROR", "storage", "it occurs error while EXMO websocket run : "  + string(e.what()));
	}
}

void EXMO::exchangeMonitoring()
{
	while (isRunningEXMO)
	{

		//set start timestamp before call REST API
		util.setStartTimestamp();
		//get data on REST API for monitoring
		string res = api.Get_UserInfo();
		//set finish timestamp before call REST API
		util.setFinishTimestamp();

		long latencyTime = abs(util.finishTimestamp - util.startTimestamp);

		size_t json_size = res.size();
		if(json_size <= 0) continue;
		long latency = latencyTime / json_size;

		string status = "online";

        stringstream input(res);
        Json::CharReaderBuilder charReaderBuilder;
        Json::Value obj;
        string errs;
        bool isParse = parseFromStream(charReaderBuilder, input, &obj, &errs);

		if (!isParse)
		{
			status = "offline";
			latency = 0;
			res_check.push_back(1);
		}
		else
		{
			if (latencyTime < 5000000000) // latencyTime < 5s
			{
				status = "online";
				res_check.push_back(0);
			}
			else if (latencyTime >= 5000000000 && latencyTime < 30000000000) // 5s <= latencyTime < 30s
			{
				status = "degraded";
				res_check.push_back(0);
			}
			else if (latencyTime >= 30000000000)
			{
				status = "offline";
				latency = 0;
				res_check.push_back(1);
			}
		}

		int sum = accumulate(res_check.begin(), res_check.end(), 0);
		if (sum == 1)
		{
			status = "degraded";
		}
		else if (sum >= 2)
		{
			status = "offline";
			latency = 0;
		}

		//publish latency through redis heartbeat channel
		util.publishLatency(redisURL, redisConnectorChannel, "exchange", connectorID, status, util.finishTimestamp, latency);

		if (status == "offline")
		{
	        app_api.LoggingEvent(connectorID, "INFO", "storage", "EXMO exchange offline, so it will close all markets");
			this->~EXMO();
			app_api.del_address(connectorID);
			break;
		}
		// delay for 10s
		this_thread::sleep_for(chrono::seconds(10));
	}
}

void EXMO::marketMonitoring()
{
	//set start timestamp before call REST API
	string api_symbols = "";
	for (string symbol : EXMO_symbols)
	{
		api_symbols += symbol + ",";
	}
	util.setStartTimestamp();
	//get data on REST API for monitoring
	string res = api.Get_OrderBook(api_symbols, "1");
	//set finish timestamp before call REST API
	util.setFinishTimestamp();

	long latencyTime = abs(util.finishTimestamp - util.startTimestamp);
	size_t json_size = res.size();
	long latency = latencyTime / json_size;

	string status = "online";

	vector<string> noSymbols;


    stringstream input(res);
    Json::CharReaderBuilder charReaderBuilder;
    Json::Value obj;
    string errs;
    bool isParse = parseFromStream(charReaderBuilder, input, &obj, &errs);
	if (!isParse)
	{
		return;
	}
	else
	{
		for (string symbol : EXMO_symbols)
		{
			if (!obj[symbol])
			{
				noSymbols.push_back(symbol);
			}
		}
	}

	for (string o_s : Offline_symbols)
	{
		bool f = false;
		for (string n_s : noSymbols)
		{
			if (o_s == n_s)
				f = true;
		}
		if (!f)
		{
			status = "online";
			string object = "market." + CypherTrust_symbols[util.findIndex(EXMO_symbols, o_s)];
			util.publishLatency(redisURL, redisConnectorChannel, object, connectorID, status, util.finishTimestamp, latency);
		}
	}

	for (string n_s : noSymbols)
	{
		bool f = false;
		for (string o_s : Offline_symbols)
		{
			if (o_s == n_s)
				f = true;
		}
		if (!f)
		{
			status = "offline";
			string object = "market." + CypherTrust_symbols[util.findIndex(EXMO_symbols, n_s)];
			util.publishLatency(redisURL, redisConnectorChannel, object, connectorID, status, util.finishTimestamp, latency);
		}
	}

	Offline_symbols = noSymbols;
}

EXMO::EXMO()
{
}

EXMO::~EXMO()
{
	isRunningEXMO = false;
	sock->~EXMOWebsock();
	for (string symbol : CypherTrust_symbols)
	{
		app_api.StopSession(connectorID, symbol);
	}
    *isExitApp = true;
}