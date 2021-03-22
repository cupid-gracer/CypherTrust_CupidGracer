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

/* Used by HITBAPI::Call to put websource into a string type */
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  ((string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

HITBAPI::HITBAPI()
{
  util = Util();
  curl_global_init(CURL_GLOBAL_DEFAULT);
}

HITBAPI::~HITBAPI()
{
  curl_global_cleanup();
}

/* Uses libcurl to get Data From API */
string HITBAPI::Call(string method, bool authed, string path, string body)
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
      chunk = curl_slist_append(chunk, ("authorization: Basic " + token).c_str());
    }
    res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
    if (method == "POST")
    {
      chunk = curl_slist_append(chunk, "Content-Type: application/x-www-form-urlencoded");
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
      curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, -1L);
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    }
    if (method == "DELETE")
    {
      chunk = curl_slist_append(chunk, "Content-Type: application/x-www-form-urlencoded");
      curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, -1L);
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    }
    if (method == "PUT")
    {
      chunk = curl_slist_append(chunk, "Content-Type: application/x-www-form-urlencoded");
      curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, -1L);
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    }
    if (method == "PATCH")
    {
      chunk = curl_slist_append(chunk, "Content-Type: application/x-www-form-urlencoded");
      curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, -1L);
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    }
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    //set start timestamp before call REST API
    util.setStartTimestamp();
    res = curl_easy_perform(curl);
    //set finish timestamp before call REST API
    util.setFinishTimestamp();
    //publish latency through redis heartbeat channel
    util.publishLatency(redisURL, redisConnectorChannel, addressID, readBuffer.size(), readBuffer.size());

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


//Public
// Available symbols
string HITBAPI::Get_list_symbols(string symbols)
{
  string res = Call("GET", false, "/public/symbol?symbols=" + symbols, "");
  // cout << res << std::endl;
  return res;
}

// Get symbol info
string HITBAPI::Get_symbol_info(string symbol)
{
  string res = Call("GET", false, "/public/symbol/" + symbol, "");
  // cout << res << std::endl;
  return res;
}

// Available currencies
//parameter  ex: ETHBTC,BETBTC...
string HITBAPI::Get_list_currencies(string currencies)
{
  string res = Call("GET", false, "/public/currency?currencies=" + currencies, "");
  cout << res << std::endl;
  return res;
}

// Get currency info
string HITBAPI::Get_currency_info(string currency)
{
  string res = Call("GET", false, "/public/currency/" + currency, "");
  cout << res << std::endl;
  return res;
}

// Get Tickers
//parameter  ex: ETHBTC,BETBTC...
string HITBAPI::Get_list_tickers(string symbols)
{
  string res = Call("GET", false, "/public/ticker?symbols=" + symbols, "");
  // cout << res << std::endl;
  return res;
}

// Get Tickers
string HITBAPI::Get_ticker(string symbol)
{
  string res = Call("GET", false, "/public/ticker/" + symbol, "");
  cout << res << std::endl;
  return res;
}

// Get Trades
string HITBAPI::Get_list_trades(string symbols, string sort, string from, string till, string limit, string offset)
{
  string url = "/public/trades?";
  url += "symbols=" + symbols;
  url += "&sort=" + sort;
  url += "&from=" + from;
  url += "&till=" + till;
  url += "&limit=" + limit;
  url += "&offset=" + offset;

  string res = Call("GET", false, url, "");
  cout << res << std::endl;
  return res;
}

//Get Trade
string HITBAPI::Get_trade(string symbol, string sort, string by, string from, string till, string limit, string offset)
{
  string url = "/public/trades/" + symbol + "?";
  url += "&by=" + by;
  url += "&sort=" + sort;
  url += "&from=" + from;
  url += "&till=" + till;
  url += "&limit=" + limit;
  url += "&offset=" + offset;

  string res = Call("GET", false, url, "");
  cout << res << std::endl;
  return res;
}

//Get list order
string HITBAPI::Get_list_orderbook(string symbols, string limit)
{
  string url = "/public/orderbook?" + symbols + "&limit=" + limit;
  string res = Call("GET", false, url, "");
  cout << res << std::endl;
  return res;
}

