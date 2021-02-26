// Api.h
#ifndef API_H
#define API_H

#include <string>
#include <vector>
#include "Util.h"

class API
{
private:
    std::string Call(std::string method, bool authed, std::string path, std::string body);

public:
    API();
    ~API();
    Util util;
    std::string uri;
    std::string access_token;
    std::string product_id;
    std::string Get_Buy_Price();
    double Get_Balance(std::string currency);
    std::string Place_Limit_Order(std::string side, std::string price, std::string size);

    //Public
    // Available Currencies
    std::string Get_list_currencies();

    // Get currency info
    std::string Get_currency_info(std::string currencyId);

    // Available markets
    std::string Get_list_markets();

    // Get list of all avialable currency pairs groups
    std::string Get_list_currency_pairs_groups();

    // Available currency pairs
    std::string Get_currency_pairs(std::string code);

    // Available currency pairs for a given group
    std::string Get_currency_pairs_given_group(std::string currencyPairGroupId);

    // Get currency pair information
    std::string Get_currency_pair_information(std::string currencyPairId);

    // Tickers list for all currency pairs
    std::string Get_ticker_list_currency_pairs();

    // Ticker for currency pair
    std::string Get_ticker_currency_pair(std::string currencyPairId);

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
    std::string Get_trades_currency_pair(std::string currencyPairId, std::string sort = "", std::string from = "", std::string till = "", std::string limit = "", std::string offset = "");

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
    std::string Get_orderbook_currency_pair(std::string currencyPairId, std::string limit_bids = "", std::string limit_asks = "");

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
    std::string Get_list_candles_currency_pair(std::string currencyPairId, std::string candlesType, std::string timeStart, std::string timeEnd, std::string limit = "", std::string offset = "");

    // Available Deposit Statuses
    std::string Get_list_deposit_statuses();

    // Get deposit status info
    std::string Get_deposit_status(std::string statusId);

    // Available Withdrawal Statuses
    std::string Get_list_withdrawal_statuses();

    // Get status info
    std::string Get_withdrawal_status(std::string statusId);

    // Test API is working and get server time
    std::string ping();

    // Shows the official mobile applications data
    std::string Get_mobile_version();

    // Get the last 20 posts (stex.com) on Twitter
    std::string Get_twitter();


    //Trading
    //Returns the user's fees for a given currency pair
    std::string Get_user_fee_currency_pair(std::string currencyPairId);

    //List your currently open orders
    /*
    parameter
    limit   integer(query)	    Default value : 100

    offset  integer(query)
    */
    std::string Get_list_open_all_orders(std::string limit = "", std::string offset = "");

    //Delete all active orders
    std::string Delete_all_active_orders();

    //List your currently open orders for given currency pair
    std::string Get_list_open_order_by_currency_pair(std::string currencyPairId, std::string limit = "", std::string offset = "");

    //Delete active orders for given currency pair
    std::string Delete_order_by_currency_pair(std::string currencyPairId);

    //Create new order and put it to the orders processing queue
    /*
    parameter
    type *          string      order type (BUY / SELL / STOP_LIMIT_BUY / STOP_LIMIT_SELL)

    amount *        number
    price *         number
    trigger_price   number      stop price for stop-limit orders. Required if the order is of type STOP_LIMIT_BUY or STOP_LIMIT_SELL
    */
    std::string Creat_new_order(std::string currencyPairId, std::string type, std::string amount, std::string price, std::string trigger_price = "");

    //Get a single order
    std::string Get_single_order(std::string orderId);

    //Cancel order
    std::string Delete_order(std::string orderId);

    //Trading History & Reports
    //Get a list of currencies user had any activity in
    /*
    parameter
    key *       Available values : Deposits, Withdrawals, Burn, Reward, Investments
    string
    (query)	
    */
    std::string Get_list_currency_user_activity(std::string key);

    //Gets the list of currency pairs the user had orders in for all the time
    std::string Get_list_all_currencypairs_by_user();

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
    std::string Get_past_orders(std::string currencyPairId = "", std::string orderStatus = "", std::string timeStart = "", std::string timeEnd = "", std::string limit = "", std::string offset = "");

    //Get specified order details
    std::string Get_order_details(std::string orderId);

    //Get a list of user trades according to request parameters
    std::string Get_list_user_spec_trades(std::string currencyPairId, std::string timeStart = "", std::string timeEnd = "", std::string limit = "", std::string offset = "");

    //Get reports list for category
    /*
    listMode *      string(path)	Available values : all, recently, scheduled
    */
    std::string Get_reports_list_category(std::string listMode, std::string limit = "", std::string offset = "");

    //Get some report info
    std::string Get_report_info(std::string id);

    //Remove report by id
    std::string Delete_report_by_id(std::string id);

    //Create new report
    /*
    type        array[string](query)	["All", "Deposits", "Withdrawals", "Orders"]
    format *    string(query)	        Available values : Html, Csv, Xls, Pdf
    */
    std::string Create_new_report(std::string name, std::string date_from, std::string date_to, std::string format, std::string type = "");

    //Get file by id
    std::string Get_file_by_id(std::string id);

    //Settings
    //User event notification settings
    std::string Get_list_notification_by_event(std::string event);

    //User event notification settings
    std::string Get_list_notification();

    //Set notification settings
    /*
    event *     string(query)	An event name you want to subscribe.
    channel *   string(query)	A channel name you want to receive the notification through.
    value *     integer(query)	1 - to subscribe to the notifications of the given event in the specified channel, 0 - to remove subscription of the specified event in the specified channel 
    Available values : 0, 1
    */
    std::string Set_notification_settings(std::string event, std::string channel, std::string value);

