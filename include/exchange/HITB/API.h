// Api.h
#ifndef HITB_API
#define HITB_API

#include <string>
#include <vector>
#include "Util.h"

using namespace std;

class HITBAPI
{
private:
    string Call(string method, bool authed, string path, string body);

public:
    HITBAPI();
    ~HITBAPI();
    Util util;
    string  uri,
            token,
            redisURL,
            redisConnectorChannel,
            addressID;

    //Public
    // Available symbols
    //parameter  ex: ETHBTC,BETBTC...
    string Get_list_symbols(string symbols = "");

    // Get symbol info
    string Get_symbol_info(string symbol);

    // Available currencies
    //parameter  ex: ETHBTC,BETBTC...
    string Get_list_currencies(string currencies = "");

    // Get currency info
    string Get_currency_info(string currency);

    // Get Tickers
    //parameter  ex: ETHBTC,BETBTC...
    string Get_list_tickers(string symbols = "");

    // Get Tickers
    string Get_ticker(string symbol);

    // Get Trades
    /*
    parameters
    symbols     string	            symbols separated by commas; if empty - all symbols by default
    sort        string	            Sort direction    Available values : DESC, ASC    Default value : DESC
    from        string	            datetime in iso format or timestamp in millisecond
    till        string	            datetime in iso format or timestamp in millisecond
    limit       integer($int32)     Allowed from 1 to 1000    Default value : 100
    offset      integer($int32)     Allowed from 0 to 100000
    */
    string Get_list_trades(string symbols = "", string sort = "", string from = "", string till = "", string limit = "", string offset = "");

    //Get Trade
    /*
    parameters
    symbol *    string      (path)	
    sort        string      (query)	            Sort direction    Available values : DESC, ASC    Default value : DESC
    by          string      (query)	            Filter field    Available values : timestamp, id    Default value : timestamp
    from        string      (query)	            If filter by timestamp, then datetime in iso format or timestamp in millisecond otherwise trade id
    till        string      (query)	            If filter by timestamp, then datetime in iso format or timestamp in millisecond otherwise trade id (inclusive)
    limit       integer     ($int32)(query)	    Allowed from 1 to 1000    Default value : 100
    offset      integer     ($int32)(query)
    */
    string Get_trade(string symbol, string sort = "", string by = "", string from = "", string till = "", string limit = "", string offset = "");

    //Get list order
    /*
    parameters
    symbols     string      (query)	    symbols separated by commas; if empty - all symbols by default
    limit       integer     (query)	    0 - full orderbook otherwise number of levels    Default value : 10
    */
    string Get_list_orderbook(string symbols = "", string limit = "");

    //Get order
    /*
    parameters
    symbols*    string      (query)	    symbols separated by commas; if empty - all symbols by default
    limit       integer     (query)	    0 - full orderbook otherwise number of levels    Default value : 10
    */
    string Get_orderbook(string symbol, string limit = "", string volumn = "");

    //Get Candles
    /*
    parameters
    symbols     string(query)	            symbols separated by commas; if empty - all symbols by default
    period      string(query)	            Available values : M1, M3, M5, M15, M30, H1, H4, D1, D7, 1M    Default value : M30
    sort        string(query)	            Sort direction    Available values : DESC, ASC    Default value : ASC
    from        string(query)	            Datetime in iso format or timestamp in millisecond
    till        string(query)	            Datetime in iso format or timestamp in millisecond
    limit       integer($int32)(query)	    Allowed from 1 to 1000    Default value : 100
    offset      integer($int32)(query)	    Allowed from 0 to 100000
    */
    string Get_list_candles(string symbols = "", string period = "", string sort = "", string from = "", string till = "", string limit = "", string offset = "");

    //Get Candles
    /*
    parameters
    symbol*     string(path)	            symbols separated by commas; if empty - all symbols by default
    period      string(query)	            Available values : M1, M3, M5, M15, M30, H1, H4, D1, D7, 1M    Default value : M30
    sort        string(query)	            Sort direction    Available values : DESC, ASC    Default value : ASC
    from        string(query)	            Datetime in iso format or timestamp in millisecond
    till        string(query)	            Datetime in iso format or timestamp in millisecond
    limit       integer($int32)(query)	    Allowed from 1 to 1000    Default value : 100
    offset      integer($int32)(query)	    Allowed from 0 to 100000
    */
    string Get_candle(string symbol, string period = "", string sort = "", string from = "", string till = "", string limit = "", string offset = "");