//Get order
string HITBAPI::Get_orderbook(string symbol, string limit, string volumn)
{
  string url = "/public/orderbook/" + symbol + "?limit=" + limit + "&volumn=" + volumn;
  string res = Call("GET", false, url, "");
  cout << res << std::endl;
  return res;
}

//Get Candles
string HITBAPI::Get_list_candles(string symbols, string period, string sort, string from, string till, string limit, string offset)
{
  string url = "/public/candles?";
  url += "&symbols=" + symbols;
  url += "&period=" + period;
  url += "&sort=" + sort;
  url += "&from=" + from;
  url += "&till=" + till;
  url += "&limit=" + limit;
  url += "&offset=" + offset;

  string res = Call("GET", false, url, "");
  cout << res << std::endl;
  return res;
}

//Get Candles
string HITBAPI::Get_candle(string symbol, string period, string sort, string from, string till, string limit, string offset)
{
  string url = "/public/candles/" + symbol + "?";
  url += "&period=" + period;
  url += "&sort=" + sort;
  url += "&from=" + from;
  url += "&till=" + till;
  url += "&limit=" + limit;
  url += "&offset=" + offset;

  string res = Call("GET", false, url, "");
  cout << res << std::endl;
  return res;
}




//Trading
//Get List open orders
string HITBAPI::Get_list_orders(string symbol)
{
  string url = "/order?symbol=" + symbol;
  string res = Call("GET", true, url, "");
  cout << res << std::endl;
  return res;
}

//Create new order
string HITBAPI::Create_new_order_post(string symbol, string side, string quantity, string clientOrderId, string type, string timeInForce, string price, string stopPrice, string expireTime, string strictValidate, string postOnly, string takeLiquidityMarkup, string provideLiquidityMarkup)
{
  string url = "/order";
  string body = "";
  body += "symbol=" + symbol;
  body += "&side=" + side;
  body += "&quantity=" + quantity;
  body += "&clientOrderId=" + clientOrderId;
  body += "&type=" + type;
  body += "&timeInForce=" + timeInForce;
  body += "&price=" + price;
  body += "&stopPrice=" + stopPrice;
  body += "&expireTime=" + expireTime;
  body += "&strictValidate=" + strictValidate;
  body += "&postOnly=" + postOnly;
  body += "&takeLiquidityMarkup=" + takeLiquidityMarkup;
  body += "&provideLiquidityMarkup=" + provideLiquidityMarkup;
  string res = Call("POST", true, url, body);
  cout << res << std::endl;
  return res;
}

//Delete order
string HITBAPI::Cancel_all_order(string symbol)
{
  string url = "/order" + symbol;
  string body = "";
  body += "symbol=" + symbol;
  string res = Call("DELETE", true, url, "");
  cout << res << std::endl;
  return res;
}

// order
string HITBAPI::Get_single_order(string clientOrderId, string wait)
{
  string url = "/order/" + clientOrderId + "?wait=" + wait;
  string res = Call("GET", true, url, "");
  cout << res << std::endl;
  return res;
}

//Create new order
string HITBAPI::Create_new_order_put(string clientOrderId, string symbol, string side, string timeInForce, string quantity, string type, string price, string stopPrice, string expireTime, string strictValidate, string postOnly, string takeLiquidityMarkup, string provideLiquidityMarkup)
{
  string url = "/order/" + clientOrderId;
  string body = "";
  body += "symbol=" + symbol;
  body += "&side=" + side;
  body += "&type=" + type;
  body += "&timeInForce=" + timeInForce;
  body += "&quantity=" + quantity;
  body += "&price=" + price;
  body += "&stopPrice=" + stopPrice;
  body += "&expireTime=" + expireTime;
  body += "&strictValidate=" + strictValidate;
  body += "&postOnly=" + postOnly;
  body += "&takeLiquidityMarkup=" + takeLiquidityMarkup;
  body += "&provideLiquidityMarkup=" + provideLiquidityMarkup;
  string res = Call("PUT", true, url, body);
  cout << res << std::endl;
  return res;
}

