// App.h
#ifndef APP_H
#define APP_H

#include <string>
#include <vector>
#include "API.h"

using namespace std;

class App
{

public:
    App();
    ~App();

    API api;
    string uri;

    void auth(string user, string password, string type);


};



#endif // APP_H
