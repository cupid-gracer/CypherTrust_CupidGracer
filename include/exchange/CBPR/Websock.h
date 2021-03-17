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
  std::vector<double> buy_prices;
  std::vector<double> sell_prices;
  std::shared_mutex buy_mut, sell_mut;
  std::vector<string> Channels;
  std::vector<string> Product_ids;
  string Uri;
  string redisURL;
  string ConnectorID;
  bool is_connected;

  Util util;

public:
  double Best_Buy_Price();
  double Best_Sell_Price();
  double MidMarket_Price();

  void Connect();
  void Disconnect();
  CBPRWebsock(vector<string> channels, vector<string> product_ids, string uri, string redisurl, string connectorID);
  ~CBPRWebsock();
};
#endif // CBPRWEBSOCK_H