    //Trading
    //Get List open orders
    string Get_list_orders(string symbol = "");
    
    //Create new order
    /*
    parameters
    clientOrderId    string    (formData)	
    symbol *    string    (formData)	
    side *    string    (formData)	    Available values : sell, buy    Default value : sell
    type    string    (formData)	    Available values : limit, market, stopLimit, stopMarket    Default value : limit
    timeInForce    string    (formData)	   Available values : GTC, IOC, FOK, Day, GTD    Default value : GTC
    quantity *    string    (formData)	
    price    string    (formData)	
    stopPrice    string    (formData)	
    expireTime    string($date-time)    (formData)	
    strictValidate    boolean    (formData)	    Strict validate amount and price precision without rounding    Default value : false
    postOnly    boolean    (formData)	    A Post-Only Order is an order that does not remove liquidity. If your post-only order would cause a match with a pre-existing order as a taker, then order will be canceled    Default value : false
    takeLiquidityMarkup    string    (formData)	    Optional liquidity taker fee, a fraction of order volume, such as 0.001 (for 0.1% fee)
    provideLiquidityMarkup    string    (formData)	    Optional liquidity provider fee, a fraction of order volume, such as 0.001 (for 0.1% fee)
    */
    string Create_new_order_post(string symbol, string side, string quantity,  string clientOrderId = "", string type = "",   string timeInForce = "",    string price = "",   string stopPrice = "",   string expireTime = "",   string strictValidate = "",   string postOnly = "",   string takeLiquidityMarkup = "",   string provideLiquidityMarkup = "");

    //Delete order
    string Cancel_all_order(string symbol);

    // order
    string Get_single_order(string clientOrderId, string wait);

    //Create new order
    /*
    parameters
    clientOrderId    string    (formData)	
    symbol *    string    (formData)	
    side *    string    (formData)	    Available values : sell, buy    Default value : sell
    type    string    (formData)	    Available values : limit, market, stopLimit, stopMarket    Default value : limit
    timeInForce    string    (formData)	   Available values : GTC, IOC, FOK, Day, GTD    Default value : GTC
    quantity *    string    (formData)	
    price    string    (formData)	
    stopPrice    string    (formData)	
    expireTime    string($date-time)    (formData)	
    strictValidate    boolean    (formData)	    Strict validate amount and price precision without rounding    Default value : false
    postOnly    boolean    (formData)	    A Post-Only Order is an order that does not remove liquidity. If your post-only order would cause a match with a pre-existing order as a taker, then order will be canceled    Default value : false
    takeLiquidityMarkup    string    (formData)	    Optional liquidity taker fee, a fraction of order volume, such as 0.001 (for 0.1% fee)
    provideLiquidityMarkup    string    (formData)	    Optional liquidity provider fee, a fraction of order volume, such as 0.001 (for 0.1% fee)
    */
    string Create_new_order_put(string clientOrderId, string symbol, string side,  string timeInForce,   string quantity,   string type = "",  string price = "",   string stopPrice = "",   string expireTime = "",   string strictValidate = "",   string postOnly = "",   string takeLiquidityMarkup = "",   string provideLiquidityMarkup = "");

    //Cancel order
    string Cancel_order(string clientOrderId);

    //Cancel Replace order
    string Cancel_replace_order(string clientOrderId, string quantity, string requestClientId, string price = "");

    //Get trading balance
    string Get_trading_balance();

    //Get trading fee rate
    string Get_trading_fee_rate(string symbol);

    //MarginTrading
    //List of your currently open margin orders
    string Get_list_margin_orders(string symbol = "");

    //Create new margin orders
    string Create_new_margin_order_post(string symbol, string side, string quantity,  string clientOrderId = "", string type = "",   string timeInForce = "",    string price = "",   string stopPrice = "",   string expireTime = "",   string strictValidate = "",   string postOnly = "",   string takeLiquidityMarkup = "",   string provideLiquidityMarkup = "");

