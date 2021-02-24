// #define _GLIBCXX_USE_CXX11_ABI 0

#include "API.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <boost/algorithm/string.hpp>
#include <iomanip>
#include <sw/redis++/redis++.h>
#include <syslog.h>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;
using namespace sw::redis;
using namespace std;


string 	redisManagementChannel,
		redisHeartbeatChannel,
		redisOrderBookChannel,
		redisHost,
		walletName,
		exchangeSecret,
		exchangePassword,
		exchangeKey,
		exchangeApiUrl,
		exchangeWsUrl,
		exchangeRedisOrderChannel,
		portfolioName;
int redisPort;
bool walletEnabled;
vector<string> coin_included;


void setGlobalValue(string res)
{
	Document d;
	d.Parse(res.c_str());

	redisManagementChannel = d["bootstrap"]["redisManagementChannel"].GetString();
	redisHeartbeatChannel = d["bootstrap"]["redisHeartbeatChannel"].GetString();
	redisOrderBookChannel = d["bootstrap"]["redisOrderBookChannel"].GetString();
	redisHost = d["bootstrap"]["redisHost"].GetString();
	redisPort = d["bootstrap"]["redisPort"].GetInt();

	const Value &c = d["connector"];

	walletName = c[0]["wallet"]["walletName"].GetString();
	walletEnabled = c[0]["wallet"]["walletEnabled"].GetBool();
	exchangeSecret = c[0]["wallet"]["exchangeSecret"].GetString();
	exchangePassword = c[0]["wallet"]["exchangePassword"].GetString();
	exchangeKey = c[0]["wallet"]["exchangeKey"].GetString();
	exchangeApiUrl = c[0]["wallet"]["exchangeApiUrl"].GetString();
	exchangeWsUrl = c[0]["wallet"]["exchangeWsUrl"].GetString();
	exchangeRedisOrderChannel = c[0]["wallet"]["exchangeRedisOrderChannel"].GetString();
	portfolioName = c[0]["wallet"]["portfolioName"].GetString();

	const Value &e = c[0]["market"]["included"];
	for(int i = 0; i < e.Size(); i++)
	{
		coin_included.push_back(e[i].GetString());
	}
}


void redisMan()
{
	try{
		cout << "tcp://" + redisHost + ":" + to_string(redisPort) << endl; 
		string url = "tcp://" + redisHost + ":" + to_string(redisPort);
		auto redis = Redis("127.0.0.1:6379");
		// cout << redis.ping("eewfefw");
		// auto redis = sw::redis::Redis(("tcp://" + redisHost + ":" + to_string(redisPort)).c_str());
		// redis.set("key", "value");
	 //    auto val = redis.get("key");    // val is of type OptionalString. See 'API Reference' section for details.
	 //    if (val) {
	 //        // Dereference val to get the returned value of std::string type.
	 //        std::cout << *val << std::endl;
	 //    }   // else key doesn't exist.
	}catch(const Error &e){

	}
}


int main(int argc, char *argv[])
{

	string user, password, type, key = "9xds0be661dc5a42e08cdca3b0bbd4";
	if(argc < 7) {
		cout << "Number of parameters is not correct!" << endl;
		return 0;
	}

	for(int i = 0; i < argc; i++)
	{
		string s = argv[i];
		if(s == "-u")
		{
			user = argv[i+1];
		}
		if(s == "-p")
		{
			password = argv[i+1];
		}
		if(s == "-t")
		{
			type = argv[i+1];
		}
	}
	if(user == "" || password == "" || type == "")
	{
		cout << "The parameters you inputed are wrong!" << endl;
		return 0;
	}


	API auth_api;
	auth_api.auth_url = "http://auth.dev.cyphertrust.eu/v1/connector/auth";

	//test
	user =  "fmaertens11";
	password = "x2NGo87ympHowidyPMhn";

	string res = auth_api.auth(user, password, key);
	if(res != "error"){
		setGlobalValue(res);
		redisMan();
	}

	

	// openlog("testident",LOG_CONS | LOG_PID | LOG_NDELAY, LOG_FTP);    
 //    syslog(LOG_SYSLOG,"Test Message1111111 %d, ",LOG_NDELAY);
 //    syslog(LOG_USER ,"Test Message2222222 %d, ",LOG_NDELAY);

	return 0;
}



