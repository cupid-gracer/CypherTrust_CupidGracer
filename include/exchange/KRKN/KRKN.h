// KRKN.h
#ifndef KRKN_H
#define KRKN_H

#include "exchange/KRKN/API.h"
#include <string>
#include <vector>

using namespace std;

class KRKN
{
public:
    KRKN();
    ~KRKN();
    /* Declare Variables */
    string uri;
    API api;



};

#endif // KRKN_H
