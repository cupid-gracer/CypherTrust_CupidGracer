// PLNX.h
#ifndef PLNX_H
#define PLNX_H

#include "exchange/PLNX/API.h"
#include <string>
#include <vector>

using namespace std;

class PLNX
{
public:
    PLNX();
    ~PLNX();
    /* Declare Variables */
    string uri;
    API api;



};

#endif // PLNX_H
