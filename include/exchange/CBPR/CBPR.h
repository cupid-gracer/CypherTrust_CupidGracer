// CBPR.h
#ifndef CBPR_H
#define CBPR_H

#include "exchange/CBPR/API.h"
#include <string>
#include <vector>

using namespace std;

class CBPR
{
public:
    CBPR();
    ~CBPR();
    /* Declare Variables */
    string uri;
    string product_id;
    API api;

    string Get_Buy_Price();
    double Get_Balance(string currency);
    string Place_Limit_Order(string side, string price, string size);

    // Accounts
    /*
  Name:
      List Accounts
  Summary:
      Get a list of trading accounts from the profile of the API key.
  */
    void Get_List_Accounts();

    /*
  Name:
      Get an Account
  Summary:
      Information for a single account. Use this endpoint when you know the account_id. API key must belong to the same profile as the account.
  */
    void Get_Account();

    /*
  Name:
      Get Account History
  Summary:
      List account activity of the API key's profile. Account activity either increases or decreases your account balance. Items are paginated and sorted latest first. See the Pagination section for retrieving additional entries after the first page.    
  */
    void Get_Account_History();

    /*
  Name:
      Get Holds
  Summary:
      List holds of an account that belong to the same profile as the API key. Holds are placed on an account for any active orders or pending withdraw requests. As an order is filled, the hold amount is updated. If an order is canceled, any remaining hold is removed. For a withdraw, once it is completed, the hold is removed.
  */
    void Get_Holds();

    //Orders
    /*
  Name:
      Place a New Order
  Summary:
      You can place two types of orders: limit and market. Orders can only be placed if your account has sufficient funds.     
  Parameters:
      client_oid	[optional] Order ID selected by you to identify your order
      type	      [optional] limit or market (default is limit)
      side	      buy or sell
      product_id	A valid product id
      stp	        [optional] Self-trade prevention flag
      stop	      [optional] Either loss or entry. Requires stop_price to be defined.
      stop_price	[optional] Only if stop is defined. Sets trigger price for stop order.
      
      LIMIT ORDER PARAMETERS
      price	      Price per bitcoin
      size	      Amount of base currency to buy or sell
      time_in_force	[optional] GTC, GTT, IOC, or FOK (default is GTC)
      cancel_after	[optional]* min, hour, day
      post_only	    [optional]** Post only flag

      MARKET ORDER PARAMETERS
      size	      [optional]* Desired amount in base currency
      funds	      [optional]* Desired amount of quote currency to use
  */
    void Place_New_Order();

    /*
  Name:
      Cancel an Order
  Summary:
      Cancel a previously placed order. Order must belong to the profile that the API key belongs to.
  Parameters:
      isOid = false; =>    request URL = DELETE /orders/<id>
      isOid = true;  =>    request URL = DELETE /orders/client:<client_oid>
  */
    void Cancel_Order();

    /*
  Name:
      Cancel all
  Summary:
      With best effort, cancel all open orders from the profile that the API key belongs to. The response is a list of ids of the canceled orders.
  */
    void Cancel_All_Order();

    /*
  Name:
      List Orders
  Summary:
      List your current open orders from the profile that the API key belongs to. Only open or un-settled orders are returned.
  */
    void List_Orders();

    /*
  Name:
      Get an Order
  Summary:
      Get a single order by order id from the profile that the API key belongs to.
  Parameters:
      isOid = false; =>    request URL = GET /orders/<id>
      isOid = true;  =>    request URL = GET /orders/client:<client_oid>
  */
    void Get_Order();

    // Products
    /*
  Name:
      Get Products
  Summary:
      Get a list of available currency pairs for trading.
  */
    void Get_Products();

    /*
  Name:
      Get Single Product
  Summary:
      Get market data for a specific currency pair.
  */
    void Get_Single_Product();

    /*
  Name:
      Get Product Order Book
  Summary:
      Get a list of open orders for a product. The amount of detail shown can be customized with the level parameter.
  */
    void Get_Product_Order_Book();

    /*
  Name:
      Get Product Ticker
  Summary:
      Snapshot information about the last trade (tick), best bid/ask and 24h volume.
  */
    void Get_Product_Ticker();

    /*
  Name:
      Get Trades
  Summary:
      List the latest trades for a product.
  */
    void Get_Trades();

    /*
  Name:
      Get Historic Rates
  Summary:
      Historic rates for a product. Rates are returned in grouped buckets based on requested granularity.
  */
    void Get_Historic_Rates();

    /*
  Name:
      Get 24hr Stats
  Summary:
      Get 24 hr stats for the product. volume is in base currency units. open, high, low are in quote currency units.
  */
    void Get_24hr_Stats();

