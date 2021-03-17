// STEX.h
#ifndef STEX_H
#define STEX_H

#include "exchange/STEX/API.h"
#include <string>
#include <vector>
#include "rapidjson/document.h"
#include "Util.h"

using namespace std;
using namespace rapidjson;

class STEX
{
public:
    STEX(vector<string> coin_included, string access_token, string uri, string redisURL, string connectorid);
    ~STEX();
    /* Declare Variables */
    Util util;
    string uri;
    string product_id;
    string ConnectorID;
    string RedisUri;
    STEXAPI api;

    vector<string> myCoinList;

    void run();

    Document currency_data_format();

    string Get_currencyPairId(string market_code, string currency_code);

    long Get_serverTimestamp();

    void Publish_orderbook(string market_code, string currency_code);

    void extractOrderbook(Value &data, string type, long timestamp, string market_code, string currency_code);
};

#endif // STEX_H
