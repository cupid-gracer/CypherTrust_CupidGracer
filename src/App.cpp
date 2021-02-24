#include "API.h"
#include "App.h"
#include <vector>
#include <iostream>
#include "curl/curl.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;
using namespace std;

App::App()
{
}

App::~App()
{
}


void App::auth(string user, string password, string type)
{
}