//Cancel order
string HITBAPI::Cancel_order(string clientOrderId)
{
  string url = "/order/" + clientOrderId;
  string res = Call("DELETE", true, url, "");
  cout << res << std::endl;
  return res;
}

//Cancel Replace order
string HITBAPI::Cancel_replace_order(string clientOrderId, string quantity, string requestClientId, string price)
{
  string url = "/order/" + clientOrderId;
  string body = "";
  body += "&quantity=" + quantity;
  body += "&requestClientId=" + requestClientId;
  body += "&price=" + price;
  string res = Call("PATCH", true, url, body);
  cout << res << std::endl;
  return res;
}

//Get trading balance
string HITBAPI::Get_trading_balance()
{
  string url = "/trading/balance";
  string res = Call("GET", true, url, "");
  cout << res << std::endl;
  return res;
}


//Get trading fee rate
string HITBAPI::Get_trading_fee_rate(string symbol)
{
  string url = "/trading/fee/" + symbol;
  string res = Call("GET", true, url, "");
  cout << res << std::endl;
  return res;
}

//MarginTrading
//List of your currently open margin orders
string HITBAPI::Get_list_margin_orders(string symbol)
{
  string url = "/margin/order?symbol=" + symbol;
  string res = Call("GET", true, url, "");
  cout << res << std::endl;
  return res;
}

//Create new margin orders
string HITBAPI::Create_new_margin_order_post(string symbol, string side, string quantity, string clientOrderId, string type, string timeInForce, string price, string stopPrice, string expireTime, string strictValidate, string postOnly, string takeLiquidityMarkup, string provideLiquidityMarkup)
{
  string url = "/margin/order";
  string body = "";
  body += "symbol=" + symbol;
  body += "&side=" + side;
  body += "&quantity=" + quantity;
  body += "&type=" + type;
  body += "&clientOrderId=" + clientOrderId;
  body += "&timeInForce=" + timeInForce;
  body += "&price=" + price;
  body += "&stopPrice=" + stopPrice;
  body += "&expireTime=" + expireTime;
  body += "&strictValidate=" + strictValidate;
  body += "&postOnly=" + postOnly;
  body += "&takeLiquidityMarkup=" + takeLiquidityMarkup;
  body += "&provideLiquidityMarkup=" + provideLiquidityMarkup;
  string res = Call("POST", true, url, body);
  cout << res << std::endl;
  return res;
}

//Create new margin orders
string HITBAPI::Create_new_margin_order_put(string clientOrderId, string symbol, string side, string timeInForce, string quantity, string type, string price, string stopPrice, string expireTime, string strictValidate, string postOnly, string takeLiquidityMarkup, string provideLiquidityMarkup)
{
  string url = "/margin/order";
  string body = "";
  body += "symbol=" + symbol;
  body += "&side=" + side;
  body += "&quantity=" + quantity;
  body += "&type=" + type;
  body += "&clientOrderId=" + clientOrderId;
  body += "&timeInForce=" + timeInForce;
  body += "&price=" + price;
  body += "&stopPrice=" + stopPrice;
  body += "&expireTime=" + expireTime;
  body += "&strictValidate=" + strictValidate;
  body += "&postOnly=" + postOnly;
  body += "&takeLiquidityMarkup=" + takeLiquidityMarkup;
  body += "&provideLiquidityMarkup=" + provideLiquidityMarkup;
  string res = Call("PUT", true, url, body);
  cout << res << std::endl;
  return res;
}

//Cancel all open margin orders
string HITBAPI::Cancel_all_margin_orders(string symbol)
{
  string url = "/margin/order";
  string body = "";
  body += "symbol=" + symbol;
  string res = Call("DELETE", true, url, body);
  cout << res << std::endl;
  return res;
}

//Get a single margin order by clientOrderId
string HITBAPI::Get_single_margin_order(string clientOrderId)
{
  string url = "/margin/order/" + clientOrderId;
  string res = Call("GET", true, url, "");
  cout << res << std::endl;
  return res;
}

//Cancel a margin order
string HITBAPI::Cancel_single_margin_order(string clientOrderId)
{
  string url = "/margin/order/" + clientOrderId;
  string res = Call("DELETE", true, url, "");
  cout << res << std::endl;
  return res;
}

