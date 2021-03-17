// Api.h
#ifndef STEXAPI_H
#define STEXAPI_H

#include <string>
#include <vector>
#include "Util.h"

using namespace std;

class STEXAPI
{
private:
    string Call(string method, bool authed, string path, string body);

public:
    STEXAPI();
    ~STEXAPI();
    Util util;
    string uri;
    string access_token;
    string product_id;
    string Get_Buy_Price();
    double Get_Balance(string currency);
    string Place_Limit_Order(string side, string price, string size);

    //Public
    // Available Currencies
    string Get_list_currencies();

    // Get currency info
    string Get_currency_info(string currencyId);

    // Available markets
    string Get_list_markets();

    // Get list of all avialable currency pairs groups
    string Get_list_currency_pairs_groups();

    // Available currency pairs
    string Get_currency_pairs(string code);

    // Available currency pairs for a given group
    string Get_currency_pairs_given_group(string currencyPairGroupId);

    // Get currency pair information
    string Get_currency_pair_information(string currencyPairId);

    // Tickers list for all currency pairs
    string Get_ticker_list_currency_pairs();

    // Ticker for currency pair
    string Get_ticker_currency_pair(string currencyPairId);

    // Trades for given currency pair
    /* parameter
    sort
    string
    (query)	
    Direction of the sort - ASCending or DESCending by trade timestamp

    Available values : DESC, ASC

    Default value : DESC

    from
    integer
    (query)	
    The timestamp in seconds

    till
    integer
    (query)	
    The timestamp in seconds

    limit
    integer
    (query)	
    Default value : 100

    offset
    integer
    (query) 
     */
    string Get_trades_currency_pair(string currencyPairId, string sort = "", string from = "", string till = "", string limit = "", string offset = "");

    // Orderbook for given currency pair
    /*
    parameter
    limit_bids
    integer
    (query)	
    Return only top N bids. Returns all if set to -1

    Default value : 20

    limit_asks
    integer
    (query)	
    Return only top N asks. Returns all if set to -1

    Default value : 20
    */
    string Get_orderbook_currency_pair(string currencyPairId, string limit_bids = "", string limit_asks = "");

    // A list of candles for given currency pair
    /*
    parameter
    currencyPairId *
    string
    (path)	
    A currency pair ID you want to get candles for

    candlesType *
    string
    (path)	
    Candle size 1 stays for 1 minute, 5 - 5 minutes and so on. 1D - stays for 1 day

    Available values : 1, 5, 30, 60, 240, 720, 1D

    Default value : 1D

    timeStart *
    integer
    (query)	
    Timestamp in second. Should be less then timeEnd

    timeEnd *
    integer
    (query)	
    Timestamp in second. Should be greater then timeStart

    limit
    integer
    (query)	
    Default value : 100

    offset
    integer
    (query)	
    */
    string Get_list_candles_currency_pair(string currencyPairId, string candlesType, string timeStart, string timeEnd, string limit = "", string offset = "");

    // Available Deposit Statuses
    string Get_list_deposit_statuses();

    // Get deposit status info
    string Get_deposit_status(string statusId);

    // Available Withdrawal Statuses
    string Get_list_withdrawal_statuses();

    // Get status info
    string Get_withdrawal_status(string statusId);

    // Test API is working and get server time
    string ping();

    // Shows the official mobile applications data
    string Get_mobile_version();

    // Get the last 20 posts (stex.com) on Twitter
    string Get_twitter();


    //Trading
    //Returns the user's fees for a given currency pair
    string Get_user_fee_currency_pair(string currencyPairId);

    //List your currently open orders
    /*
    parameter
    limit   integer(query)	    Default value : 100

    offset  integer(query)
    */
    string Get_list_open_all_orders(string limit = "", string offset = "");

    //Delete all active orders
    string Delete_all_active_orders();

    //List your currently open orders for given currency pair
    string Get_list_open_order_by_currency_pair(string currencyPairId, string limit = "", string offset = "");

    //Delete active orders for given currency pair
    string Delete_order_by_currency_pair(string currencyPairId);

    //Create new order and put it to the orders processing queue
    /*
    parameter
    type *          string      order type (BUY / SELL / STOP_LIMIT_BUY / STOP_LIMIT_SELL)

    amount *        number
    price *         number
    trigger_price   number      stop price for stop-limit orders. Required if the order is of type STOP_LIMIT_BUY or STOP_LIMIT_SELL
    */
    string Creat_new_order(string currencyPairId, string type, string amount, string price, string trigger_price = "");

