#include "exchange/EXMO/API.h"
#include <iostream>
#include "curl/curl.h"
#include <ctime>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "exchange/EXMO/HMAC_SHA512.h"

using namespace rapidjson;
using namespace std;

/* Used by EXMOAPI::Call to put websource into a string type */
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  ((string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

/* Uses libcurl to get Data From API */
string EXMOAPI::Call(string method, bool authed, string path, string body)
{
  CURL *curl;
  CURLcode res;
  string readBuffer;
  curl = curl_easy_init();
  nonce = ::time(nullptr);

  try
  {
    if (curl)
    {
      struct curl_slist *chunk = NULL;
      curl_easy_setopt(curl, CURLOPT_URL, (uri + path).c_str());
      curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl/1.0");
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
      chunk = curl_slist_append(chunk, "Content-Type: application/x-www-form-urlencoded");
      if (authed)
      {
        string params = "nonce=";
        nonce++;
        params.append(std::to_string(nonce));
        
        if (body.size() != 0) {
          params.append("&");
        }
        params.append(body);
        body = params;
        chunk = curl_slist_append(chunk, ("Key: " + api_key).c_str());
        chunk = curl_slist_append(chunk, ("Sign: " + Signature(params)).c_str());
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

string EXMOAPI::Signature(const string& params) {
    HMAC_SHA512 hmac_sha512(secret_key, params);
    return hmac_sha512.hex_digest();
  }



string EXMOAPI::Get_Trades(string pair)
{
  Document d;
  d.SetObject();
  rapidjson::Document::AllocatorType &allocator = d.GetAllocator();

  Value v_pair;
  v_pair = StringRef(pair.c_str());
  d.AddMember("pair", v_pair, allocator);

  /* creates the string in json */
  StringBuffer strbuf;
  Writer<StringBuffer> writer(strbuf);
  d.Accept(writer);
  string res = Call("POST", false, "/trades", strbuf.GetString());

  return res;
}


string EXMOAPI::Get_OrderBook(string pair, string limit)
{
  Document d;
  d.SetObject();
  rapidjson::Document::AllocatorType &allocator = d.GetAllocator();

  Value v_pair;
  v_pair = StringRef(pair.c_str());
  d.AddMember("pair", v_pair, allocator);

  Value v_limit;
  v_limit = StringRef(limit.c_str());
  d.AddMember("limit", v_limit, allocator);

  /* creates the string in json */
  StringBuffer strbuf;
  Writer<StringBuffer> writer(strbuf);
  d.Accept(writer);
  string res = Call("POST", false, "/order_book", strbuf.GetString());

  return res;
}

string EXMOAPI::Get_Ticker()
{
  string res = Call("POST", false, "/ticker", "");
  return res;
}

string EXMOAPI::Get_Currency()
{
  string res = Call("POST", false, "/currency", "");
  return res;
}



//private API
string EXMOAPI::Get_UserInfo()
{
  string res = Call("POST", true, "/user_info", "");
  return res;
}

string EXMOAPI::Get_UserTrades(string pair, string limit, string offset)
{
  Document d;
  d.SetObject();
  rapidjson::Document::AllocatorType &allocator = d.GetAllocator();

  Value v_pair;
  v_pair = StringRef(pair.c_str());
  d.AddMember("pair", v_pair, allocator);

  Value v_limit;
  v_limit = StringRef(limit.c_str());
  d.AddMember("limit", v_limit, allocator);

  Value v_offset;
  v_offset = StringRef(offset.c_str());
  d.AddMember("offset", v_offset, allocator);

  /* creates the string in json */
  StringBuffer strbuf;
  Writer<StringBuffer> writer(strbuf);
  d.Accept(writer);
  string res = Call("POST", true, "/user_trades", strbuf.GetString());

  return res;
}














EXMOAPI::EXMOAPI()
{
  curl_global_init(CURL_GLOBAL_DEFAULT);
}

EXMOAPI::~EXMOAPI()
{
  curl_global_cleanup();
}
