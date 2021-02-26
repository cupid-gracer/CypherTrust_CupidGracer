// HUOB.h
#ifndef HUOB_H
#define HUOB_H

#include "exchange/HUOB/API.h"
#include <string>
#include <vector>

using namespace std;

class HUOB
{
public:
    HUOB();
    ~HUOB();
    /* Declare Variables */
    string uri;
    API api;



};

#endif // HUOB_H