    //Get a single order
    string Get_single_order(string orderId);

    //Cancel order
    string Delete_order(string orderId);

    //Trading History & Reports
    //Get a list of currencies user had any activity in
    /*
    parameter
    key *       Available values : Deposits, Withdrawals, Burn, Reward, Investments
    string
    (query)	
    */
    string Get_list_currency_user_activity(string key);

    //Gets the list of currency pairs the user had orders in for all the time
    string Get_list_all_currencypairs_by_user();

    //Get past orders
    /*
    parameter
    currencyPairId  integer (query)	
    orderStatus     string  (query)	    Available values : ALL, FINISHED, CANCELLED, PARTIAL, WITH_TRADES   Default value : ALL
    timeStart       integer (query)	    Timestamp in seconds.
    timeEnd         integer (query)	    Timestamp in seconds.
    limit           integer (query)	    Default value : 100
    offset          integer (query)	
    */
    string Get_past_orders(string currencyPairId = "", string orderStatus = "", string timeStart = "", string timeEnd = "", string limit = "", string offset = "");

    //Get specified order details
    string Get_order_details(string orderId);

    //Get a list of user trades according to request parameters
    string Get_list_user_spec_trades(string currencyPairId, string timeStart = "", string timeEnd = "", string limit = "", string offset = "");

    //Get reports list for category
    /*
    listMode *      string(path)	Available values : all, recently, scheduled
    */
    string Get_reports_list_category(string listMode, string limit = "", string offset = "");

    //Get some report info
    string Get_report_info(string id);

    //Remove report by id
    string Delete_report_by_id(string id);

    //Create new report
    /*
    type        array[string](query)	["All", "Deposits", "Withdrawals", "Orders"]
    format *    string(query)	        Available values : Html, Csv, Xls, Pdf
    */
    string Create_new_report(string name, string date_from, string date_to, string format, string type = "");

    //Get file by id
    string Get_file_by_id(string id);

    //Settings
    //User event notification settings
    string Get_list_notification_by_event(string event);

    //User event notification settings
    string Get_list_notification();

    //Set notification settings
    /*
    event *     string(query)	An event name you want to subscribe.
    channel *   string(query)	A channel name you want to receive the notification through.
    value *     integer(query)	1 - to subscribe to the notifications of the given event in the specified channel, 0 - to remove subscription of the specified event in the specified channel 
    Available values : 0, 1
    */
    string Set_notification_settings(string event, string channel, string value);

    //Set notification settings
    string Set_notification_settings_one_request();


    //Profile
    //Account Information
    /*
    show_balances   string(query)	if present and is > 0, the response will include the 'approx_balance' section. It will be returned as null if this parameter is not present or is not positive number
    */
    string Get_accoutn_information(string show_balances = "");

    //Get a list of user wallets
    /*
    sort string(query)	Sort direction. Available values : DESC, ASC  Default value : DESC 
    sortBy  string(query)	Sort direction.    Available values : BALANCE, FROZEN, BONUS, HOLD, TOTAL  Default value : BALANCE
    */
    string Get_list_user_wallets(string sort = "", string sortBy = "");

    //Single wallet information
    string Get_single_user_wallet(string walletId);

    //Burns the given wallet
    string Burn_wallet(string walletId);

    //Create a wallet for given currency
    /*
    protocol_id integer(query)	Default value : The value that represents legacy protocol (in case of USDT it is 10 as Tether OMNI was default before multi-currency approach used). The list of values can be obtained from the /public/currencies/{currencyId} endpoint that returns the list of all available protocols for a given currency
    */
    string Create_wallet(string currencyId, string protocol_id = "");

    //Get deposit address for given wallet
    /*
    protocol_id integer(query)	Default value : The value that represents legacy protocol (in case of USDT it is 10 as Tether OMNI was default before multi-currency approach used). The list of values can be obtained from the /public/currencies/{currencyId} endpoint that returns the list of all available protocols for a given currency
    */
    string Get_deposit_address_wallet(string walletId, string protocol_id = "");

    //Create new deposit address
    /*
    protocol_id integer(query)	Default value : The value that represents legacy protocol (in case of USDT it is 10 as Tether OMNI was default before multi-currency approach used). The list of values can be obtained from the /public/currencies/{currencyId} endpoint that returns the list of all available protocols for a given currency
    */
    string Create_new_deposit_address(string walletId, string protocol_id = "");

