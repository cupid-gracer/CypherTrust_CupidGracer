// STEX.h
#ifndef STEX_H
#define STEX_H

#include "exchange/STEX/API.h"
#include <string>
#include <vector>

using namespace std;

class STEX
{
public:
    STEX();
    ~STEX();
    /* Declare Variables */
    string uri;
    string product_id;
    API api;


    //Public
    //1 Available Currencies
    void Get_list_currencies();

    //2 Get currency info
    void Get_currency_info();

    //3 Available markets
    void Get_list_markets();

    //4 Get list of all avialable currency pairs groups
    void Get_list_currency_pairs_groups();

    //5 Available currency pairs
    void Get_currency_pairs();

    //6 Available currency pairs for a given group
    void Get_currency_pairs_given_group();

    //7 Get currency pair information
    void Get_currency_pair_information();

    //8 Tickers list for all currency pairs
    //  isUse:  true -> will use the currency pairs in conf file, or not
    void Get_ticker_list_currency_pairs(bool isUse = false);

    //9 Ticker for currency pair
    void Get_ticker_currency_pair();

    //10 Trades for given currency pair
    void Get_trades_currency_pair();

    //11 Orderbook for given currency pair
    void Get_orderbook_currency_pair();

    //12 A list of candles for given currency pair
    void Get_list_candles_currency_pair();

    //13 Available Deposit Statuses
    void Get_list_deposit_statuses();

    //14 Get deposit status info
    void Get_deposit_status();

    //15 Available Withdrawal Statuses
    void Get_list_withdrawal_statuses();

    //16 Get status info
    void Get_withdrawal_status();

    //17 Test API is working and get server time
    void ping();

    //18 Shows the official mobile applications data
    void Get_mobile_version();

    //19 Get the last 20 posts (stex.com) on Twitter
    void Get_twitter();

    //Trading
    //20 Returns the user's fees for a given currency pair
    void Get_user_fee_currency_pair();

    //21 List your currently open orders
    void Get_list_open_all_orders();

    //22 Delete all active orders
    void Delete_all_active_orders();

    //23 List your currently open orders for given currency pair
    void Get_list_open_order_by_currency_pair();

    //24 Delete active orders for given currency pair
    void Delete_order_by_currency_pair();

    //25 Create new order and put it to the orders processing queue
    void Creat_new_order();

    //26 Get a single order
    void Get_single_order();

    //27 Cancel order
    void Delete_order();

    //trading history
    //28 Get a list of currencies user had any activity in
    void Get_list_currency_user_activity();

    //29 Gets the list of currency pairs the user had orders in for all the time
    void Get_list_all_currencypairs_by_user();

    //30 Get past orders
    void Get_past_orders();

    //31 Get specified order details
    void Get_order_details();

    //32 Get a list of user trades according to request parameters
    void Get_list_user_spec_trades();

    //33 Get reports list for category
    void Get_reports_list_category();

    //34 Get some report info
    void Get_report_info();

    //35 Remove report by id
    void Delete_report_by_id();

    //36 Create new report
    void Create_new_report();

    //37 Get file by id
    void Get_file_by_id();

    //Settings
    //38 User event notification settings by event
    void Get_list_notification_by_event();

    //39 User event notification settings
    void Get_list_notification();

    //40 Set notification settings in specific channel for the user
    void Set_notification_settings();

    //41 Set notification settings in many channels for the user at one request
    void Set_notification_settings_one_request();

    //Profile
    //42 Account Information 
    void Get_accoutn_information();

    //43 Get a list of user wallets
    void Get_list_user_wallets();

    //44 Single wallet information
    void Get_single_user_wallet();

    //45 Burns the given wallet
    void Burn_wallet();

    //46 Create a wallet for given currency
    void Create_wallet();

    //47 Get a list of deposits made by user
    void Get_list_user_deposit();

    //48 Get deposit by id
    void Get_deposit();

    //49 Get a list of rewards obtained by user (e.g. in trading competitions)
    void Get_list_rewards();

    //50 Get reward by id
    void Get_reward();

    //51 Get a list of user address book items
    void Get_list_user_address();

    //52 Single address book item
    void Get_single_address();

    //53 Deletes address book item
    void Delete_address_book();

    //54 Disables the address book item
    void Disable_address_book_item();

    //55 Disables the address book item
    void Enable_address_book_item();

    //56 Restrict the withdrawals to only addresses that are active in addressbook
    void Restrict_withdrawal_addressbook();

    //57 Remove restriction to withdraw to only addresses that are active in addressbook. E.g. allow to withdraw to any address.
    void Allow_withdrawal_addressbook();

    //58 Get a list of withdrawals made by user
    void Get_list_withdrawal();

    //59 Get withdrawal by id
    void Get_withdrawal();

    //60 Create withdrawal request
    void Create_withdrawal_request();

    //61 Cancel unconfirmed withdrawal
    void Cancel_unconfirmed_withdrawal();

    
    //62 Get deposit address for given wallet
    void Get_deposit_address_wallet();

    //63 Create new deposit address
    void Create_new_deposit_address();

    //64 Get notifications
    void Get_notifications();

    //65 Get a list of active price alerts
    void Get_list_active_price_alert();

    //66 Create new price alert
    void Create_new_price_alert();

    //67 Delete the price alert by ID
    void Delete_price_alert();

    //68 Create referral program
    void Create_referral_program();

    //69 Insert referral code
    void Insert_referral_code();

    //70 Transfer referral bonuses balance to main balance for given currency
    void Transfer_referral_bonuses();

    //71 Get favorite currency pairs
    void Get_fav_currency_pair();

    //72 Set favorite currency pairs 
    void Set_fav_currency_pair();

    //73 Get current token scopes
    void Get_current_token_scopes();



};

#endif // STEX_H