    //Set notification settings
    std::string Set_notification_settings_one_request();


    //Profile
    //Account Information
    /*
    show_balances   string(query)	if present and is > 0, the response will include the 'approx_balance' section. It will be returned as null if this parameter is not present or is not positive number
    */
    std::string Get_accoutn_information(std::string show_balances = "");

    //Get a list of user wallets
    /*
    sort string(query)	Sort direction. Available values : DESC, ASC  Default value : DESC 
    sortBy  string(query)	Sort direction.    Available values : BALANCE, FROZEN, BONUS, HOLD, TOTAL  Default value : BALANCE
    */
    std::string Get_list_user_wallets(std::string sort = "", std::string sortBy = "");

    //Single wallet information
    std::string Get_single_user_wallet(std::string walletId);

    //Burns the given wallet
    std::string Burn_wallet(std::string walletId);

    //Create a wallet for given currency
    /*
    protocol_id integer(query)	Default value : The value that represents legacy protocol (in case of USDT it is 10 as Tether OMNI was default before multi-currency approach used). The list of values can be obtained from the /public/currencies/{currencyId} endpoint that returns the list of all available protocols for a given currency
    */
    std::string Create_wallet(std::string currencyId, std::string protocol_id = "");

    //Get deposit address for given wallet
    /*
    protocol_id integer(query)	Default value : The value that represents legacy protocol (in case of USDT it is 10 as Tether OMNI was default before multi-currency approach used). The list of values can be obtained from the /public/currencies/{currencyId} endpoint that returns the list of all available protocols for a given currency
    */
    std::string Get_deposit_address_wallet(std::string walletId, std::string protocol_id = "");

    //Create new deposit address
    /*
    protocol_id integer(query)	Default value : The value that represents legacy protocol (in case of USDT it is 10 as Tether OMNI was default before multi-currency approach used). The list of values can be obtained from the /public/currencies/{currencyId} endpoint that returns the list of all available protocols for a given currency
    */
    std::string Create_new_deposit_address(std::string walletId, std::string protocol_id = "");

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
    std::string Get_list_user_deposit(std::string currencyId = "", std::string sort = "", std::string timeStart = "", std::string timeEnd = "", std::string limit = "", std::string offset = "");

    //Get deposit by id
    std::string Get_deposit(std::string id);

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
    std::string Get_list_rewards(std::string currencyId = "", std::string sort = "", std::string timeStart = "", std::string timeEnd = "", std::string limit = "", std::string offset = "");

    //Get reward by id
    std::string Get_reward(std::string id);

    //Get a list of user address book items
    std::string Get_list_user_address();

    //Single address book item
    std::string Get_single_address(std::string itemId);

    //Deletes address book item
    std::string Delete_address_book(std::string itemId);

    //Disables the address book item
    std::string Disable_address_book_item(std::string itemId);

    //Enable the address book item
    std::string Enable_address_book_item(std::string itemId);

    //Restrict the withdrawals to only addresses that are active in addressbook
    std::string Restrict_withdrawal_addressbook();

    //Remove restriction to withdraw to only addresses that are active in addressbook. E.g. allow to withdraw to any address.
    std::string Allow_withdrawal_addressbook();

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
    std::string Get_list_withdrawal(std::string currencyId = "", std::string sort = "", std::string timeStart = "", std::string timeEnd = "", std::string limit = "", std::string offset = "");

    //Get withdrawal by id
    std::string Get_withdrawal(std::string id);

    //Create withdrawal request
    /*
    protocol_id integer     This optional parameter has to be used only for multicurrency wallets (for example for USDT). The list of possible values can be obtained in wallet address for such a currency
    additional_address_parameter    string  If withdrawal address requires the payment ID or some key or destination tag etc pass it here
    one_time_code   string  Optional Google Authenticator one-time code
    */
    std::string Create_withdrawal_request(std::string currencyId, std::string amount, std::string address, std::string protocol_id = "",  std::string additional_address_parameter = "",  std::string one_time_code = "");

    //Cancel unconfirmed withdrawal
    std::string Cancel_unconfirmed_withdrawal(std::string withdrawalId);

    //Get notifications
    std::string Get_notifications(std::string limit  = "", std::string offset = "");

    //Get a list of active price alerts
    std::string Get_list_active_price_alert(std::string currencyPairId  = "");

    //Create new price alert
    /*
    comparison *    string      One of the 'GREATER' or 'LESS'
    */
    std::string Create_new_price_alert(std::string currencyPairId, std::string comarison, std::string price);

    //Delete the price alert by ID
    std::string Delete_price_alert(std::string priceAlertId);

    //Create referral program
    std::string Create_referral_program();

    //Insert referral code
    std::string Insert_referral_code(std::string code);

    //Transfer referral bonuses balance to main balance for given currency
    std::string Transfer_referral_bonuses(std::string currencyId);

    //Get favorite currency pairs
    std::string Get_fav_currency_pair();

    //Set favorite currency pairs
    /*
    addPairIds  array[integer](query)	add ids of currency pairs to list
    removePairIds   array[integer](query)	remove ids of currency pairs from list
    show        boolean(query)
    */
    std::string Set_fav_currency_pair(std::string addPairIds, std::string removePairIds = "", std::string show = "");

    //Get current token scopes
    std::string Get_current_token_scopes();







};



#endif // API_H
