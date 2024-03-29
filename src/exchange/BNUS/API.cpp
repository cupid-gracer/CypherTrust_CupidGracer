#include "exchange/BNUS/API.h"
#include <iostream>
#include "curl/curl.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;
using namespace std;

/* Used by BNUSAPI::Call to put websource into a string type */
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  ((string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

/* Uses libcurl to get Data From API */
string BNUSAPI::Call(string method, bool authed, string path, string body)
{
  CURL *curl;
  CURLcode res;
  string readBuffer;
  curl = curl_easy_init();
  try
  {
    if (curl)
    {
      struct curl_slist *chunk = NULL;
      curl_easy_setopt(curl, CURLOPT_URL, (uri + path).c_str());
      curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl/1.0");
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
      chunk = curl_slist_append(chunk, "Content-Type: application/json");
      if (authed)
      {
        // string time_stamp = auth.GetTimestamp();
        // string sign = auth.Sign(time_stamp, method, path, body);
        // chunk = curl_slist_append(chunk, ("CB-ACCESS-KEY: " + auth.Key).c_str());
        // chunk = curl_slist_append(chunk, ("CB-ACCESS-SIGN: " + sign).c_str());
        // chunk = curl_slist_append(chunk, ("CB-ACCESS-TIMESTAMP: " + time_stamp).c_str());
        // chunk = curl_slist_append(chunk, ("CB-ACCESS-PASSPHRASE: " + auth.Passphrase).c_str());
      }
      res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
      if (method == "POST")
      {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, -1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
      }
      if (method == "DELETE")
      {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
      }
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
      res = curl_easy_perform(curl);
      /* Check for errors */
      if (res != CURLE_OK){
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        std::cerr << "curl_easy_perform() url error: " << uri + path << std::endl;
      }
      /* always cleanup */
      curl_easy_cleanup(curl);
      /* free the custom headers */
      curl_slist_free_all(chunk);
    }
  }
  catch(exception e)
  {
    // cout << "hitb api occur: " << e.what() << endl;
  }
  return readBuffer;
}

string BNUSAPI::Get_orderbook(string market, string limit)
{
  string url = "/api/v3/depth?symbol=" + market + "&limit=" + limit;
  string res = Call("GET", false, url, "");
  return res;
}



//List Accounts
string BNUSAPI::Get_List_Accounts()
{
  string res = Call("GET", true, "/accounts/", "");
  return res;
}

//Get an Account
string BNUSAPI::Get_Account(string account_id)
{
  string res = Call("GET", true, "/accounts/" + account_id, "");
  return res;
}

//Get Account History
string BNUSAPI::Get_Account_History(string account_id)
{
  string res = Call("GET", true, "/accounts/" + account_id + "/ledger", "");
  return res;
}

//Get Holds
string BNUSAPI::Get_Holds(string account_id)
{
  string res = Call("GET", true, "/accounts/" + account_id + "/ledger", "");
  return res;
}

// Place a New Order
string BNUSAPI::Place_New_Order(string side, string price, string size, string client_oid, string type, string stp, string stop, string stop_price, string time_in_force, string cancel_after, bool post_only, string funds)
{
  string order_id = "";
  Document d;
  d.SetObject();
  rapidjson::Document::AllocatorType &allocator = d.GetAllocator();

  /* [optional] adds type */
  Value v_type;
  v_type = StringRef(type.c_str());
  d.AddMember("type", v_type, allocator);

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

  /* [optional] Gets the Order to be a maker and not a taker */
  d.AddMember("post_only", post_only, allocator);

  /* [optional] Order ID selected by you to identify your order */
  Value v_client_oid;
  v_client_oid = StringRef(client_oid.c_str());
  d.AddMember("client_oid", v_client_oid, allocator);

  /* [optional] Only if stop is defined. Sets trigger price for stop order. */
  Value v_stop_price;
  v_stop_price = StringRef(stop_price.c_str());
  d.AddMember("stop_price", v_stop_price, allocator);

  /* [optional] Self-trade prevention flag   
  dc  Decrease and Cancel (default)
  co  Cancel oldest
  cn  Cancel newest
  cb  Cancel both  */
  Value v_stp;
  v_stp = StringRef(stp.c_str());
  d.AddMember("stp", v_stp, allocator);

  /* [optional] Either loss or entry. Requires stop_price to be defined. */
  Value v_stop;
  v_stop = StringRef(stop.c_str());
  d.AddMember("stop", v_stop, allocator);

  /* [optional] GTC, GTT, IOC, or FOK (default is GTC) */
  Value v_time_in_force;
  v_time_in_force = StringRef(time_in_force.c_str());
  d.AddMember("time_in_force", v_time_in_force, allocator);

  /* [optional]* min, hour, day */
  Value v_cancel_after;
  v_cancel_after = StringRef(cancel_after.c_str());
  d.AddMember("cancel_after", v_cancel_after, allocator);

  /* [optional] Desired amount of quote currency to use */
  Value v_funds;
  v_funds = StringRef(funds.c_str());
  d.AddMember("funds", v_funds, allocator);

  /* creates the string in json */
  StringBuffer strbuf;
  Writer<StringBuffer> writer(strbuf);
  d.Accept(writer);
  string res = Call("POST", true, "/orders", strbuf.GetString());
  Document d_1;
  d_1.Parse(res.c_str());
  if (d_1.HasMember("id"))
  {
    assert(d_1["id"].IsString());
    order_id = d_1["id"].GetString();
  }
  if (d_1.HasMember("message"))
  {
    assert(d_1["message"].IsString());
    std::cout << "(Place a new order) Message: " << d_1["message"].GetString() << std::endl;
  }
  std::cout << "(Place a new order) Res: " << res << std::endl;
  return res;
}

// Cancel Order
string BNUSAPI::Cancel_Order(string oid, bool isOid)
{
  string url = "";
  if (!isOid)
  {
    url = "/orders/" + oid;
  }
  else
    url = "/orders/client:" + oid;
  string res = Call("DELETE", true, url, "");
  Document d;
  d.Parse(res.c_str());
  if (d.HasMember("message"))
  {
    assert(d["message"].IsString());
    std::cout << "(Cancel_Order) Message: " << d["message"].GetString() << std::endl;
  }
  return res;
}

// Cancel All Orders
string BNUSAPI::Cancel_All_Order()
{
  string res = Call("DELETE", true, "/orders", "");
  return res;
}

//List Orders
string BNUSAPI::List_Orders(string product_id, bool isOpen, bool isPending, bool isActive)
{
  string url = "/orders?";
  if (product_id != "")
  {
    url += "product_id=" + product_id + "&";
  }
  if (isOpen)
  {
    url += "status=open&";
  }
  if (isPending)
  {
    url += "status=pending&";
  }
  if (isActive)
  {
    url += "status=active";
  }

  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return res;
}

//Get an Order
string BNUSAPI::Get_Order(string oid, bool isOid)
{
  string url = "";
  if (!isOid)
  {
    url = "/orders/" + oid;
  }
  else
    url = "/orders/client:" + oid;
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  Document d;
  d.Parse(res.c_str());
  if (d.HasMember("message"))
  {
    assert(d["message"].IsString());
    std::cout << "(Get_Order) Message: " << d["message"].GetString() << std::endl;
  }
  return res;
}

//Products
// Get Products
string BNUSAPI::Get_Products()
{
  string url = "/products";

  string res = Call("GET", false, url, "");
  std::cout << res << std::endl;
  return res;
}

//Get Single Product
string BNUSAPI::Get_Single_Product()
{
  string url = "/products/" + product_id;
  string res = Call("GET", false, url, "");
  std::cout << res << std::endl;
  return res;
}

//Get Product Order Book
string BNUSAPI::Get_Product_Order_Book()
{
  string url = "/products/" + product_id + "/book";
  string res = Call("GET", false, url, "");
  std::cout << res << std::endl;
  return res;
}

//Get Product Ticker
string BNUSAPI::Get_Product_Ticker()
{
  string url = "/products/" + product_id + "/ticker";
  string res = Call("GET", false, url, "");
  std::cout << res << std::endl;
  return res;
}

//Get Trades
string BNUSAPI::Get_Trades()
{
  string url = "/products/" + product_id + "/trades";
  string res = Call("GET", false, url, "");
  std::cout << res << std::endl;
  return res;
}

//Get Historic Rates
string BNUSAPI::Get_Historic_Rates()
{
  string url = "/products/" + product_id + "/candles";
  string res = Call("GET", false, url, "");
  std::cout << res << std::endl;
  return res;
}

//Get 24hr Stats
string BNUSAPI::Get_24hr_Stats()
{
  string url = "/products/" + product_id + "/stats";
  string res = Call("GET", false, url, "");
  std::cout << res << std::endl;
  return res;
}

//Get Currencies
string BNUSAPI::Get_Currencies()
{
  string url = "/currencies/";
  string res = Call("GET", false, url, "");
  std::cout << res << std::endl;
  return res;
}

//Get Currency
string BNUSAPI::Get_Currency(string cid)
{
  string url = "/currencies/" + cid;
  string res = Call("GET", false, url, "");
  std::cout << res << std::endl;
  return res;
}

//Time
//Get Time
string BNUSAPI::Get_Time()
{
  string url = "/time";
  string res = Call("GET", false, url, "");
  std::cout << res << std::endl;
  return res;
}

//Fills
//List Fills
std:: string BNUSAPI::List_Fills(string order_id, string product_id)
{
  string url = "/fills?";
  if(order_id != ""){
    url += "order_id=" + order_id + "&";
  }
  if(product_id != ""){
    url += "product_id=" + product_id;
  }
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return res;
}

//Payment Methods
//List_Payment_Methods
string BNUSAPI::List_Payment_Methods()
{
  string url = "/payment-methods";
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return res;
}

//Coinbase Accounts
//List Coinbase Accounts
string BNUSAPI::List_Coinbase_Accounts()
{
  string url = "/coinbase-accounts";
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return res;
}

//Limits
//Get Current Exchange Limits
string BNUSAPI::Get_Current_Exchange_Limits()
{
  string url = "/users/self/exchange-limits";
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return res;
}

//Deposit
//List Deposits
string BNUSAPI::List_Deposits(string profile_id, string before, string after, string limit)
{
  string url = "/transfers?type=deposit";
  if(profile_id != ""){
    url += "profile_id=" + profile_id + "&";
  }
  if(before != ""){
    url += "before=" + before + "&";
  }
  if(after != ""){
    url += "after=" + after + "&";
  }
  if(limit != ""){
    url += "limit=" + limit;
  }
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return res;
}

//Single Deposit
string BNUSAPI::Single_Deposit(string transfer_id)
{
  string url = "/transfers/:" + transfer_id;
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return res;
}

//Payment method
string BNUSAPI::Payment_Method_Deposit(string amount, string currency, string payment_method_id)
{
  Document d;
  d.SetObject();
  rapidjson::Document::AllocatorType &allocator = d.GetAllocator();

  /* The amount to deposit */
  Value v_amount;
  v_amount = StringRef(amount.c_str());
  d.AddMember("amount", v_amount, allocator);
  
  /* The type of currency */
  Value v_currency;
  v_currency = StringRef(currency.c_str());
  d.AddMember("currency", v_currency, allocator);

  /* ID of the payment method */
  Value v_payment_method_id;
  v_payment_method_id = StringRef(payment_method_id.c_str());
  d.AddMember("payment_method_id", v_payment_method_id, allocator);

  /* creates the string in json */
  StringBuffer strbuf;
  Writer<StringBuffer> writer(strbuf);
  d.Accept(writer);
  string res = Call("POST", true, "/deposits/payment-method", strbuf.GetString());
  std::cout << res ;
  return res;
}

//Coinbase Deposit
string BNUSAPI::Coinbase_Deposit(string amount, string currency, string coinbase_account_id)
{
  Document d;
  d.SetObject();
  rapidjson::Document::AllocatorType &allocator = d.GetAllocator();

  /* The amount to deposit */
  Value v_amount;
  v_amount = StringRef(amount.c_str());
  d.AddMember("amount", v_amount, allocator);
  
  /* The type of currency */
  Value v_currency;
  v_currency = StringRef(currency.c_str());
  d.AddMember("currency", v_currency, allocator);

  /* ID of the coinbase_account */
  Value v_coinbase_account_id;
  v_coinbase_account_id = StringRef(coinbase_account_id.c_str());
  d.AddMember("coinbase_account_id", v_coinbase_account_id, allocator);

  /* creates the string in json */
  StringBuffer strbuf;
  Writer<StringBuffer> writer(strbuf);
  d.Accept(writer);
  string res = Call("POST", true, "/deposits/coinbase-account", strbuf.GetString());
  std::cout << res ;
  return res;
}

//Generate a Crypto Deposit Address
string BNUSAPI::Generate_Crypto_Deposit_Address(string coinbase_account_id)
{

  string res = Call("POST", true, "/coinbase-accounts/" + coinbase_account_id + "/addresses", "");
  std::cout << res ;
  return res;
}

//Withdraw
//List Withdrawals
string BNUSAPI::List_Withdrawals(string profile_id, string before, string after, string limit)
{
  string url = "/transfers?type=withdraw";
  if(profile_id != ""){
    url += "profile_id=" + profile_id + "&";
  }
  if(before != ""){
    url += "before=" + before + "&";
  }
  if(after != ""){
    url += "after=" + after + "&";
  }
  if(limit != ""){
    url += "limit=" + limit;
  }
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return res;
}

//Single Withdrawal
string BNUSAPI::Single_Withdrawal(string transfer_id)
{
  string url = "/transfers/:" + transfer_id;
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return res;
}

//Payment method
string BNUSAPI::Payment_Method_Withdraw(string amount, string currency, string payment_method_id)
{
  Document d;
  d.SetObject();
  rapidjson::Document::AllocatorType &allocator = d.GetAllocator();

  /* The amount to deposit */
  Value v_amount;
  v_amount = StringRef(amount.c_str());
  d.AddMember("amount", v_amount, allocator);
  
  /* The type of currency */
  Value v_currency;
  v_currency = StringRef(currency.c_str());
  d.AddMember("currency", v_currency, allocator);

  /* ID of the payment method */
  Value v_payment_method_id;
  v_payment_method_id = StringRef(payment_method_id.c_str());
  d.AddMember("payment_method_id", v_payment_method_id, allocator);

  /* creates the string in json */
  StringBuffer strbuf;
  Writer<StringBuffer> writer(strbuf);
  d.Accept(writer);
  string res = Call("POST", true, "/withdrawals/payment-method", strbuf.GetString());
  std::cout << res ;
  return res;
}

//Coinbase Withdraw
string BNUSAPI::Coinbase_Withdraw(string amount, string currency, string coinbase_account_id)
{
  Document d;
  d.SetObject();
  rapidjson::Document::AllocatorType &allocator = d.GetAllocator();

  /* The amount to deposit */
  Value v_amount;
  v_amount = StringRef(amount.c_str());
  d.AddMember("amount", v_amount, allocator);
  
  /* The type of currency */
  Value v_currency;
  v_currency = StringRef(currency.c_str());
  d.AddMember("currency", v_currency, allocator);

  /* ID of the coinbase_account */
  Value v_coinbase_account_id;
  v_coinbase_account_id = StringRef(coinbase_account_id.c_str());
  d.AddMember("coinbase_account_id", v_coinbase_account_id, allocator);

  /* creates the string in json */
  StringBuffer strbuf;
  Writer<StringBuffer> writer(strbuf);
  d.Accept(writer);
  string res = Call("POST", true, "/withdrawals/coinbase-account", strbuf.GetString());
  std::cout << res ;
  return res;
}

//Crytop Withdraw
string BNUSAPI::Crypto_Withdraw(string amount, string currency, string crypto_address, string destination_tag, string no_destination_tag, string add_network_fee_to_total)
{
  Document d;
  d.SetObject();
  rapidjson::Document::AllocatorType &allocator = d.GetAllocator();

  /* The amount to deposit */
  Value v_amount;
  v_amount = StringRef(amount.c_str());
  d.AddMember("amount", v_amount, allocator);
  
  /* The type of currency */
  Value v_currency;
  v_currency = StringRef(currency.c_str());
  d.AddMember("currency", v_currency, allocator);

  /* A crypto address of the recipient */
  Value v_crypto_address;
  v_crypto_address = StringRef(crypto_address.c_str());
  d.AddMember("crypto_address", v_crypto_address, allocator);

  /* A destination tag for currencies that support one */
  Value v_destination_tag;
  v_destination_tag = StringRef(destination_tag.c_str());
  d.AddMember("destination_tag", v_destination_tag, allocator);

  /* A boolean flag to opt out of using a destination tag for currencies that support one. This is required when not providing a destination tag. */
  Value v_no_destination_tag;
  v_no_destination_tag = StringRef(no_destination_tag.c_str());
  d.AddMember("no_destination_tag", v_no_destination_tag, allocator);

  /* A boolean flag to add the network fee on top of the amount. If this is blank, it will default to deducting the network fee from the amount. */
  Value v_add_network_fee_to_total;
  v_add_network_fee_to_total = StringRef(add_network_fee_to_total.c_str());
  d.AddMember("add_network_fee_to_total", v_add_network_fee_to_total, allocator);

  /* creates the string in json */
  StringBuffer strbuf;
  Writer<StringBuffer> writer(strbuf);
  d.Accept(writer);
  string res = Call("POST", true, "/withdrawals/crypto", strbuf.GetString());
  std::cout << res ;
  return res;
}

//Fee Estimate
string BNUSAPI::Fee_Estimate(string currency, string crypto_address)
{
  string url = "/withdrawals/fee-estimate?currency=" + currency + "&crypto_address" + crypto_address;
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return res;
}

//Stablecoin Conversions
//Create Conversion
string BNUSAPI::Create_Conversion(string from, string to, string amount)
{
  Document d;
  d.SetObject();
  rapidjson::Document::AllocatorType &allocator = d.GetAllocator();

  /* A valid currency id */
  Value v_from;
  v_from = StringRef(from.c_str());
  d.AddMember("from", v_from, allocator);
  
  /* A valid currency id */
  Value v_to;
  v_to = StringRef(to.c_str());
  d.AddMember("to", v_to, allocator);

  /* Amount of from to convert to to, current limit is 10,000,000 */
  Value v_amount;
  v_amount = StringRef(amount.c_str());
  d.AddMember("amount", v_amount, allocator);

  /* creates the string in json */
  StringBuffer strbuf;
  Writer<StringBuffer> writer(strbuf);
  d.Accept(writer);
  string res = Call("POST", true, "/conversions", strbuf.GetString());
  std::cout << res ;
  return res;
}

//Fees
//Get Current Fees
string BNUSAPI::Get_Current_Fees()
{
  string url = "/fees";
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return res;
}

//Reports
//Create a new report
string BNUSAPI::Create_new_report(string type, string start_date, string end_date, string product_id, string account_id, string format, string email)
{
  Document d;
  d.SetObject();
  rapidjson::Document::AllocatorType &allocator = d.GetAllocator();

  /* fills or account */
  Value v_type;
  v_type = StringRef(type.c_str());
  d.AddMember("type", v_type, allocator);
  
  /* Starting date for the report (inclusive) */
  Value v_start_date;
  v_start_date = StringRef(start_date.c_str());
  d.AddMember("start_date", v_start_date, allocator);

  /* Ending date for the report (inclusive) */
  Value v_end_date;
  v_end_date = StringRef(end_date.c_str());
  d.AddMember("end_date", v_end_date, allocator);

  /* ID of the product to generate a fills report for. E.g. BTC-USD. Required if type is fills */
  Value v_product_id;
  v_product_id = StringRef(product_id.c_str());
  d.AddMember("product_id", v_product_id, allocator);

  /* ID of the account to generate an account report for. Required if type is account */
  Value v_account_id;
  v_account_id = StringRef(account_id.c_str());
  d.AddMember("account_id", v_account_id, allocator);

  /* pdf or csv (defualt is pdf) */
  Value v_format;
  v_format = StringRef(format.c_str());
  d.AddMember("format", v_format, allocator);

  /* Email address to send the report to (optional) */
  Value v_email;
  v_email = StringRef(email.c_str());
  d.AddMember("email", v_email, allocator);

  /* creates the string in json */
  StringBuffer strbuf;
  Writer<StringBuffer> writer(strbuf);
  d.Accept(writer);
  string res = Call("POST", true, "/reports", strbuf.GetString());
  std::cout << res ;
  return res;
}

//Get report status
string BNUSAPI::Get_report_status(string report_id)
{
  string url = "/reports/:" + report_id;
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return res;
}

//List Profiles
string BNUSAPI::List_Profiles(string active)
{
  string url = "/profiles";
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return res;
}

//Get a Profile
string BNUSAPI::Get_Profile(string profile_id)
{
  string url = "/profiles/" + profile_id;
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return res;
}

//Create profile transfer
string BNUSAPI::Create_profile_transfer(string from, string to, string currency, string amount)
{
  Document d;
  d.SetObject();
  rapidjson::Document::AllocatorType &allocator = d.GetAllocator();

  /* The profile id the API key belongs to and where the funds are sourced */
  Value v_from;
  v_from = StringRef(from.c_str());
  d.AddMember("from", v_from, allocator);
  
  /* The target profile id of where funds will be transferred to */
  Value v_to;
  v_to = StringRef(to.c_str());
  d.AddMember("to", v_to, allocator);

  /* i.e. BTC or USD */
  Value v_currency;
  v_currency = StringRef(currency.c_str());
  d.AddMember("currency", v_currency, allocator);

  /* Amount of currency to be transferred */
  Value v_amount;
  v_amount = StringRef(amount.c_str());
  d.AddMember("amount", v_amount, allocator);

  /* creates the string in json */
  StringBuffer strbuf;
  Writer<StringBuffer> writer(strbuf);
  d.Accept(writer);
  string res = Call("POST", true, "/profiles/transfer", strbuf.GetString());
  std::cout << res ;
  return res;
}

//User Account
//Trailing Volume
string BNUSAPI::Trailing_Volume()
{
  string url = "/users/self/trailing-volume";
  string res = Call("GET", true, url, "");
  std::cout << res << std::endl;
  return res;
}


















BNUSAPI::BNUSAPI()
{
  curl_global_init(CURL_GLOBAL_DEFAULT);
}

BNUSAPI::~BNUSAPI()
{
  curl_global_cleanup();
}
