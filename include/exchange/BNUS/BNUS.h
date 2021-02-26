// BNUS.h
#ifndef BNUS_H
#define BNUS_H

#include "exchange/BNUS/API.h"
#include <string>
#include <vector>

using namespace std;

class BNUS
{
public:
    BNUS();
    ~BNUS();
    /* Declare Variables */
    std::string uri;
    API api;



};

#endif // BNUS_H