//Cancel-Replace margin order
string HITBAPI::Cancel_replace_margin_order(string clientOrderId, string quantity, string requestClientId, string price)
{
  string url = "/margin/order/" + clientOrderId;
  string body = "";
  body += "quantity=" + quantity;
  body += "&requestClientId=" + requestClientId;
  body += "&price=" + price;
  string res = Call("PATCH", true, url, body);
  cout << res << std::endl;
  return res;
}

//MarginAccount
//Get list current margin accounts
string HITBAPI::Get_list_margin_accounts()
{
  string url = "/margin/account";
  string res = Call("GET", true, url, "");
  cout << res << std::endl;
  return res;
}

//Close all margin accounts with their positions
string HITBAPI::Close_all_margin_accounts()
{
  string url = "/margin/account";
  string res = Call("DELETE", true, url, "");
  cout << res << std::endl;
  return res;
}

//Get margin account by symbol
string HITBAPI::Get_margin_account(string symbol)
{
  string url = "/margin/account/" + symbol;
  string res = Call("GET", true, url, "");
  cout << res << std::endl;
  return res;
}
//Set up a margin accout and reserve margin balance for it
string HITBAPI::Set_margin_account(string symbol, string marginBalance, string strictValidate)
{
  string url = "/margin/account/" + symbol;
  string body = "";
  body += "marginBalance=" + marginBalance;
  body += "&strictValidate=" + strictValidate;
  string res = Call("PUT", true, url, body);
  cout << res << std::endl;
  return res;
}

//Close the margin account with theire position
string HITBAPI::Close_single_margin_account(string symbol)
{
  string url = "/margin/account/" + symbol;
  string res = Call("DELETE", true, url, "");
  cout << res << std::endl;
  return res;
}

//Get list all open position
string HITBAPI::Get_list_open_position()
{
  string url = "/margin/position";
  string res = Call("GET", true, url, "");
  cout << res << std::endl;
  return res;
}

//Cloase all open position
string HITBAPI::Close_all_open_position()
{
  string url = "/margin/position";
  string res = Call("DELETE", true, url, "");
  cout << res << std::endl;
  return res;
}

//Get open poosition for a symbol
string HITBAPI::Get_single_open_position(string symbol)
{
  string url = "/margin/position/" + symbol;
  string res = Call("GET", true, url, "");
  cout << res << std::endl;
  return res;
}

//Close the position for a symbol
string HITBAPI::Close_single_position(string symbol, string price, string strictValidate)
{
  string url = "/margin/position/" + symbol;
  string body = "";
  body += "price=" + price;
  body += "&strictValidate=" + strictValidate;
  string res = Call("DELETE", true, url, body);
  cout << res << std::endl;
  return res;
}

//TradingHistory
//Get historical trades
string HITBAPI::Get_historical_trades(string symbol, string sort, string by, string from, string till, string limit, string offset, string margin)
{
  string url = "/history/trades?";
  url += "symbol=" + symbol;
  url += "&sort=" + sort;
  url += "&by=" + by;
  url += "&from=" + from;
  url += "&till=" + till;
  url += "&limit=" + limit;
  url += "&offset=" + offset;
  url += "&margin=" + margin;
  string res = Call("GET", true, url, "");
  cout << res << std::endl;
  return res;
}

//Get historical orders
string HITBAPI::Get_historical_orders(string symbol, string from, string till, string limit, string offset, string clientOrderId)
{
  string url = "/history/order?";
  url += "symbol=" + symbol;
  url += "&from=" + from;
  url += "&till=" + till;
  url += "&limit=" + limit;
  url += "&offset=" + offset;
  url += "&clientOrderId=" + clientOrderId;
  string res = Call("GET", true, url, "");
  cout << res << std::endl;
  return res;
}

//Get historical trades by specified order
string HITBAPI::Get_historical_by_orderid(string id)
{
  string url = "/history/order/" + id + "/trades";
  string res = Call("GET", true, url, "");
  cout << res << std::endl;
  return res;
}