    //Create new margin orders
    string Create_new_margin_order_put(string clientOrderId, string symbol, string side,  string timeInForce,   string quantity,  string type = "",   string price = "",   string stopPrice = "",   string expireTime = "",   string strictValidate = "",   string postOnly = "",   string takeLiquidityMarkup = "",   string provideLiquidityMarkup = "");

    //Cancel all open margin orders
    string Cancel_all_margin_orders(string symbol = "");

    //Get a single margin order by clientOrderId
    string Get_single_margin_order(string clientOrderId);

    //Cancel a margin order
    string Cancel_single_margin_order(string clientOrderId);

    //Cancel-Replace margin order
    string Cancel_replace_margin_order(string clientOrderId, string quantity, string requestClientId, string price = "");

    //MarginAccount
    //Get list current margin accounts
    string Get_list_margin_accounts();

    //Close all margin accounts with their positions
    string Close_all_margin_accounts();

    //Get margin account by symbol
    string Get_margin_account(string symbol);
    
    //Set up a margin accout and reserve margin balance for it
    /*
    parameters
    symbol *            string(path)	
    marginBalance *     string($double)(formData)	    amount of currency to reserve for margin trading purposes.
    strictValidate      boolean(formData)	            force stric validate margin value    Default value : falase
    */
    string Set_margin_account(string symbol, string marginBalance, string strictValidate = "");

    //Close the margin account with theire position
    string Close_single_margin_account(string symbol);

    //Get list all open position
    string Get_list_open_position();

    //Cloase all open position
    string Close_all_open_position();
    
    //Get open poosition for a symbol
    string Get_single_open_position(string symbol);

    //Close the position for a symbol
    string Close_single_position(string symbol, string price = "", string strictValidate = "");


    //TradingHistory
    //Get historical trades
    /*
    parameters
    symbol      string(query)	
    sort        string(query)	    Sort direction    Available values : DESC, ASC    Default value : DESC
    by          string(query)	    Filter field    Available values : timestamp, id    Default value : id
    from        string(query)	    If filter by timestamp, then datetime in iso format or timestamp in millisecond otherwise trade id
    till        string(query)	    If filter by timestamp, then datetime in iso format or timestamp in millisecond otherwise trade id
    limit       integer($int32)(query)	    Default value : 100
    offset      integer($int32)(query)	
    margin      string(query)	    Controls inclusion of marginal trades    Available values : include, only, ignore
    */
    string Get_historical_trades(string symbol = "", string sort = "", string by = "", string from = "", string till = "", string limit = "", string offset = "", string margin = "");

    //Get historical orders
    /*
    parameters
    symbol          string(query)	
    from            string(query)	        Datetime in iso format or timestamp in millisecond.
    till            string(query)	        Datetime in iso format or timestamp in millisecond.
    limit           integer($int32)(query)	Default value : 100
    offset          integer($int32)(query)	
    clientOrderId   string(query)	
    */
    string Get_historical_orders(string symbol = "", string from = "", string till = "", string limit = "", string offset = "", string clientOrderId = "");

    //Get historical trades by specified order
    string Get_historical_by_orderid(string id);


    //Sub-Accounts
    //Get sub-accounts list
    string Get_list_sub_accounts();

    //Freeze sub-accounts
    /*
    parameter
    ids *string(formData)	subAccount userIds separate by ‘,’
    */
    string Freeze_sub_accounts(string ids);

    //Active sub-accounts
    /*
    parameter
    ids *string(formData)	subAccount userIds separate by ‘,’
    */
    string Active_sub_accounts(string ids);

    //Transfer from main to sub-account or from sub-account to main
    /*
    parameters
    subAccountId *      integer   (formData)	
    amount *            string    (formData)	
    currency *          string    (formData)	
    type *              string    (formData)	    Available values : deposit, withdraw
    */
    string Transfer_main_sub_account(string subAccountId, string amount, string currency, string type);

    //Sub account withdraw setting list
    string Get_list_withdraw_sub_account(string subAccountIds);

