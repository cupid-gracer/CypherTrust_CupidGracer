// Websock.h
#ifndef EXMOWEBSOCK_H
#define EXMOWEBSOCK_H

#include <openssl/conf.h>
#include <cpprest/ws_client.h>
#include <vector>
#include <string>
#include <shared_mutex>
#include "rapidjson/document.h"
#include <API.h>
#include "Util.h"

using namespace std;
using namespace rapidjson;

class EXMOWebsock
{
private:

  API app_api;
  Util util;
  web::websockets::client::websocket_callback_client client;

  void message_handler(web::websockets::client::websocket_incoming_message msg);
  void message_process(web::websockets::client::websocket_incoming_message msg);
  void send_message(string to_send);
  void redisPublishOrder(Value &data, string type, long ts, uint64_t seq, string market, string streamingType);
  void sendToRedis(string context, string market, string sideclass, string price, string volume, uint64_t sequence, long exchangeTimestamp);
  void snapshotMass(string context, string market, string bids, string asks);
  void redisPublishStartOrStop(string type);
  void StopOrder(); // publish stop order through redis order channel
  int findIndex(string symbol);
  string subscribeOrderbook(bool sub, string sub_symbol);
  vector<double> prices;
  vector<double> sizes;
  // shared_timed_mutex price_mut, size_mut;
  vector<string> CypherTrust_symbols;
  vector<string> EXMO_symbols;
  vector<string> EXMO_subscribe_symbols;
  string RedisUri, ConnectorID, wssURL;
  string redisOrderBookChannel, redisConnectorChannel;
  bool is_connected = false;

public:

  EXMOWebsock();
  EXMOWebsock(API app_api, vector<string> cypherTrust_symbols, string wssURL, string connectorID, string redisUri, string redisOrderBookChannel, string redisConnectorChannel);
  ~EXMOWebsock();

  double Best_Buy_Price();
  double Best_Sell_Price();
  double MidMarket_Price();
  void HeartbeatThread();

  void Connect();
  void Disconnect();
  
};
#endif // EXMOWEBSOCK_H