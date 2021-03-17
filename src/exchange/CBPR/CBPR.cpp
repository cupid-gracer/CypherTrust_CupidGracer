#include "exchange/CBPR/CBPR.h"
#include "exchange/CBPR/Auth.h"
#include "exchange/CBPR/API.h"
#include "exchange/CBPR/Websock.h"

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


using namespace rapidjson;
using namespace std;


CBPR::CBPR(vector<string> coin_included, string api_uri, string api_key, string secret, string passcode, string redisurl, string connectorID)
{
	
	string uri = "https://" + api_uri;
	string product_id = "BTC-USD";
	myCoinList = coin_included;
	redisURL = redisurl;
	ConnectorID = connectorID;


	Auth auth(api_key, secret, passcode);
	api.uri = uri;
	api.product_id = product_id;
	api.auth = auth;

}

CBPR::~CBPR()
{
}

void CBPR::run()
{
	websock();
}


void CBPR::websock()
{
	vector<string> channels = {"level2"};
	vector<string> product_ids = {"BTC-USD", "ETH-BTC", "BTC-EUR", "ETH-GBP"};
	string uri = "wss://ws-feed.pro.coinbase.com";

	CBPRWebsock sock(channels, product_ids, uri, redisURL, ConnectorID);
	sock.Connect();

	int i = 0;
	while (1)
	{
		if (i++ > 10)
			break;
		this_thread::sleep_for(chrono::seconds(30));
	}
	sock.Disconnect();
}
