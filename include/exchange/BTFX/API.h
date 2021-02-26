// Api.h
#ifndef API_H
#define API_H

#include <string>
#include <vector>
#include "Util.h"

using namespace std;

class API
{

private:
    string Call(string method, bool authed, string path, string body);

public:
    API();
    ~API();
    Util util;
    string uri;

};

#endif // API_H
