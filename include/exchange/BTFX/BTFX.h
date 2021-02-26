// BTFX.h
#ifndef BTFX_H
#define BTFX_H

#include "exchange/BTFX/API.h"
#include <string>
#include <vector>

using namespace std;

class BTFX
{
public:
    BTFX();
    ~BTFX();
    /* Declare Variables */
    string uri;
    API api;



};

#endif // BTFX_H
