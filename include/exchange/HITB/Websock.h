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
  void redisPublishOrder(Value &data, string type, string ts, uint64_t seq);
  void redisPublishStartOrStop(string type);
  void StopOrder(); // publish stop order through redis order channel
  string subscribeOrderbook(bool sub);
  int* signal_status;
  vector<double> prices;
  vector<double> sizes;
  // shared_timed_mutex price_mut, size_mut;
  string BaseSymbol, QuoteSymbol, wssURL;
  string RedisUri, ConnectorID;
  string redisOrderBookChannel, redisConnectorChannel;
  bool is_connected = false;

public:

  HITBWebsock();
  HITBWebsock(string basesymbol, string quotesymbol, string wssURL, string connectorID, string redisUri, string redisOrderBookChannel, string redisConnectorChannel);
  ~HITBWebsock();

  double Best_Buy_Price();
  double Best_Sell_Price();
  double MidMarket_Price();
  void HeartbeatThread();

  void Connect();
  void Disconnect();
  
};
#endif // HITBWEBSOCK_H