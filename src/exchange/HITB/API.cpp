#include "exchange/HITB/API.h"
#include "Util.h"
#include <vector>
#include <iostream>
#include "curl/curl.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;
using namespace std;

/* Used by API::Call to put websource into a string type */
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  ((string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

API::API()
{
  curl_global_init(CURL_GLOBAL_DEFAULT);
}

API::~API()
{
  curl_global_cleanup();
}

/* Uses libcurl to get Data From API */
string API::Call(string method, bool authed, string path, string body)
{
  CURL *curl;
  CURLcode res;
  string readBuffer;
  curl = curl_easy_init();
  if (curl)
  {
    struct curl_slist *chunk = NULL;
    curl_easy_setopt(curl, CURLOPT_URL, (uri + path).c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl/1.0");
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    chunk = curl_slist_append(chunk, "accept: application/json");
    if (authed)
    {
      chunk = curl_slist_append(chunk, ("Authorization: Bearer " + access_token).c_str());
    }
    res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
    if (method == "POST")
    {
      chunk = curl_slist_append(chunk, "Content-Type: application/x-www-form-urlencoded");
      curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, -1L);
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    }
    if (method == "DELETE")
    {
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    }
    if (method == "PUT")
    {
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
    }
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    res = curl_easy_perform(curl);
    /* Check for errors */
    if (res != CURLE_OK)
      std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
    /* always cleanup */
    curl_easy_cleanup(curl);
    /* free the custom headers */
    curl_slist_free_all(chunk);
  }
  return readBuffer;
}

string API::Get_Buy_Price()
{
  string ret;
  string st = Call("GET", false, "/products/" + product_id + "/book", "");
  Document d;
  d.Parse(st.c_str());
  if (d.HasMember("message"))
  {
    assert(d["message"].IsString());
    std::cout << "(Get_Buy_Price) Message: " << d["message"].GetString() << std::endl;
  }
  if (d.HasMember("bids"))
  {
    assert(d["bids"].IsArray());
    const Value &c = d["bids"];
    assert(c[0].IsArray());
    const Value &b = c[0];
    assert(b[0].IsString());
    ret = b[0].GetString();
  }
  return ret;
}

double API::Get_Balance(string currency)
{
  double ret = 0;
  string txt = Call("GET", true, "/accounts", "");
  Document d;
  d.Parse(txt.c_str());
  assert(d.IsArray());
  for (SizeType i = 0; i < d.Size(); i++)
  {
    assert(d[i].HasMember("currency"));
    assert(d[i]["currency"].IsString());
    string cur = d[i]["currency"].GetString();
    if (cur == currency)
    {
      assert(d[i].HasMember("available"));
      assert(d[i]["available"].IsString());
      ret = std::stod(d[i]["available"].GetString());
      return ret;
    }
  }
  return ret;
}

string API::Place_Limit_Order(string side, string price, string size)
{
  string order_id;
  Document d;
  d.SetObject();
  rapidjson::Document::AllocatorType &allocator = d.GetAllocator();

  /* adds type */
  d.AddMember("type", "limit", allocator);

  /* Adds the Size */
  Value v_side;
  v_side = StringRef(side.c_str());
  d.AddMember("side", v_side, allocator);

  /* Adds the Product id */
  Value v_product_id;
  v_product_id = StringRef(product_id.c_str());
  d.AddMember("product_id", v_product_id, allocator);

  /* Adds the price (USD) */
  Value v_price;
  v_price = StringRef(price.c_str());
  d.AddMember("price", v_price, allocator);

  /* Adds the size (BTC, ETH, LTC) */
  Value v_size;
  v_size = StringRef(size.c_str());
  d.AddMember("size", v_size, allocator);

  /* Gets the Order to be a maker and not a taker */
  d.AddMember("post_only", true, allocator);

  /* creates the string in json */
  StringBuffer strbuf;
  Writer<StringBuffer> writer(strbuf);
  d.Accept(writer);
  string returned = Call("POST", true, "/orders", strbuf.GetString());
  // std::cout << "returned = " << returned << std::endl;
  // std::cout << "returned strbuf.GetString = " << strbuf.GetString() << std::endl;
  Document d_1;
  d_1.Parse(returned.c_str());
  // std::cout << "returned.c_str() = " << returned.c_str() << std::endl;
  if (d_1.HasMember("id"))
  {
    assert(d_1["id"].IsString());
    order_id = d_1["id"].GetString();
  }
  if (d_1.HasMember("message"))
  {
    assert(d_1["message"].IsString());
    std::cout << "(Limit_Order) Message: " << d_1["message"].GetString() << std::endl;
  }
  return order_id;
}

//Public
// Available Currencies
string API::Get_list_currencies()
{
  string res = Call("GET", false, "/public/currencies", "");
  std::cout << res << std::endl;
  return "";
}

// Get currency info
string API::Get_currency_info(string currencyId)
{
  string url = "/public/currencies/" + currencyId;
  string res = Call("GET", false, url, "");
  std::cout << res << std::endl;
  return "";
}

// Available markets
string API::Get_list_markets()
{
  string url = "/public/markets/";
  string res = Call("GET", false, url, "");
  std::cout << res << std::endl;
  return "";
}

// Get list of all avialable currency pairs groups
string API::Get_list_currency_pairs_groups()
{
  string url = "/public/pairs-groups";
  string res = Call("GET", false, url, "");
  std::cout << res << std::endl;
  return "";
}

// Available currency pairs
string API::Get_currency_pairs(string code)
{
  string url = "/public/currency_pairs/list/" + code;
  string res = Call("GET", false, url, "");
  std::cout << res << std::endl;
  return "";
}

// Available currency pairs for a given group
string API::Get_currency_pairs_given_group(string currencyPairGroupId)
{
  string url = "/public/currency_pairs/group/" + currencyPairGroupId;
  string res = Call("GET", false, url, "");
  std::cout << res << std::endl;
  return "";
}

// Get currency pair information
string API::Get_currency_pair_information(string currencyPairId)
{
  string url = "/public/currency_pairs/" + currencyPairId;
  string res = Call("GET", false, url, "");
  std::cout << res << std::endl;
  return "";
}

// Tickers list for all currency pairs
string API::Get_ticker_list_currency_pairs()
{
  string url = "/public/ticker";
  string res = Call("GET", false, url, "");
  // std::cout << res << std::endl;
  return res;
}

// Ticker for currency pair
string API::Get_ticker_currency_pair(string currencyPairId)
{
  string url = "/public/ticker/" + currencyPairId;
  string res = Call("GET", false, url, "");
  std::cout << res << std::endl;
  return "";
}

// Trades for given currency pair
string API::Get_trades_currency_pair(string currencyPairId, string sort, string from, string till, string limit, string offset)
{
  string url = "/public/trades/" + currencyPairId;
  url += "?sort=" + sort;
  url += "&from=" + from;
  url += "&till=" + till;
  url += "&limit=" + limit;
  string res = Call("GET", false, url, "");
  std::cout << res << std::endl;
  return "";
}

// Orderbook for given currency pair
string API::Get_orderbook_currency_pair(string currencyPairId, string limit_bids, string limit_asks)
{
  string url = "/public/orderbook/" + currencyPairId;
  url += "?limit_bids=" + limit_bids;
  url += "&limit_asks=" + limit_asks;
  string res = Call("GET", false, url, "");
  std::cout << res << std::endl;
  return "";
}

// A list of candles for given currency pair
string API::Get_list_candles_currency_pair(string currencyPairId, string candlesType, string timeStart, string timeEnd, string limit, string offset)
{
  string url = "/public/trades/" + currencyPairId + "/" + candlesType;
  url += "?timeStart=" + timeStart;
  url += "&timeEnd=" + timeEnd;
  url += "&limit=" + limit;
  url += "&offset=" + offset;
  string res = Call("GET", false, url, "");
  std::cout << res << std::endl;
  return "";
}

// Available Deposit Statuses
string API::Get_list_deposit_statuses()
{
  string url = "/public/deposit-statuses";
  string res = Call("GET", false, url, "");
  std::cout << res << std::endl;
  return "";
}

// Get deposit status info
string API::Get_deposit_status(string statusId)
{
  string url = "/public/deposit-statuses/" + statusId;
  string res = Call("GET", false, url, "");
  std::cout << res << std::endl;
  return "";
}

// Available Withdrawal Statuses
string API::Get_list_withdrawal_statuses()
{
  string url = "/public/withdrawal-statuses";
  string res = Call("GET", false, url, "");
  std::cout << res << std::endl;
  return "";
}

// Get status info
string API::Get_withdrawal_status(string statusId)
{
  string url = "/public/withdrawal-statuses/" + statusId;
  string res = Call("GET", false, url, "");
  std::cout << res << std::endl;
  return "";
}

// Test API is working and get server time
string API::ping()
{
  string url = "/public/ping";
  string res = Call("GET", false, url, "");
  std::cout << res << std::endl;
  return "";
}

// Shows the official mobile applications data
string API::Get_mobile_version()
{
  string url = "/public/mobile-versions";
  string res = Call("GET", false, url, "");
  std::cout << res << std::endl;
  return "";
}

// Get the last 20 posts (stex.com) on Twitter
string API::Get_twitter()
{
  string url = "/public/twitter";
  string res = Call("GET", false, url, "");
  std::cout << res << std::endl;
  return "";
}

//Trading
//Returns the user's fees for a given currency pair
string API::Get_user_fee_currency_pair(string currencyPairId)
{
  string url = "/trading/fees/" + currencyPairId;
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//List your currently open orders
string API::Get_list_open_all_orders(string limit, string offset)
{
  string url = "/trading/orders";
  url += "?limit=" + limit;
  url += "&offset=" + offset;
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Delete all active orders
string API::Delete_all_active_orders()
{
  string url = "/trading/orders/";
  string res = Call("DELETE", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//List your currently open orders for given currency pair
string API::Get_list_open_order_by_currency_pair(string currencyPairId, string limit, string offset)
{
  string url = "/trading/orders/" + currencyPairId;
  url += "?limit=" + limit;
  url += "&offset=" + offset;
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Delete active orders for given currency pair
string API::Delete_order_by_currency_pair(string currencyPairId)
{
  string url = "/trading/orders/" + currencyPairId;
  string res = Call("DELETE", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Create new order and put it to the orders processing queue
string API::Creat_new_order(string currencyPairId, string type, string amount, string price, string trigger_price)
{
  string url = "/trading/orders/" + currencyPairId;
  string body = "type=" + type;
  body += "&amount=" + amount;
  body += "&price=" + price;
  body += "&trigger_price=" + trigger_price;
  string res = Call("POST", true, url, body);

  std::cout << res << std::endl;
  return "";
}

//Get a single order
string API::Get_single_order(string orderId)
{
  string url = "/trading/orders/" + orderId;
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Cancel order
string API::Delete_order(string orderId)
{
  string url = "/trading/orders/" + orderId;
  string res = Call("DELETE", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Get a list of currencies user had any activity in
string API::Get_list_currency_user_activity(string key)
{
  string url = "/reports/currencies?key=" + key;
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Gets the list of currency pairs the user had orders in for all the time
string API::Get_list_all_currencypairs_by_user()
{
  string url = "/reports/currency_pairs";
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Get past orders
string API::Get_past_orders(string currencyPairId, string orderStatus, string timeStart, string timeEnd, string limit, string offset)
{
  string url = "/reports/orders";
  url += "?currencyPairId=" + currencyPairId;
  url += "&orderStatus=" + orderStatus;
  url += "&timeStart=" + timeStart;
  url += "&timeEnd=" + timeEnd;
  url += "&limit=" + limit;
  url += "&offset=" + offset;
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Get specified order details
string API::Get_order_details(string orderId)
{
  string url = "/reports/orders/" + orderId;
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Get a list of user trades according to request parameters
string API::Get_list_user_spec_trades(string currencyPairId, string timeStart, string timeEnd, string limit, string offset)
{
  string url = "/reports/trades";
  url += "?currencyPairId=" + currencyPairId;
  url += "&timeStart=" + timeStart;
  url += "&timeEnd=" + timeEnd;
  url += "&limit=" + limit;
  url += "&offset=" + offset;
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Get reports list for category
string API::Get_reports_list_category(string listMode, string limit, string offset)
{
  string url = "/reports/background/" + listMode;
  url += "&limit=" + limit;
  url += "&offset=" + offset;
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Get some report info
string API::Get_report_info(string id)
{
  string url = "/reports/background/" + id;
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Remove report by id
string API::Delete_report_by_id(string id)
{
  string url = "/reports/background/" + id;
  string res = Call("DELETE", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Create new report
string API::Create_new_report(string name, string date_from, string date_to, string format, string type)
{
  string url = "/reports/background/create";
  string body = "name=" + name;
  body += "&date_from=" + date_from;
  body += "&date_to=" + date_to;
  body += "&format=" + format;
  body += "&type=" + type;
  string res = Call("POST", true, url, body);

  std::cout << res << std::endl;
  return "";
}

//Get file by id
string API::Get_file_by_id(string id)
{
  string url = "/reports/background/download/" + id;
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Settings
//User event notification settings
string API::Get_list_notification_by_event(string event)
{
  string url = "/settings/notifications/" + event;
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//User event notification settings
string API::Get_list_notification()
{
  string url = "/settings/notifications";
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Set notification settings
/*
    event *     string(query)	An event name you want to subscribe.
    channel *   string(query)	A channel name you want to receive the notification through.
    value *     integer(query)	1 - to subscribe to the notifications of the given event in the specified channel, 0 - to remove subscription of the specified event in the specified channel 
    Available values : 0, 1
    */
string API::Set_notification_settings(string event, string channel, string value)
{
  string url = "/settings/notifications?";
  url += "event=" + event;
  url += "&channel=" + channel;
  url += "&value=" + value;
  string res = Call("PUT", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Set notification settings
string API::Set_notification_settings_one_request()
{
  string url = "/settings/notifications/set";
  string res = Call("PUT", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Profile
//Account Information
/*
    show_balances   string(query)	if present and is > 0, the response will include the 'approx_balance' section. It will be returned as null if this parameter is not present or is not positive number
    */
string API::Get_accoutn_information(string show_balances)
{
  string url = "/profile/info?show_balances=" + show_balances;
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Get a list of user wallets
/*
    sort string(query)	Sort direction. Available values : DESC, ASC  Default value : DESC 
    sortBy  string(query)	Sort direction.    Available values : BALANCE, FROZEN, BONUS, HOLD, TOTAL  Default value : BALANCE
    */
string API::Get_list_user_wallets(string sort, string sortBy)
{
  string url = "/profile/wallets";
  url += "?sort=" + sort;
  url += "&sortBy=" + sortBy;
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Single wallet information
string API::Get_single_user_wallet(string walletId)
{
  string url = "/profile/wallets/" + walletId;
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Burns the given wallet
string API::Burn_wallet(string walletId)
{
  string url = "/profile/wallets/burn/" + walletId;
  string res = Call("POST", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Create a wallet for given currency
/*
    protocol_id integer(query)	Default value : The value that represents legacy protocol (in case of USDT it is 10 as Tether OMNI was default before multi-currency approach used). The list of values can be obtained from the /public/currencies/{currencyId} endpoint that returns the list of all available protocols for a given currency
    */
string API::Create_wallet(string currencyId, string protocol_id)
{
  string url = "/profile/wallets/" + currencyId;
  if (protocol_id != "")
    url += "?protocol_id=" + protocol_id;
  string res = Call("POST", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Get deposit address for given wallet
/*
    protocol_id integer(query)	Default value : The value that represents legacy protocol (in case of USDT it is 10 as Tether OMNI was default before multi-currency approach used). The list of values can be obtained from the /public/currencies/{currencyId} endpoint that returns the list of all available protocols for a given currency
    */
string API::Get_deposit_address_wallet(string walletId, string protocol_id)
{
  string url = "/profile/wallets/address/" + walletId;
  if (protocol_id != "")
    url += "?protocol_id=" + protocol_id;
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Create new deposit address
/*
    protocol_id integer(query)	Default value : The value that represents legacy protocol (in case of USDT it is 10 as Tether OMNI was default before multi-currency approach used). The list of values can be obtained from the /public/currencies/{currencyId} endpoint that returns the list of all available protocols for a given currency
    */
string API::Create_new_deposit_address(string walletId, string protocol_id)
{
  string url = "/profile/wallets/address/" + walletId;
  if (protocol_id != "")
    url += "?protocol_id=" + protocol_id;
  string res = Call("POST", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Get a list of deposits made by user
/*
    currencyId  integer(query)	the ID of the currency to filter results by
    sort    string(query)	Sort direction. Results are always ordered by date. You may adjust the order direction here    
    Available values : DESC, ASC    Default value : DESC
    timeStart   integer(query)	Timestamp in seconds
    timeEnd     integer(query)	Timestamp in seconds
    limit       integer(query)	Default value : 100
    offset      integer(query)	
    */
string API::Get_list_user_deposit(string currencyId, string sort, string timeStart, string timeEnd, string limit, string offset)
{
  string url = "/profile/deposits?";
  if (currencyId != "")
    url += "currencyId=" + currencyId + "&";
  if (sort != "")
    url += "sort=" + sort + "&";
  if (timeStart != "")
    url += "timeStart=" + timeStart + "&";
  if (timeEnd != "")
    url += "timeEnd=" + timeEnd + "&";
  if (limit != "")
    url += "limit=" + limit + "&";
  if (offset != "")
    url += "offset=" + offset;
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Get deposit by id
string API::Get_deposit(string id)
{
  string url = "/profile/deposits/" + id;
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Get a list of rewards obtained by user (e.g. in trading competitions)
/*
    currencyId  integer(query)	the ID of the currency to filter results by
    sort    string(query)	Sort direction. Results are always ordered by date. You may adjust the order direction here    
    Available values : DESC, ASC    Default value : DESC
    timeStart   integer(query)	Timestamp in seconds
    timeEnd     integer(query)	Timestamp in seconds
    limit       integer(query)	Default value : 100
    offset      integer(query)	
    */
string API::Get_list_rewards(string currencyId, string sort, string timeStart, string timeEnd, string limit, string offset)
{
  string url = "/profile/rewards?";
  if (currencyId != "")
    url += "currencyId=" + currencyId + "&";
  if (sort != "")
    url += "sort=" + sort + "&";
  if (timeStart != "")
    url += "timeStart=" + timeStart + "&";
  if (timeEnd != "")
    url += "timeEnd=" + timeEnd + "&";
  if (limit != "")
    url += "limit=" + limit + "&";
  if (offset != "")
    url += "offset=" + offset;
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Get reward by id
string API::Get_reward(string id)
{
  string url = "/profile/rewards/" + id;
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Get a list of user address book items
string API::Get_list_user_address()
{
  string url = "/profile/addressbook/";
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Single address book item
string API::Get_single_address(string itemId)
{
  string url = "/profile/addressbook/" + itemId;
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Deletes address book item
string API::Delete_address_book(string itemId)
{
  string url = "/profile/addressbook/" + itemId;
  string res = Call("DELETE", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Disables the address book item
string API::Disable_address_book_item(string itemId)
{
  string url = "/profile/addressbook/disable_item/" + itemId;
  string res = Call("POST", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Enable the address book item
string API::Enable_address_book_item(string itemId)
{
  string url = "/profile/addressbook/enable_item/" + itemId;
  string res = Call("POST", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Restrict the withdrawals to only addresses that are active in addressbook
string API::Restrict_withdrawal_addressbook()
{
  string url = "/profile/addressbook/enable_strict_wd/";
  string res = Call("POST", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Remove restriction to withdraw to only addresses that are active in addressbook. E.g. allow to withdraw to any address.
string API::Allow_withdrawal_addressbook()
{
  string url = "/profile/addressbook/disable_strict_wd/";
  string res = Call("POST", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Get a list of withdrawals made by user
/*
    currencyId  integer(query)	the ID of the currency to filter results by
    sort    string(query)	Sort direction. Results are always ordered by date. You may adjust the order direction here    
    Available values : DESC, ASC    Default value : DESC
    timeStart   integer(query)	Timestamp in seconds
    timeEnd     integer(query)	Timestamp in seconds
    limit       integer(query)	Default value : 100
    offset      integer(query)	
    */
string API::Get_list_withdrawal(string currencyId, string sort, string timeStart, string timeEnd, string limit, string offset)
{
  string url = "/profile/withdrawals?";
  if (currencyId != "")
    url += "currencyId=" + currencyId + "&";
  if (sort != "")
    url += "sort=" + sort + "&";
  if (timeStart != "")
    url += "timeStart=" + timeStart + "&";
  if (timeEnd != "")
    url += "timeEnd=" + timeEnd + "&";
  if (limit != "")
    url += "limit=" + limit + "&";
  if (offset != "")
    url += "offset=" + offset;
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Get withdrawal by id
string API::Get_withdrawal(string id)
{
  string url = "/profile/withdrawals/" + id;
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Create withdrawal request
/*
    protocol_id integer     This optional parameter has to be used only for multicurrency wallets (for example for USDT). The list of possible values can be obtained in wallet address for such a currency
    additional_address_parameter    string  If withdrawal address requires the payment ID or some key or destination tag etc pass it here
    one_time_code   string  Optional Google Authenticator one-time code
    */
string API::Create_withdrawal_request(string currencyId, string amount, string address, string protocol_id, string additional_address_parameter, string one_time_code)
{
  string url = "/profile/withdraw";
  string body = "";
  body += "currency_id=" + currencyId + "&";
  body += "amount=" + amount + "&";
  body += "address=" + address + "&";
  if (protocol_id != "")
    body += "protocol_id=" + protocol_id + "&";
  if (additional_address_parameter != "")
    body += "additional_address_parameter=" + additional_address_parameter + "&";
  if (one_time_code != "")
    body += "one_time_code=" + one_time_code;
  string res = Call("POST", true, url, body);
  std::cout << res << std::endl;
  return "";
}

//Cancel unconfirmed withdrawal
string API::Cancel_unconfirmed_withdrawal(string withdrawalId)
{
  string url = "/profile/withdraw/" + withdrawalId;
  string res = Call("DELETE", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Get notifications
string API::Get_notifications(string limit , string offset )
{
  string url = "/profile/notifications";
  url += "?limit=" + limit;
  url += "&offset=" + offset;
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Get a list of active price alerts
string API::Get_list_active_price_alert(string currencyPairId)
{
  string url = "/profile/notifications/price?" + currencyPairId;
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Create new price alert
/*
    comparison *    string      One of the 'GREATER' or 'LESS'
    */
string API::Create_new_price_alert(string currencyPairId, string comarison, string price)
{
  string url = "/profile/notifications/price";
  string body = "";
  body += "currencyPairId=" + currencyPairId;
  body += "&comarison=" + comarison;
  body += "&price=" + price;
  string res = Call("POST", true, url, body);
  std::cout << res << std::endl;
  return "";
}

//Delete the price alert by ID
string API::Delete_price_alert(string priceAlertId)
{
  string url = "/profile/notifications/price/" + priceAlertId;
  string res = Call("DELETE", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Create referral program
string API::Create_referral_program()
{
  string url = "/profile/referral/program";
  string res = Call("POST", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Insert referral code
string API::Insert_referral_code(string code)
{
  string url = "/profile/referral/insert/" + code;
  string res = Call("POST", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Transfer referral bonuses balance to main balance for given currency
string API::Transfer_referral_bonuses(string currencyId)
{
  string url = "/profile/referral/bonus_transfer/" + currencyId;
  string res = Call("POST", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Get favorite currency pairs
string API::Get_fav_currency_pair()
{
  string url = "/profile/favorite/currency_pairs";
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Set favorite currency pairs
/*
    addPairIds  array[integer](query)	add ids of currency pairs to list  (1,2,3....)
    removePairIds   array[integer](query)	remove ids of currency pairs from list (1,2,3....)
    show        boolean(query)
    */
string API::Set_fav_currency_pair(string addPairIds, string removePairIds, string show)
{
  string body = "";
  std::vector<string> v_addPairIds;
  std::vector<string> v_removePairIds;
  util.split(addPairIds, ',', v_addPairIds);
  util.split(removePairIds, ',', v_removePairIds);
  for(int i = 0; i < v_addPairIds.size(); i ++){
    body += "addPairIds=" + v_addPairIds[i] + "&";
  }
  for(int i = 0; i < v_removePairIds.size(); i ++){
    body += "removePairIds=" + v_removePairIds[i] + "&";
  }
    body += "show=" + show;

  string url = "/profile/favorite/currency_pairs/set";
  string res = Call("PUT", true, url, "");
  std::cout << res << std::endl;
  return "";
}

//Get current token scopes
string API::Get_current_token_scopes()
{
  string url = "/profile/token-scopes";
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return "";
}
