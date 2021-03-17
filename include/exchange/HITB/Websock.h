// Websock.h
#ifndef HITBWEBSOCK_H
#define HITBWEBSOCK_H

#include <openssl/conf.h>
#include <cpprest/ws_client.h>
#include <vector>
#include <string>
#include <shared_mutex>
#include "rapidjson/document.h"
#include "Util.h"

using namespace std;
using namespace rapidjson;

class HITBWebsock
{
private:

  Util util;
  web::websockets::client::websocket_callback_client client;
  void message_handler(web::websockets::client::websocket_incoming_message msg);
  void send_message(string to_send);
  void redisPublish(Value &data, string type, string ts, uint64_t seq);
  string subscribeOrderbook(bool sub);
  vector<double> prices;
  vector<double> sizes;
  shared_timed_mutex price_mut, size_mut;
  string BaseSymbol, QuoteSymbol, Uri;
  string RedisUri, ConnectorID;
  bool is_connected;

public:
  double Best_Buy_Price();
  double Best_Sell_Price();
  double MidMarket_Price();

  void Connect();
  void Disconnect();
  HITBWebsock(string basesymbol, string quotesymbol, string uri, string connectorID, string redisUri);
  ~HITBWebsock();
};
#endif // HITBWEBSOCK_H