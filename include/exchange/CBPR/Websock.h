// Websock.h
#ifndef CBPRWEBSOCK_H
#define CBPRWEBSOCK_H

#include <json/json.h>
#include <json/value.h>
#include <sw/redis++/redis++.h>

#include <cpprest/ws_client.h>
#include <vector>
#include <string>
#include <API.h>
#include <Util.h>

using namespace std;

class CBPRWebsock
{
public:
  CBPRWebsock();
  CBPRWebsock(API app_api, vector<string> channels, vector<string> product_ids, string wssURL, string redisurl, string connectorID, string redisOrderBookChannel, string redisConnectorChannel);
  ~CBPRWebsock();


  void Connect();
  void Run();
  void Disconnect();
  
  web::websockets::client::websocket_callback_client client;
  void message_handler(web::websockets::client::websocket_incoming_message msg);
  void message_process(web::websockets::client::websocket_incoming_message msg);
  void send_message(string to_send);
  void redisPublishStartOrStop(string type);
  void StopOrder();
  void sendToRedis(string context, string market, string sideclass, string price, string volume, uint64_t sequence, long exchangeTimestamp);
  void snapshotMass(string context, string market, string bids, string asks);
  API app_api;
  string subscribe(bool sub);

  vector<string> Channels;
  vector<string> Product_ids;
  vector<string> Offline_symbols;
  string wssURL;
  string redisURL;
  string ConnectorID;
  bool is_connected;
  string redisOrderBookChannel, redisConnectorChannel;
  Util util;

};
#endif // CBPRWEBSOCK_H