//Sub-Accounts
//Get sub-accounts list
string HITBAPI::Get_list_sub_accounts()
{
  string url = "/sub-acc";
  string res = Call("GET", true, url, "");
  cout << res << std::endl;
  return res;
}

//Freeze sub-accounts
string HITBAPI::Freeze_sub_accounts(string ids)
{
  string url = "/sub-acc/freeze";
  string body = "ids=" + ids;
  string res = Call("POST", true, url, body);
  cout << res << std::endl;
  return res;
}

//Active sub-accounts
string HITBAPI::Active_sub_accounts(string ids)
{
  string url = "/sub-acc/activate";
  string body = "ids=" + ids;
  string res = Call("POST", true, url, body);
  cout << res << std::endl;
  return res;
}
//Transfer from main to sub-account or from sub-account to main
string HITBAPI::Transfer_main_sub_account(string subAccountId, string amount, string currency, string type)
{
  string url = "/sub-acc/transfer";
  string body = "subAccountId=" + subAccountId;
  body = "&amount=" + amount;
  body = "&currency=" + currency;
  body = "&type=" + type;
  string res = Call("POST", true, url, body);
  cout << res << std::endl;
  return res;
}

//Sub account withdraw setting list
string HITBAPI::Get_list_withdraw_sub_account(string subAccountIds)
{
  string url = "/sub-acc/acl?";
  url = "subAccountIds=" + subAccountIds;
  string res = Call("GET", true, url, "");
  cout << res << std::endl;
  return res;
}

//disable or enable withdraw for sub-accounts.
string HITBAPI::Switch_status_withdraw_sub_accounts(string subAccountUserID, string descrption, string isPayoutEnabled)
{
  string url = "/sub-acc/acl/:" + subAccountUserID;
  string body = "descrption=" + descrption;
  body = "&isPayoutEnabled=" + isPayoutEnabled;
  string res = Call("PUT", true, url, "");
  cout << res << std::endl;
  return res;
}

//get main and trading sub-account balances
string HITBAPI::Get_main_trading_balances(string subAccountUserID)
{
  string url = "/sub-acc/balance/:" + subAccountUserID;
  string res = Call("GET", true, url, "");
  cout << res << std::endl;
  return res;
}

//Get sub-account crypto address
string HITBAPI::Get_sub_account_crypto_address(string subAccountUserID, string currency)
{
  string url = "/sub-acc/deposit-address/:" + subAccountUserID + "/:" + currency;
  string res = Call("GET", true, url, "");
  cout << res << std::endl;
  return res;
}

//Accounts
//Get main account balance
string HITBAPI::Get_main_account_balance()
{
  string url = "/account/balance";
  string res = Call("GET", true, url, "");
  cout << res << std::endl;
  return res;
}

//Get account transactions
string HITBAPI::Get_list_acount_transactions(string currency, string sort, string by, string from, string till, string limit, string offset)
{
  string url = "/account/transactions?";
  url += "currency=" + currency;
  url += "&sort=" + sort;
  url += "&by=" + by;
  url += "&from=" + from;
  url += "&till=" + till;
  url += "&limit=" + limit;
  url += "&offset=" + offset;
  string res = Call("GET", true, url, "");
  cout << res << std::endl;
  return res;
  
}

//Get account transaction by id
string HITBAPI::Get_single_account_transation(string id)
{
  string url = "/account/transactions/" + id;
  string res = Call("GET", true, url, "");
  cout << res << std::endl;
  return res;
}

//Withdraw crypto
string HITBAPI::Withdraw_crypto(string currency, string amount, string address, string paymentId, string includeFee, string autoCommit, string useOffchain, string feeLevelId, string publicComment)
{
  string url = "/account/crypto/withdraw";
  string body = "";
  body += "currency=" + currency;
  body += "&amount=" + amount;
  body += "&address=" + address;
  body += "&paymentId=" + paymentId;
  body += "&includeFee=" + includeFee;
  body += "&autoCommit=" + autoCommit;
  body += "&useOffchain=" + useOffchain;
  body += "&feeLevelId=" + feeLevelId;
  body += "&publicComment=" + publicComment;
  string res = Call("POST", true, url, body);
  cout << res << std::endl;
  return res;
  
}

