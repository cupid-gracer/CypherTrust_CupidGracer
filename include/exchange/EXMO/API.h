// Api.h
#ifndef EXMOAPI_H
#define EXMOAPI_H

#include <string>
#include <vector>


using namespace std;

class EXMOAPI
{
private:
    string Call(string method, bool authed, string path, string body);

public:
    EXMOAPI();
    ~EXMOAPI();
    string uri;

    string api_key;
    string secret_key;
    string addressID;
    string redisURL;
    string redisConnectorChannel;

    unsigned long nonce;

    // used for private key
    string Signature(const std::string& params);



    //public API

    string Get_Trades(string pair);

    string Get_OrderBook(string pair, string limit);

    string Get_Ticker();

    string Get_Currency();

    //private API
    string Get_UserInfo();

    string Get_UserTrades(string pair, string limit, string offset);



};



#endif // EXMOAPI_H