    //Get a list of deposits made by user
    /*
    currencyId  integer(query)	the ID of the currency to filter results by
    sort    string(query)	Sort direction. Results are always ordered by date. You may adjust the order direction here    
    Available values : DESC, ASC    Default value : DESC
    timeStart   integer(query)	Timestamp in seconds
    timeEnd     integer(query)	Timestamp in seconds
    limit       integer(query)	Default value : 100
    offset      integer(query)	
    */
    string Get_list_user_deposit(string currencyId = "", string sort = "", string timeStart = "", string timeEnd = "", string limit = "", string offset = "");

    //Get deposit by id
    string Get_deposit(string id);

    //Get a list of rewards obtained by user (e.g. in trading competitions)
    /*
    currencyId  integer(query)	the ID of the currency to filter results by
    sort    string(query)	Sort direction. Results are always ordered by date. You may adjust the order direction here    
    Available values : DESC, ASC    Default value : DESC
    timeStart   integer(query)	Timestamp in seconds
    timeEnd     integer(query)	Timestamp in seconds
    limit       integer(query)	Default value : 100
    offset      integer(query)	
    */
    string Get_list_rewards(string currencyId = "", string sort = "", string timeStart = "", string timeEnd = "", string limit = "", string offset = "");

    //Get reward by id
    string Get_reward(string id);

    //Get a list of user address book items
    string Get_list_user_address();

    //Single address book item
    string Get_single_address(string itemId);

    //Deletes address book item
    string Delete_address_book(string itemId);

    //Disables the address book item
    string Disable_address_book_item(string itemId);

    //Enable the address book item
    string Enable_address_book_item(string itemId);

    //Restrict the withdrawals to only addresses that are active in addressbook
    string Restrict_withdrawal_addressbook();

    //Remove restriction to withdraw to only addresses that are active in addressbook. E.g. allow to withdraw to any address.
    string Allow_withdrawal_addressbook();

    //Get a list of withdrawals made by user
    /*
    currencyId  integer(query)	the ID of the currency to filter results by
    sort    string(query)	Sort direction. Results are always ordered by date. You may adjust the order direction here    
    Available values : DESC, ASC    Default value : DESC
    timeStart   integer(query)	Timestamp in seconds
    timeEnd     integer(query)	Timestamp in seconds
    limit       integer(query)	Default value : 100
    offset      integer(query)	
    */
    string Get_list_withdrawal(string currencyId = "", string sort = "", string timeStart = "", string timeEnd = "", string limit = "", string offset = "");

    //Get withdrawal by id
    string Get_withdrawal(string id);

    //Create withdrawal request
    /*
    protocol_id integer     This optional parameter has to be used only for multicurrency wallets (for example for USDT). The list of possible values can be obtained in wallet address for such a currency
    additional_address_parameter    string  If withdrawal address requires the payment ID or some key or destination tag etc pass it here
    one_time_code   string  Optional Google Authenticator one-time code
    */
    string Create_withdrawal_request(string currencyId, string amount, string address, string protocol_id = "",  string additional_address_parameter = "",  string one_time_code = "");

    //Cancel unconfirmed withdrawal
    string Cancel_unconfirmed_withdrawal(string withdrawalId);

    //Get notifications
    string Get_notifications(string limit  = "", string offset = "");

    //Get a list of active price alerts
    string Get_list_active_price_alert(string currencyPairId  = "");

    //Create new price alert
    /*
    comparison *    string      One of the 'GREATER' or 'LESS'
    */
    string Create_new_price_alert(string currencyPairId, string comarison, string price);

    //Delete the price alert by ID
    string Delete_price_alert(string priceAlertId);

    //Create referral program
    string Create_referral_program();

    //Insert referral code
    string Insert_referral_code(string code);

    //Transfer referral bonuses balance to main balance for given currency
    string Transfer_referral_bonuses(string currencyId);

    //Get favorite currency pairs
    string Get_fav_currency_pair();

    //Set favorite currency pairs
    /*
    addPairIds  array[integer](query)	add ids of currency pairs to list
    removePairIds   array[integer](query)	remove ids of currency pairs from list
    show        boolean(query)
    */
    string Set_fav_currency_pair(string addPairIds, string removePairIds = "", string show = "");

    //Get current token scopes
    string Get_current_token_scopes();







};



#endif // STEXAPI_H