    //Currencies
    /*
  Name:
      Get currencies
  Summary:
      List known currencies.
  */
    void Get_Currencies();

    /*
  Name:
      Get a currency
  Summary:
      List the currency for specified id.
  */
    void Get_Currency();

    //Time
    /*
    Name:
        Get Time
    Summary:
        Get the API server time.
    */
    void Get_Time();

    //Fills
    /*
    Name:
        List Fills
    Summary:
        Get a list of recent fills of the API key's profile.
    */
    void List_Fills();

    //Limits
    /*
    Name:
        Get Current Exchange Limits
    Summary:
        This request will return information on your payment method transfer limits, as well as buy/sell limits per currency.
    */
   void Get_Current_Exchange_Limits(); 

    //Payment Methods
    /*
    Name:
        List Payment Methods
    Summary:
        Get a list of your payment methods.
    */
   void List_Payment_Methods(); 

   //Coinbase Accounts
    /*
    Name:
        List Coinbase Accounts
    Summary:
        Get a list of your coinbase accounts.
    */
   void List_Coinbase_Accounts(); 

    //Deposits
    /*
    Name:
        List Deposits
    Summary:
        Get a list of deposits from the profile of the API key, in descending order by created time
    */
   void List_Deposits(); 

    /*
    Name:
        Single Deposit
    Summary:
        Get information on a single deposit.
    */
    void Single_Deposit();

    /*
    Name:
        Payment method for deposit
    Summary:
        Deposit funds from a payment method. See the Payment Methods section for retrieving your payment methods.
    */
    void Payment_Method_Deposit();

    /*
    Name:
        Coinbase deposit
    Summary:
         You can move funds between your Coinbase accounts and your Coinbase Pro trading accounts within your daily limits.
    */
    void Coinbase_Deposit();

    /*
    Name:
        Generate a Crypto Deposit Address
    Summary:
        You can generate an address for crypto deposits.
    */
    void Generate_Crypto_Deposit_Address();


    //Withdraw
    /*
    Name:
        List Withdrawals
    Summary:
        Get a list of withdrawals from the profile of the API key, in descending order by created time. 
    */
    void List_Withdrawals();

    /*
    Name:
        Single Withdrawal
    Summary:
        Get information on a single withdrawal.
        It may contain the cancel code, so you must process the cancel code!
    */
    void Single_Withdrawal();

    /*
    Name:
        Payment method for Withdraw
    Summary:
        Withdraw funds from a payment method. See the Payment Methods section for retrieving your payment methods.
    */
    void Payment_Method_Withdraw();

    /*
    Name:
        Coinbase for withdraw
    Summary:
         You can move funds between your Coinbase accounts and your Coinbase Pro trading accounts within your daily limits.
    */
    void Coinbase_Withdraw();

    /*
    Name:
        Crypto
    Summary:
        Withdraws funds to a crypto address.
    */
    void Crypto_Withdraw();

    /*
    Name:
        Fee Estimate
    Summary:
        Gets the network fee estimate when sending to the given address.
    */
    void Fee_Estimate();

    //Stablecoin Conversions
    /*
    Name:
        Create Conversion
    Summary:
        A successful conversion will be assigned a conversion id. The corresponding ledger entries for a conversion will reference this conversion id. 
    */
    void Create_Conversion();

    //Fees
    /*
    Name:
        Get Current Fees
    Summary:
        This request will return your current maker & taker fee rates, as well as your 30-day trailing volume.
    */
    void Get_Current_Fees();

    //Reports
    /*
    Name:
        Create a new report
    Summary:
        Reports provide batches of historic information about your profile in various human and machine readable forms.
    */
    void Create_new_report();

    /*
    Name:
        Get report status
    Summary:
        Once a report request has been accepted for processing, the status is available by polling the report resource endpoint.
    */
    void Get_report_status();

    //Profiles
    /*
    Name:
        List Profiles
    Summary:
        List your profiles.
    */
    void List_Profiles();

    /*
    Name:
        Get a Profile
    Summary:
        Get a single profile by profile id.
    */
    void Get_Profile();

    /*
    Name:
        Create profile transfer
    Summary:
        Transfer funds from API key's profile to another user owned profile.
    */
    void Create_profile_transfer();
    
    //User Account
    /*
    Name:
        Trailing Volume
    Summary:
        This request will return your 30-day trailing volume for all products of the API key's profile. This is a cached value that's calculated every day at midnight UTC.
    */
    void Trailing_Volume();


};

#endif // CBPR_H
