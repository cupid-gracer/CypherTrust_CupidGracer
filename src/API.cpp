/* CypherTrust */

#include "API.h"
#include <vector>
#include <iostream>
#include "curl/curl.h"
#include <syslog.h>
#include <boost/algorithm/string.hpp>

#include <json/json.h>
#include <json/value.h>

#include "Util.h"

#define CYPHER_TRUST "CypherTrust"

using namespace std;


/* Used by API::Call to put websource into a string type */
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  ((string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

static int DetectHttpError(string res)
{
  Util util = Util();


  Json::CharReaderBuilder charReaderBuilder;
  Json::Value obj;
  stringstream input(res);
  string errs;
  bool isParse = parseFromStream(charReaderBuilder, input, &obj, &errs);
  if(!isParse) return 500;
  

  if(obj["error"])
  {
    string str_error = obj["error"].asString();

    boost::erase_all(str_error, "x");
    int error_code = atoi(str_error.c_str());
    if(error_code == 40100)
    {
      cout << util.Red + "Warning!\t" + util.ResetColor + util.Yellow << "Wrong credentials" + util.ResetColor << endl;
    }
    else if(error_code == 40101)
    {
      cout << util.Red + "Warning!\t" + util.ResetColor + util.Yellow << "Connector Type not allowed for the User Type" + util.ResetColor  << endl;
    }
    else if(error_code == 40300)
    {
      cout << util.Red + "Warning!\t" + util.ResetColor + util.Yellow << "IP address not whitelisted" + util.ResetColor  << endl;
    }
    else if(error_code == 40301)
    {
      cout << util.Red + "Warning!\t" + util.ResetColor + util.Yellow << "User-Agent not whitelisted" + util.ResetColor  << endl;
    }
    else if(error_code == 40302)
    {
      cout << util.Red + "Warning!\t" + util.ResetColor + util.Yellow << "Username is not enabled" + util.ResetColor  << endl;
    }
    else if(error_code == 40303)
    {
      cout << util.Red + "Warning!\t" + util.ResetColor + util.Yellow << "Portfolio is not enabled" + util.ResetColor  << endl;
    }
    else if(error_code == 40304)
    {
      cout << util.Red + "Warning!\t" + util.ResetColor + util.Yellow << "Role is not allowed for this operation" + util.ResetColor  << endl;
    }
    else if(error_code == 40305)
    {
      // cout << util.Red + "Warning!\t" + util.ResetColor + util.Yellow << "Address already registered on the network" + util.ResetColor  << endl;
    }
    else if(error_code == 40306)
    {
      cout << util.Red + "Warning!\t" + util.ResetColor + util.Yellow << "Address is missing from argument" + util.ResetColor  << endl;
    }
    else if(error_code == 40400)
    {
      cout << util.Red + "Warning!\t" + util.ResetColor + util.Yellow << "The key X-Exchange-Key is not set" + util.ResetColor  << endl;
    }
    else if(error_code == 40401)
    {
      cout << util.Red + "Warning!\t" + util.ResetColor + util.Yellow << "The key X-Auth-Password is not set" + util.ResetColor  << endl;
    }
    else if(error_code == 40402)
    {
      cout << util.Red + "Warning!\t" + util.ResetColor + util.Yellow << "The key X-Auth-Username is not set" + util.ResetColor  << endl;
    }
    else if(error_code == 40403)
    {
      cout << util.Red + "Warning!\t" + util.ResetColor + util.Yellow << "The address could not be found on the network" + util.ResetColor  << endl;
    }
    else if(error_code == 40404)
    {
      cout << util.Red + "Warning!\t" + util.ResetColor + util.Yellow << "The market could not be found for this connector" + util.ResetColor  << endl;
    }
    else if(error_code == 40405)
    {
      cout << util.Red + "Warning!\t" + util.ResetColor + util.Yellow << "No markets are defined for the Exchange" + util.ResetColor  << endl;
    }
    else if(error_code == 40406)
    {
      cout << util.Red + "Warning!\t" + util.ResetColor + util.Yellow << "No user type has been defined for the Username" + util.ResetColor  << endl;
    }
    else if(error_code == 40407)
    {
      cout << util.Red + "Warning!\t" + util.ResetColor + util.Yellow << "No connector type has been set for this request" + util.ResetColor  << endl;
    }
    return error_code;
  }else
    return 200;
}

API::API()
{
  curl_global_init(CURL_GLOBAL_DEFAULT);
  util = Util();
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
      chunk = curl_slist_append(chunk, "Content-Type: application/json");
      curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, -1L);
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    }
    if (method == "DELETE")
    {
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    }
    if (method == "PUT")
    {
      chunk = curl_slist_append(chunk, "Content-Type: application/json");
      curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, -1L);
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    }
    if (method == "PATCH")
    {
      chunk = curl_slist_append(chunk, "Content-Type: application/json");
      curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, -1L);
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    }
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    res = curl_easy_perform(curl);
    /* Check for errors */

    // if (res != CURLE_OK)
    //   std::cerr << "APP API curl_easy_perform() failed: " << curl_easy_strerror(res) << endl << (url + path) << std::endl;
    /* always cleanup */
    curl_easy_cleanup(curl);
    /* free the custom headers */
    curl_slist_free_all(chunk);

  }
  return readBuffer;
}


string API::auth()
{
  string _authBody = "{\"type\":\"" + type + "\"}";

  string res = Call("POST", false, "/connector", _authBody);
  int respose_code = DetectHttpError(res);
  if(respose_code == 200)
  {
    return res;
  }else {
      if(respose_code == 40305)
      {


        Json::CharReaderBuilder charReaderBuilder;
        Json::Value obj;
        stringstream input(res);
        string errs;
        bool isParse = parseFromStream(charReaderBuilder, input, &obj, &errs);
        if(!isParse) return "error";
  
        string address_id = obj["debug"].asString();
        if(del_address(address_id))
        {
          return res = auth();
        }
      }
  }
  return "error";
}


bool API::del_address(string address_id)
{
  this->LoggingEvent(address_id, "INFO", "auth", "The Connector " + address_id + " is deregistered.");

  string res = Call("DELETE", false, "/connector/" + address_id, "");

  Json::CharReaderBuilder charReaderBuilder;
  Json::Value obj;
  stringstream input(res);
  string errs;
  bool isParse = parseFromStream(charReaderBuilder, input, &obj, &errs);
  if(!isParse) return false;
  
  if(obj["address"])
  {
    return true;
  }
  return false;
}


string API::ping(string address_id)
{
  string res = Call("GET", false, "/heartbeat/ping/" + address_id, "");

  return res;
}

void API::StartSession(string address_id, string market, bool snapshot, string context, string classType)
{
  Json::Value obj;
  obj["context"]  = context;
  obj["classType"] = classType;

  Json::StreamWriterBuilder streamWriterBuilder;
  streamWriterBuilder["indentation"] = "";
  string out = writeString(streamWriterBuilder, obj);

  string res = Call("POST", true, "/stream/" + address_id + "/" + market, out);
}

void API::StopSession(string address_id, string market)
{
  string res = Call("DELETE", true, "/stream/" + address_id + "/" + market, "");
}


void API::LoggingEvent(string address_id, string clss, string context, string text)
{
  Json::Value obj;
  obj["class"]  = clss;
  obj["context"] = context;
  obj["text"] = text;

  Json::StreamWriterBuilder streamWriterBuilder;
  streamWriterBuilder["indentation"] = "";
  string out = writeString(streamWriterBuilder, obj);

  string res = Call("POST", true, "/log/" + address_id, out);

}