//Commit withdraw crypto
string HITBAPI::Commit_withdraw_crypto(string id)
{
  string url = "/account/crypto/withdraw/" + id;
  string res = Call("PUT", true, url, "");
  cout << res << std::endl;
  return res;
}

//Rollback withdraw crypto
string HITBAPI::Rollback_withdraw_crypto(string id)
{
  string url = "/account/crypto/withdraw/" + id;
  string res = Call("DELETE", true, url, "");
  cout << res << std::endl;
  return res;
}

//Check that offchain payout is avaliable for a destination address
string HITBAPI::Check_offchain_payout(string currency, string address, string paymentId)
{
  string url = "/account/crypto/check-offchain-available";
  string body = "";
  body += "currency=" + currency;
  body += "&address=" + address;
  body += "&paymentId=" + paymentId;
  string res = Call("POST", true, url, body);
  cout << res << std::endl;
  return res;
}

//transer convert between currencies
string HITBAPI::Transfer_convert_between_currencies(string fromCurrency, string toCurrency, string amount)
{
  string url = "/account/crypto/transfer-convert";
  string body = "";
  body += "fromCurrency=" + fromCurrency;
  body += "&toCurrency=" + toCurrency;
  body += "&amount=" + amount;
  string res = Call("POST", true, url, body);
  cout << res << std::endl;
  return res;
}

//Estimate fee for withdraw
string HITBAPI::Estimate_fee_withdraw(string currency, string amount)
{
  string url = "/account/crypto/estimate-withdraw?currency=" + currency + "&amount=" + amount;
  string res = Call("GET", true, url, "");
  cout << res << std::endl;
  return res;
}

//Estimates fee levels for withdraw
string HITBAPI::Estimate_fee_level_withdraw(string currency, string amount)
{
  string url = "/account/crypto/estimate-withdraw-levels?currency=" + currency + "&amount=" + amount;
  string res = Call("GET", true, url, "");
  cout << res << std::endl;
  return res;
}

//Check if crypto address belongs to current user
string HITBAPI::Check_crypto_address_current_user(string address)
{
  string url = "/account/crypto/is-mine/" + address;
  string res = Call("GET", true, url, "");
  cout << res << std::endl;
  return res;
}

//Get deposit crypto address
string HITBAPI::Get_deposit_crypto_address(string currency)
{
  string url = "/account/crypto/address/" + currency;
  string res = Call("GET", true, url, "");
  cout << res << std::endl;
  return res;
}

//Create new deposit crypto address
string HITBAPI::Create_new_deposit_crypto_address(string currency)
{
  string url = "/account/crypto/address/" + currency;
  string res = Call("POST", true, url, "");
  cout << res << std::endl;
  return res;
}

//Get last 10 deposit crypto addresses
string HITBAPI::Get_last10_deposit_crypto_addresses(string currency)
{
  string url = "/account/crypto/addresses/" + currency;
  string res = Call("GET", true, url, "");
  cout << res << std::endl;
  return res;
}

//Get last 10 unique payout addresses
string HITBAPI::Get_last10_unique_payout_addresses(string currency)
{
  string url = "/account/crypto/used-addresses/" + currency;
  string res = Call("GET", true, url, "");
  cout << res << std::endl;
  return res;
}

//Transfer amount to trading 
string HITBAPI::Transfer_amount_trading(string currency, string amount, string type)
{
  string url = "/account/transfer";
  string body = "";
  body += "currency=" + currency;
  body += "&amount=" + amount;
  body += "&type=" + type;
  string res = Call("POST", true, url, body);
  cout << res << std::endl;
  return res;
}

//Transfer money to another user by email or username 
string HITBAPI::Transfer_money_another(string currency, string amount, string by, string identifier)
{
  string url = "/account/transfer/internal";
  string body = "";
  body += "currency=" + currency;
  body += "&amount=" + amount;
  body += "&by=" + by;
  body += "&identifier=" + identifier;
  string res = Call("POST", true, url, body);
  cout << res << std::endl;
  return res;
}
