#include "exchange/CBPR/CBPR.h"
#include "exchange/CBPR/Auth.h"
#include "exchange/CBPR/API.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>

#include <iostream>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"


using namespace rapidjson;
using namespace std;

void tokenize(string &str, char delim, std::vector<string> &out)
{
	size_t start;
	size_t end = 0;

	while ((start = str.find_first_not_of(delim, end)) != string::npos)
	{
		end = str.find(delim, start);
		out.push_back(str.substr(start, end - start));
	}
}

//1
void CBPR::Get_List_Accounts()
{
	api.Get_List_Accounts();
}
//2
void CBPR::Get_Account()
{
	string account_id;
	std::cout << "please enter account id:\t";
	std::cin >> account_id;
	std::cout << std::endl;
	api.Get_Account(account_id);
}
//3
void CBPR::Get_Account_History()
{
	string account_id;
	std::cout << "please enter account id:\t";
	std::cin >> account_id;
	std::cout << std::endl;
	api.Get_Account_History(account_id);
}
//4
void CBPR::Get_Holds()
{
	string account_id;
	std::cout << "please enter account id:\t";
	std::cin >> account_id;
	std::cout << std::endl;
	api.Get_Holds(account_id);
}
//5
void CBPR::Place_New_Order()
{
	double balance;
	string side;
	string price;
	string size;
	std::cout << "please enter side (sell/buy):\t";
	std::cin >> side;
	if(side == "sell"){
		balance =  api.Get_Balance("BTC");
	}
	std::cout << "please enter price:\t";
	std::cin >> price;
	std::cout << "please enter size:\t";
	std::cin >> size;
	std::cout << std::endl;
	if(side == "sell"){
		std::cout << "my current balance: " << balance << "  BTC" << std::endl;
	}
	api.Place_New_Order(side, price, size);
}
//6
void CBPR::Cancel_Order()
{
	string account_id;
	std::cout << "please enter Oder id:\t";
	std::cin >> account_id;
	std::cout << std::endl;
	api.Get_Holds(account_id);
}
//7
void CBPR::Cancel_All_Order()
{
	api.Cancel_All_Order();
}
//8
void CBPR::List_Orders()
{
	api.List_Orders();
}
//9
void CBPR::Get_Order()
{
	string account_id;
	std::cout << "please enter Oder id:\t";
	std::cin >> account_id;
	std::cout << std::endl;
	api.Get_Order(account_id);
}
//10
void CBPR::Get_Products()
{
	api.Get_Products();
}
//11
void CBPR::Get_Single_Product()
{
	api.Get_Single_Product();
}
//12
void CBPR::Get_Product_Order_Book()
{
	api.Get_Product_Order_Book();
}
//13
void CBPR::Get_Product_Ticker()
{
	api.Get_Product_Ticker();
}
//14
void CBPR::Get_Trades()
{
	api.Get_Trades();
}
//15
void CBPR::Get_Historic_Rates()
{
	api.Get_Historic_Rates();
}
//16
void CBPR::Get_24hr_Stats()
{
	api.Get_24hr_Stats();
}
//17
void CBPR::Get_Currencies()
{
	api.Get_Currencies();
}
//18
void CBPR::Get_Currency()
{
	string account_id;
	std::cout << "please enter Client id:\t";
	std::cin >> account_id;
	std::cout << std::endl;
	api.Get_Currency(account_id);
}
//19
void CBPR::Get_Time()
{
	api.Get_Time();
}
//20
void CBPR::List_Fills()
{
	string product_id;
	std::cout << "please enter product_id (BTC-USD):\t";
	std::cin >> product_id;
	api.List_Fills("", product_id);
}
//21
void CBPR::Get_Current_Exchange_Limits()
{
	api.Get_Current_Exchange_Limits();
}
//22
void CBPR::List_Payment_Methods()
{
	api.List_Payment_Methods();
}
//23
void CBPR::List_Coinbase_Accounts()
{
	api.List_Coinbase_Accounts();
}
//24
void CBPR::List_Deposits()
{
	api.List_Deposits();
}
//25
void CBPR::Single_Deposit()
{
	string transfer_id;
	std::cout << "please enter transfer_id:\t";
	std::cin >> transfer_id;
	std::cout << std::endl;
	api.Single_Deposit(transfer_id);
}
//26
void CBPR::Payment_Method_Deposit()
{
	string amount;
	std::cout << "please enter amount:\t";
	std::cin >> amount;
	string currency;
	std::cout << "please enter currency:\t";
	std::cin >> currency;
	string payment_method_id;
	std::cout << "please enter payment_method_id:\t";
	std::cin >> payment_method_id;
	std::cout << std::endl;
	api.Payment_Method_Deposit(amount, currency, payment_method_id);
}
//27
void CBPR::Coinbase_Deposit()
{
	string amount;
	std::cout << "please enter amount:\t";
	std::cin >> amount;
	string currency;
	std::cout << "please enter currency:\t";
	std::cin >> currency;
	string coinbase_account_id;
	std::cout << "please enter coinbase_account_id:\t";
	std::cin >> coinbase_account_id;
	std::cout << std::endl;
	api.Coinbase_Deposit(amount, currency, coinbase_account_id);
}
//28
void CBPR::Generate_Crypto_Deposit_Address()
{
	string coinbase_account_id;
	std::cout << "please enter coinbase_account_id:\t";
	std::cin >> coinbase_account_id;
	std::cout << std::endl;
	api.Generate_Crypto_Deposit_Address(coinbase_account_id);
}
//29
void CBPR::List_Withdrawals()
{
	api.List_Withdrawals();
}
//30
void CBPR::Single_Withdrawal()
{
	string transfer_id;
	std::cout << "please enter transfer_id:\t";
	std::cin >> transfer_id;
	std::cout << std::endl;
	api.Single_Withdrawal(transfer_id);
}
//31
void CBPR::Payment_Method_Withdraw()
{
	string amount;
	std::cout << "please enter amount:\t";
	std::cin >> amount;
	string currency;
	std::cout << "please enter currency:\t";
	std::cin >> currency;
	string payment_method_id;
	std::cout << "please enter payment_method_id:\t";
	std::cin >> payment_method_id;
	std::cout << std::endl;
	api.Payment_Method_Withdraw(amount, currency, payment_method_id);
}
//32
void CBPR::Coinbase_Withdraw()
{
	string amount;
	std::cout << "please enter amount:\t";
	std::cin >> amount;
	string currency;
	std::cout << "please enter currency:\t";
	std::cin >> currency;
	string coinbase_account_id;
	std::cout << "please enter coinbase_account_id:\t";
	std::cin >> coinbase_account_id;
	std::cout << std::endl;
	api.Coinbase_Withdraw(amount, currency, coinbase_account_id);
}
//33
void CBPR::Crypto_Withdraw()
{
	string amount;
	std::cout << "please enter amount:\t";
	std::cin >> amount;
	string currency;
	std::cout << "please enter currency:\t";
	std::cin >> currency;
	string crypto_address;
	std::cout << "please enter crypto_address:\t";
	std::cin >> crypto_address;
	std::cout << std::endl;
	api.Crypto_Withdraw(amount, currency, crypto_address);
}
//34
void CBPR::Fee_Estimate()
{
	string currency;
	std::cout << "please enter currency:\t";
	std::cin >> currency;
	string crypto_address;
	std::cout << "please enter crypto_address:\t";
	std::cin >> crypto_address;
	std::cout << std::endl;
	api.Fee_Estimate(currency, crypto_address);
}
//35
void CBPR::Create_Conversion()
{
	string from;
	std::cout << "please enter from:\t";
	std::cin >> from;
	string to;
	std::cout << "please enter to:\t";
	std::cin >> to;
	string amount;
	std::cout << "please enter amount:\t";
	std::cin >> amount;
	std::cout << std::endl;
	api.Create_Conversion(from, to, amount);
}
//36
void CBPR::Get_Current_Fees()
{
	api.Get_Current_Fees();
}
//37
void CBPR::Create_new_report()
{
	string type;
	std::cout << "please enter type (fills or account):\t";
	std::cin >> type;
	string start_date;
	std::cout << "please enter start_date:\t";
	std::cin >> start_date;
	string end_date;
	std::cout << "please enter end_date:\t";
	std::cin >> end_date;
	std::cout << std::endl;
	api.Create_new_report(type, start_date, end_date);
}
//38
void CBPR::Get_report_status()
{
	string report_id;
	std::cout << "please enter report_id:\t";
	std::cin >> report_id;
	api.Get_report_status(report_id);
}
//39
void CBPR::List_Profiles()
{
	api.List_Profiles();
}
//40
void CBPR::Get_Profile()
{
	string profile_id;
	std::cout << "please enter profile_id:\t";
	std::cin >> profile_id;
	api.Get_Profile(profile_id);
}
//41
void CBPR::Create_profile_transfer()
{
	string from;
	std::cout << "please enter from:\t";
	std::cin >> from;
	string to;
	std::cout << "please enter to:\t";
	std::cin >> to;
	string currency;
	std::cout << "please enter currency:\t";
	std::cin >> currency;
	string amount;
	std::cout << "please enter amount:\t";
	std::cin >> amount;
	std::cout << std::endl;
	api.Create_profile_transfer(from, to, currency, amount);
}

//42
void CBPR::Trailing_Volume()
{
	api.Trailing_Volume();
}




CBPR::CBPR()
{
	string line;
	std::ifstream myfile("key.conf");
	string api_key = "";
	string secret = "";
	string passcode = "";

	if (myfile.is_open())
	{
		while (std::getline(myfile, line))
		{
			std::vector<string> temp;
			tokenize(line, ':', temp);
			string key_name = temp[0];
			string key_value = temp[1];
			if (key_name == "api_key")
			{
				api_key = key_value;
			}
			if (key_name == "secret")
			{
				secret = key_value;
			}
			if (key_name == "passcode")
			{
				passcode = key_value;
			}
		}
		myfile.close();
	}
	else
	{
		std::cout << "Unable to open conf file";
	}
	string uri = "https://api-public.sandbox.pro.coinbase.com";
	string product_id = "BTC-USD";
	string currency = "BTC";

	Auth auth(api_key, secret, passcode);
	api.uri = uri;
	api.product_id = product_id;
	api.auth = auth;
}

CBPR::~CBPR()
{
}
