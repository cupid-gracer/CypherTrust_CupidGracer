// Websock.h
#ifndef CBPRWEBSOCK_H
#define CBPRWEBSOCK_H

#include <openssl/conf.h>
#include <cpprest/ws_client.h>
#include <vector>
#include <string>
#include <shared_mutex>
#include "Util.h"

using namespace std;

class CBPRWebsock
{
private:
  web::websockets::client::websocket_callback_client client;
  void message_handler(web::websockets::client::websocket_incoming_message msg);
  void send_message(string to_send);
  string subscribe(bool sub);
  vector<double> buy_prices;
  vector<double> sell_prices;
  vector<string> Channels;
  vector<string> Product_ids;
  vector<string> Offline_symbols;
  string wssURL;
  string redisURL;
  string ConnectorID;
  bool is_connected;
  string redisOrderBookChannel, redisConnectorChannel;
  Util util;

public:

  void Connect();
  void Disconnect();
  CBPRWebsock(vector<string> channels, vector<string> product_ids, string wssURL, string redisurl, string connectorID, string redisOrderBookChannel, string redisConnectorChannel);
  ~CBPRWebsock();
};
#endif // CBPRWEBSOCK_H