    //disable or enable withdraw for sub-accounts
    /*
    parameters
    subAccountUserID *      string    (path)	    subAccount userIds separate by ‘,’
    isPayoutEnabled         boolean   (formData)	is withdrow enabled for sub-accounts
    description *           string    (formData)	description
    */
    string Switch_status_withdraw_sub_accounts(string subAccountUserID, string descrption, string isPayoutEnabled = "");

    //get main and trading sub-account balances
    string Get_main_trading_balances(string subAccountUserID);

    //Get sub-account crypto address
    /*
    subAccountUserID *      integer   (path)	
    currency *              string    (path)
    */
    string Get_sub_account_crypto_address(string subAccountUserID, string currency);

    //Accounts
    //Get main account balance
    string Get_main_account_balance();

    //Get account transactions
    /*
    parameters
    currency    string(query)	
    sort        string(query)	            Sort direction    Available values : DESC, ASC    Default value : DESC
    by          string(query)	            Filter field    Available values : timestamp, index    Default value : timestamp
    from        string(query)	            Datetime in iso format or timestamp in millisecond, or index.
    till        string(query)	            Datetime in iso format or timestamp in millisecond, or index.
    limit       integer($int32)(query)	    Default value : 100
    offset      integer($int32)(query)
    */
    string Get_list_acount_transactions(string currency = "", string sort = "", string by = "", string from = "", string till = "", string limit = "", string offset = "");

    //Get account transaction by id
    string Get_single_account_transation(string id);

    //Withdraw crypto
    /*
    parameters
    currency *      string    (formData)	
    amount *        string    (formData)	
    address *       string    (formData)	
    paymentId       string    (formData)	
    includeFee      boolean   (formData)	    If enabled, then fee will be subtracted from amount.    Default value : false
    autoCommit      boolean   (formData)	    If Auto commit disabled you should commit it or rollback within 1 hour. After expires 1 hour, the transaction will automatically be rolled back.    Default value : true
    useOffchain     string    (formData)	    Allow create internal offchain transfer (fast, no fees, no hash). If required selected, then only offchain transaction can be created.  Available values : never, optionally, required    Default value : never
    feeLevelId      integer   (formData)	    Allows you to select the desired commission level. (The level affects the speed of transactions).
    publicComment   string    (formData)
    */
    string Withdraw_crypto(string currency, string amount, string address, string paymentId = "", string includeFee = "", string autoCommit = "", string useOffchain = "", string feeLevelId = "", string publicComment = "");

    //Commit withdraw crypto
    string Commit_withdraw_crypto(string id);

    //Rollback withdraw crypto
    string Rollback_withdraw_crypto(string id);

    //Check that offchain payout is avaliable for a destination address
    string Check_offchain_payout(string currency, string address, string paymentId = "");

    //transer convert between currencies
    string Transfer_convert_between_currencies(string fromCurrency, string toCurrency, string amount);

    //Estimate fee for withdraw
    string Estimate_fee_withdraw(string currency, string amount);

    //Estimates fee levels for withdraw
    string Estimate_fee_level_withdraw(string currency, string amount);

    //Check if crypto address belongs to current user
    string Check_crypto_address_current_user(string address);

    //Get deposit crypto address
    string Get_deposit_crypto_address(string currency);

    //Create new deposit crypto address
    string Create_new_deposit_crypto_address(string currency);

    //Get last 10 deposit crypto addresses
    string Get_last10_deposit_crypto_addresses(string currency);

    //Get last 10 unique payout addresses
    string Get_last10_unique_payout_addresses(string currency);

    //Transfer amount to trading
    /*
    parameters
    currency *      string    (formData)	
    amount *        string    (formData)	
    type *          string    (formData)	    Available values : bankToExchange, exchangeToBank
    */
    string Transfer_amount_trading(string currency, string amount, string type);

    //Transfer money to another user by email or username
    /*
    parameters
    currency *      string    (formData)	
    amount *        string    (formData)	
    by *            string    (formData)	    Available values : email, username
    identifier *    string    (formData)
    */
    string Transfer_money_another(string currency, string amount, string by, string identifier);




};



#endif // HITB_API
