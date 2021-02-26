// HITB.h
#ifndef HITB_H
#define HITB_H

#include "exchange/HITB/API.h"
#include <string>
#include <vector>

class HITB
{
public:
    HITB();
    ~HITB();
    /* Declare Variables */
    std::string uri;
    std::string product_id;
    API api;


    //Public
    //1 Available symbols
    void Get_list_symbols();

    //2 Get symbol info
    void Get_symbol_info();

    //3 Available currencies
    void Get_list_currencies();

    //4 Get currency info
    void Get_currency_info();

    //5 Get Tickers
    void Get_list_tickers();

    //6 Get Tickers
    void Get_ticker();

    //7 Get list Trades
    void Get_list_trades();

    //8 Get Trade
    void Get_trade();

    //9 Get list order
    void Get_list_orderbook();

    //10 Get order
    void Get_orderbook();

    //11 Get list Candles
    void Get_list_candles();

    //12 Get Candle
    void Get_candle();

    //13 Get Candle
    void Get_list_orders();

    //14 Get Arbitrage Opportunity
    void Get_arbitrage_opportunity();
    



};

#endif // HITB_H
