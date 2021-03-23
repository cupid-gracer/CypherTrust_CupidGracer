/* CypherTrust */

#include "API.h"
#include <vector>
#include <iostream>
#include "curl/curl.h"
#include <syslog.h>
#include <boost/algorithm/string.hpp>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#define CYPHER_TRUST "CypherTrust"

using namespace rapidjson;
using namespace std;

/* Used by API::Call to put websource into a string type */
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  ((string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

static int DetectHttpError(string res)
{
  Document d;
  // cout<< res << endl;
  d.Parse(res.c_str());
  if(d.HasMember("error"))
  {
    string str_error = d["error"].GetString();
    cout<< res << endl;

    boost::erase_all(str_error, "x");
    int error_code = atoi(str_error.c_str());
    if(error_code == 20000)
    {
      openlog(CYPHER_TRUST,LOG_CONS | LOG_PID | LOG_NDELAY, LOG_AUTH);    
      syslog(LOG_INFO,"Authentication is success. The status code is %d, ",error_code);
    }
    if(error_code >= 40000 && error_code < 50000)
    {
      openlog(CYPHER_TRUST,LOG_CONS | LOG_PID | LOG_NDELAY, LOG_AUTH);    
      syslog(LOG_ERR,"Authentication is error condition. The status code is %d, ",error_code);
    }
    if (error_code >= 50000 )
    {
      openlog(CYPHER_TRUST, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_AUTH);    
      syslog(LOG_CRIT,"Authentication is critical  condition. The status code is %d, ",error_code);
    }
    closelog();
    return error_code;
  }else
    return 200;
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
    curl_easy_setopt(curl, CURLOPT_URL, (url + path).c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl/1.0");
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    chunk = curl_slist_append(chunk, "User-Agent: CypherTrust/v1.0-Trixie");
    chunk = curl_slist_append(chunk, ("X-Exchange-Key: " + key).c_str());
    chunk = curl_slist_append(chunk, ("X-Auth-Username: " + user).c_str());
    chunk = curl_slist_append(chunk, ("X-Auth-Password: " + password).c_str());
    // chunk = curl_slist_append(chunk, "accept: application/json");
    
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
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
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


string API::auth()
{
  string res = Call("GET", false, "/auth", "");
  int respose_code = DetectHttpError(res);
  if(respose_code == 200)
  {
    return res;
  }else {
      if(respose_code == 40305)
      {
        Document d;
        d.Parse(res.c_str());
        string address_id = d["debug"].GetString();
        if(del_address(address_id))
        {
          return res = Call("GET", false, "/auth", "");
        }
      }
  }
  return "error";
}


bool API::del_address(string address_id)
{
  string res = Call("DELETE", false, "/address/" + address_id, "");
  Document d;
  d.Parse(res.c_str());
  cout << res << endl;
  if(d.HasMember("address"))
  {
    return true;
  }
  return false;
}


void API::ping(string address_id)
{
  string res = Call("GET", false, "/heartbeat/" + address_id, "");
  Document d;
  d.Parse(res.c_str());
  cout << res << endl;
  // if(d.HasMember("address"))
  // {
  //   return true;
  // }
  // return false;
}
