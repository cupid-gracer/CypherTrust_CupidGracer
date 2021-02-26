#include "exchange/STEX/STEX.h"
#include "exchange/STEX/API.h"
#include "Util.h"

#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

// #include "Util.cpp"

using namespace rapidjson;
using namespace std;

vector<string> coinList;
vector<string> myCoinList;
vector<vector<int>> all_cycles;
// C++ program for the above approach
//#include <bits/stdc++.h>

// Structure to represent a weighted
// edge in graph
struct Edge
{
	int src, dest;
	float weight;
};

// Structure to represent a directed
// and weighted graph
struct Graph
{

	// V -> Number of vertices,
	// E -> Number of edges
	int V, E;

	// Graph is represented as an
	// array of edges
	struct Edge *edge;
};

// Creates a new graph with V vertices
// and E edges
struct Graph *createGraph(int V, int E)
{
	struct Graph *graph = new Graph;
	graph->V = V;
	graph->E = E;
	graph->edge = new Edge[graph->E];
	return graph;
}

void insertCoin(string coin)
{
	auto it = find(coinList.begin(), coinList.end(), coin);
	if (it == coinList.end())
	{
		coinList.push_back(coin);
	}
}

int searchIndex(string coin)
{
	return find(coinList.begin(), coinList.end(), coin) - coinList.begin();
}

// Function runs Bellman-Ford algorithm
// and prints negative cycle(if present)
void NegCycleBellmanFord(struct Graph *graph, int src)
{
	int V = graph->V;
	int E = graph->E;
	// and all parent as -1
	vector<int> parent(V, -1);

	// Initialize distances from src
	// to all other vertices as INFINITE
	vector<float> dist(V, numeric_limits<float>::max());
	dist[src] = 0;

	// Relax all edges |V| - 1 times.
	for (int i = 1; i <= V - 1; i++)
	{
		for (int j = 0; j < E; j++)
		{

			int u = graph->edge[j].src;
			int v = graph->edge[j].dest;
			float weight = graph->edge[j].weight;

			if (dist[u] != numeric_limits<float>::max() && dist[u] + weight < dist[v])
			{

				dist[v] = dist[u] + weight;
				parent[v] = u;
			}
		}
	}

	// Check for negative-weight cycles
	int C = -1;
	vector<bool> seen(V, false);

	for (int i = 0; i < E; i++)
	{

		int u = graph->edge[i].src;
		int v = graph->edge[i].dest;
		float weight = graph->edge[i].weight;

		if (dist[u] != numeric_limits<float>::max() && dist[u] + weight < dist[v])
		{

			// Store one of the vertex of
			// the negative weight cycle
			C = v;

			if (seen[C] == true)
			{
				continue;
			}

			for (int j = 0; j < V; j++)
			{
				C = parent[C];
			}

			// To store the cycle vertex
			vector<int> cycle;
			for (int j = C;; j = parent[j])
			{

				seen[j] = true;
				cycle.push_back(j);
				if (j == C && cycle.size() > 1)
					break;
			}

			// Reverse cycle[]
			reverse(cycle.begin(), cycle.end());
			all_cycles.push_back(cycle);
		}
	}

}

STEX::STEX()
{
	std::string line;
	std::ifstream myfile("key.conf");
	std::string access_token = "";
	std::string str_currency = "";
	Util util;

	if (myfile.is_open())
	{
		while (std::getline(myfile, line))
		{
			std::vector<std::string> temp;
			util.split(line, ':', temp);
			std::string key_name = temp[0];
			std::string key_value = temp[1];
			if (key_name == "access_token")
			{
				access_token = key_value;
			}
			if (key_name == "currency")
			{
				str_currency = key_value;
			}
		}
		myfile.close();
	}
	else
	{
		std::cout << "Unable to open conf file";
	}
	std::string uri = "https://api3.stex.com";

	api.uri = uri;
	api.access_token = access_token;
	util.split(str_currency, ',', myCoinList);
}

STEX::~STEX()
{
}

//Public
// Available Currencies
void STEX::Get_list_currencies()
{
	api.Get_list_currencies();
}

// Get currency info
void STEX::Get_currency_info()
{
	std::string currencyId;
	std::cout << "please enter currencyId:\t";
	std::cin >> currencyId;
	std::cout << std::endl;
	api.Get_currency_info(currencyId);
}

// Available markets
void STEX::Get_list_markets()
{
	api.Get_list_markets();
}

// Get list of all avialable currency pairs groups
void STEX::Get_list_currency_pairs_groups()
{
	api.Get_list_currency_pairs_groups();
}

// Available currency pairs
void STEX::Get_currency_pairs()
{
	std::string code;
	std::cout << "please enter code (Available values : ALL, marketSymbol):\t";
	std::cin >> code;
	std::cout << std::endl;
	api.Get_currency_pairs(code);
}

// Available currency pairs for a given group
void STEX::Get_currency_pairs_given_group()
{
	std::string currencyPairGroupId;
	std::cout << "please enter currencyPairGroupId:\t";
	std::cin >> currencyPairGroupId;
	std::cout << std::endl;
	api.Get_currency_pairs_given_group(currencyPairGroupId);
}

// Get currency pair information
void STEX::Get_currency_pair_information()
{
	std::string currencyPairId;
	std::cout << "please enter currencyPairId:\t";
	std::cin >> currencyPairId;
	std::cout << std::endl;
	api.Get_currency_pair_information(currencyPairId);
}

// Tickers list for all currency pairs
void STEX::Get_ticker_list_currency_pairs(bool isUse)
{
	all_cycles.clear();
	if(isUse && myCoinList.size() < 1) 
	{
		cout<< "No Coin in conf file!" << endl;
		return;
	} 
	string st = api.Get_ticker_list_currency_pairs();

	// cout << st;
	Document d;
	d.Parse(st.c_str());

	// Number of vertices in graph
	int V;
	// Number of edges in graph
	int E = 0;

	assert(d["data"].IsArray());
	const Value &c = d["data"];

				
	for (int i = 0; i < c.Size(); i++)
	{
		string bid = c[i]["bid"].GetString();
		string ask = c[i]["ask"].GetString();
		string ccode = c[i]["currency_code"].GetString();
		string mcode = c[i]["market_code"].GetString();
		if (bid != "" && ask != "")
			if (stof(bid) > 0 && stof(ask) > 0)
			{
				if(isUse){
					auto ic = find(myCoinList.begin(), myCoinList.end(), ccode) - myCoinList.begin();
					auto im = find(myCoinList.begin(), myCoinList.end(), mcode) - myCoinList.begin();
					if(ic != myCoinList.size() && im != myCoinList.size() ){	// if ic, im are in my coin list, insert to coinList
						insertCoin(ccode);
						insertCoin(mcode);
						E++;
					}else continue;
				}else{
					insertCoin(ccode);
					insertCoin(mcode);
					E++;
				}
			}
	}






	E = 2 * E;
	V = coinList.size();

	// cout << V << endl;
	// cout << E << endl;
	string table[V][V];
	for (int i = 0; i < V; i++)
	{
		// cout << i << "  " << coinList.at(i) << endl;
		for (int j = 0; j < V; j++)
			table[i][j] = "";
	}
	struct Graph *graph = createGraph(V, E);

	for (int i = 0, j = 0; i < c.Size(); i++)
	{
		string bid = c[i]["bid"].GetString();
		string ask = c[i]["ask"].GetString();
		if (bid == "" || ask == "")
			continue;
		if (stof(bid) <= 0 || stof(ask) <= 0)
			continue;

		int u = searchIndex(c[i]["currency_code"].GetString());
		int v = searchIndex(c[i]["market_code"].GetString());
		if(u == coinList.size() || v == coinList.size()) continue;
		// cout << i << " " << c[i]["currency_code"].GetString() << "-" << c[i]["market_code"].GetString() << "  " << bid << endl;
		float weight1 = -log(stof(bid));
		float weight2 = -log(1 / stof(ask));

		stringstream stream;
		stream << std::fixed << (1 / stof(ask));
		std::string s_ask = stream.str();

		table[u][v] = s_ask;
		table[v][u] = bid;
		// stringstream stream1;
		// stream1 << std::fixed << weight2;
		// s_ask = stream1.str();
		// table[u][v] = s_ask;
		// stringstream stream2;
		// stream2 << std::fixed << weight1;
		// s_ask = stream2.str();
		// table[v][u] = s_ask;

		graph->edge[j].src = u;
		graph->edge[j].dest = v;
		graph->edge[j].weight = weight2;
		graph->edge[j + 1].src = v;
		graph->edge[j + 1].dest = u;
		graph->edge[j + 1].weight = weight1;
		j += 2;
		// std::cout << c[i / 2]["currency_code"].GetString()<< "-" << c[i / 2]["market_code"].GetString()<<" :\t"<<weight1 << endl;
		// std::cout << c[i / 2]["market_code"].GetString()<< "-" << c[i / 2]["currency_code"].GetString()<<" :\t"<<weight2 << endl;
	}
	ofstream myfile;
	myfile.open("test.csv");
	bool f = true;
	myfile << ",";
	for (int i = 0; i < V; i++)
	{
		myfile << coinList.at(i) << ",";
	}
	myfile << endl;
	for (int i = 0; i < V; i++)
	{
		if (f)
		{
			f = false;
			myfile << coinList.at(i) << ",";
		}
		for (int j = 0; j < V; j++)
		{
			myfile << table[i][j] << ",";
		}
		f = true;
		myfile << endl;
		NegCycleBellmanFord(graph, i);
	}
				

	vector<vector<int>> temp;
	for (int i = 0; i < all_cycles.size(); i++)
	{
		bool f = true;
		for (int j = 0; j < temp.size(); j++)
		{
			if (temp[j] == all_cycles[i])
			{
				f = false;
			}
		}
		if (f)
		{
			temp.push_back(all_cycles[i]);
			vector<int> cycle = all_cycles[i];
			// Printing the negative cycle
			// Get the total weight
			float total = 0;
			int prev = -1;
			cout << "\033[1;31m PATH: \033[0m [ ";
			for (int k : cycle)
			{
				cout << coinList.at(k) << ' ';
				if (prev == -1)
				{
					prev = k;
					continue;
				}
				for (int j = 0; j < E; j++)
				{
					if (graph->edge[j].src == prev && graph->edge[j].dest == k)
						total += graph->edge[j].weight;
				}
				prev = k;
			}

			cout << "]  \033[1;33m" << (exp(-total) - 1) * 100 << "% \033[0m" << endl;
		}
	}
	if(temp.size() == 0) cout << "\033[1;33mNo arbitrage opportunity!\033[0m" << endl;
				
}

// Ticker for currency pair
void STEX::Get_ticker_currency_pair()
{
	std::string currencyPairId;
	std::cout << "please enter currencyPairId:\t";
	std::cin >> currencyPairId;
	std::cout << std::endl;
	api.Get_ticker_currency_pair(currencyPairId);
}

// Trades for given currency pair
void STEX::Get_trades_currency_pair()
{
	std::string currencyPairId;
	std::cout << "please enter currencyPairId:\t";
	std::cin >> currencyPairId;
	std::cout << std::endl;

	api.Get_trades_currency_pair(currencyPairId);
}

// Orderbook for given currency pair
void STEX::Get_orderbook_currency_pair()
{
	std::string currencyPairId;
	std::cout << "please enter currencyPairId:\t";
	std::cin >> currencyPairId;
	std::cout << std::endl;

	api.Get_orderbook_currency_pair(currencyPairId);
}

// A list of candles for given currency pair
void STEX::Get_list_candles_currency_pair()
{
	std::string currencyPairId;
	std::cout << "please enter currencyPairId:\t";
	std::cin >> currencyPairId;
	std::cout << std::endl;

	std::string candlesType;
	std::cout << "please enter candlesType (1, 5, 30, 60, 240, 720, 1D):\t";
	std::cin >> candlesType;
	std::cout << std::endl;

	std::string timeStart;
	std::cout << "please enter timeStart (e.g  1609459200):\t";
	std::cin >> timeStart;
	std::cout << std::endl;

	std::string timeEnd;
	std::cout << "please enter timeEnd (e.g  1612354600):\t";
	std::cin >> timeEnd;
	std::cout << std::endl;

	api.Get_list_candles_currency_pair(currencyPairId, candlesType, timeStart, timeEnd);
}

// Available Deposit Statuses
void STEX::Get_list_deposit_statuses()
{
	api.Get_list_deposit_statuses();
}

// Get deposit status info
void STEX::Get_deposit_status()
{
	std::string statusId;
	std::cout << "please enter statusId:\t";
	std::cin >> statusId;
	std::cout << std::endl;
	api.Get_orderbook_currency_pair(statusId);
}

// Available Withdrawal Statuses
void STEX::Get_list_withdrawal_statuses()
{
	api.Get_list_withdrawal_statuses();
}

// Get status info
void STEX::Get_withdrawal_status()
{
	std::string statusId;
	std::cout << "please enter statusId:\t";
	std::cin >> statusId;
	std::cout << std::endl;
	api.Get_orderbook_currency_pair(statusId);
}

// Test API is working and get server time
void STEX::ping()
{
	api.ping();
}

// Shows the official mobile applications data
void STEX::Get_mobile_version()
{
	api.Get_mobile_version();
}

// Get the last 20 posts (stex.com) on Twitter
void STEX::Get_twitter()
{
	api.Get_twitter();
}

//Trading
//Returns the user's fees for a given currency pair
void STEX::Get_user_fee_currency_pair()
{
	std::string currencyPairId;
	std::cout << "please enter currencyPairId (1, 2, 3..):\t";
	std::cin >> currencyPairId;
	std::cout << std::endl;
	api.Get_user_fee_currency_pair(currencyPairId);
}

//List your currently open orders
void STEX::Get_list_open_all_orders()
{
	api.Get_list_open_all_orders();
}

//Delete all active orders
void STEX::Delete_all_active_orders()
{
	api.Delete_all_active_orders();
}

//List your currently open orders for given currency pair
void STEX::Get_list_open_order_by_currency_pair()
{
	std::string currencyPairId;
	std::cout << "please enter currencyPairId (1, 2, 3..):\t";
	std::cin >> currencyPairId;
	std::cout << std::endl;
	api.Get_list_open_order_by_currency_pair(currencyPairId);
}

//Delete active orders for given currency pair
void STEX::Delete_order_by_currency_pair()
{
	std::string currencyPairId;
	std::cout << "please enter currencyPairId (1, 2, 3..):\t";
	std::cin >> currencyPairId;
	std::cout << std::endl;
	api.Get_list_open_order_by_currency_pair(currencyPairId);
}

//Create new order and put it to the orders processing queue
void STEX::Creat_new_order()
{
	std::string currencyPairId;
	std::cout << "please enter currencyPairId (1, 2, 3..):\t";
	std::cin >> currencyPairId;
	std::cout << std::endl;

	std::string type;
	std::cout << "please enter type (BUY / SELL / STOP_LIMIT_BUY / STOP_LIMIT_SELL):\t";
	std::cin >> type;
	std::cout << std::endl;

	std::string amount;
	std::cout << "please enter amount :\t";
	std::cin >> amount;
	std::cout << std::endl;

	std::string price;
	std::cout << "please enter price :\t";
	std::cin >> price;
	std::cout << std::endl;

	std::string trigger_price;
	std::cout << "please enter trigger_price :\t";
	std::cin >> trigger_price;
	std::cout << std::endl;

	api.Creat_new_order(currencyPairId, type, amount, price, trigger_price);
}

//Get a single order
void STEX::Get_single_order()
{
	std::string orderId;
	std::cout << "please enter orderId:\t";
	std::cin >> orderId;
	std::cout << std::endl;
	api.Get_list_open_order_by_currency_pair(orderId);
}

//Cancel order
void STEX::Delete_order()
{
	std::string orderId;
	std::cout << "please enter orderId:\t";
	std::cin >> orderId;
	std::cout << std::endl;
	api.Delete_order(orderId);
}

//Get a list of currencies user had any activity in
void STEX::Get_list_currency_user_activity()
{
	std::string key;
	std::cout << "please enter key ( Deposits, Withdrawals, Burn, Reward, Investments ):\t";
	std::cin >> key;
	std::cout << std::endl;
	api.Get_list_currency_user_activity(key);
}

//Gets the list of currency pairs the user had orders in for all the time
void STEX::Get_list_all_currencypairs_by_user()
{
	api.Get_list_all_currencypairs_by_user();
}

//Get past orders
void STEX::Get_past_orders()
{
	api.Get_past_orders();
}

//Get specified order details
void STEX::Get_order_details()
{
	std::string orderId;
	std::cout << "please enter orderId :\t";
	std::cin >> orderId;
	std::cout << std::endl;
	api.Get_order_details(orderId);
}

//Get a list of user trades according to request parameters
void STEX::Get_list_user_spec_trades()
{
	std::string currencyPairId;
	std::cout << "please enter currencyPairId :\t";
	std::cin >> currencyPairId;
	std::cout << std::endl;
	api.Get_list_user_spec_trades(currencyPairId);
}

//Get reports list for category
void STEX::Get_reports_list_category()
{
	std::string listMode;
	std::cout << "please enter listMode ( all, recently, scheduled ):\t";
	std::cin >> listMode;
	std::cout << std::endl;
	api.Get_reports_list_category(listMode);
}

//Get some report info
void STEX::Get_report_info()
{
	std::string id;
	std::cout << "please enter id :\t";
	std::cin >> id;
	std::cout << std::endl;
	api.Get_report_info(id);
}

//Remove report by id
void STEX::Delete_report_by_id()
{
	std::string id;
	std::cout << "please enter id :\t";
	std::cin >> id;
	std::cout << std::endl;
	api.Delete_report_by_id(id);
}

//Create new report
void STEX::Create_new_report()
{
	std::string name;
	std::cout << "please enter name :\t";
	std::cin >> name;
	std::cout << std::endl;
	std::string date_from;
	std::cout << "please enter date_from :\t";
	std::cin >> date_from;
	std::cout << std::endl;
	std::string date_to;
	std::cout << "please enter date_to :\t";
	std::cin >> date_to;
	std::cout << std::endl;
	std::string format;
	std::cout << "please enter format  ( Html, Csv, Xls, Pdf ) :\t";
	std::cin >> format;
	std::cout << std::endl;
	std::string type;
	std::cout << "please enter type array 	['All', 'Deposits', 'Withdrawals', 'Orders'] :\t";
	std::cin >> type;
	std::cout << std::endl;
	api.Create_new_report(name, date_from, date_to, format, type);
}

//Get file by id
void STEX::Get_file_by_id()
{
	std::string id;
	std::cout << "please enter id :\t";
	std::cin >> id;
	std::cout << std::endl;
	api.Delete_report_by_id(id);
}

//Settings
//User event notification settings
void STEX::Get_list_notification_by_event()
{
	std::string event;
	std::cout << "please enter event (An event name you want to subscribe.) :\t";
	std::cin >> event;
	std::cout << std::endl;
	api.Get_list_notification_by_event(event);
}

//User event notification settings
void STEX::Get_list_notification()
{
	api.Get_list_notification();
}

//Set notification settings
void STEX::Set_notification_settings()
{
	std::string event;
	std::cout << "please enter event (An event name you want to subscribe.) :\t";
	std::cin >> event;
	std::cout << std::endl;
	std::string channel;
	std::cout << "please enter channel (A channel name you want to receive the notification through.) :\t";
	std::cin >> channel;
	std::cout << std::endl;
	std::string value;
	std::cout << "please enter value (1, 0) :\t";
	std::cin >> value;
	std::cout << std::endl;

	api.Set_notification_settings(event, channel, value);
}

//Set notification settings
void STEX::Set_notification_settings_one_request()
{
	api.Set_notification_settings_one_request();
}

//Profile
//Account Information
void STEX::Get_accoutn_information()
{
	api.Get_accoutn_information();
}

//Get a list of user wallets
void STEX::Get_list_user_wallets()
{
	api.Get_list_user_wallets();
}

//Single wallet information
void STEX::Get_single_user_wallet()
{
	std::string walletId;
	std::cout << "please enter walletId :\t";
	std::cin >> walletId;
	std::cout << std::endl;
	api.Get_single_user_wallet(walletId);
}

//Burns the given wallet
void STEX::Burn_wallet()
{
	std::string walletId;
	std::cout << "please enter walletId :\t";
	std::cin >> walletId;
	std::cout << std::endl;
	api.Burn_wallet(walletId);
}

//Create a wallet for given currency
void STEX::Create_wallet()
{
	std::string currencyId;
	std::cout << "please enter currencyId :\t";
	std::cin >> currencyId;
	std::cout << std::endl;
	api.Create_wallet(currencyId);
}

//Get deposit address for given wallet
void STEX::Get_deposit_address_wallet()
{
	std::string walletId;
	std::cout << "please enter walletId :\t";
	std::cin >> walletId;
	std::cout << std::endl;
	api.Get_deposit_address_wallet(walletId);
}

//Create new deposit address
void STEX::Create_new_deposit_address()
{
	std::string walletId;
	std::cout << "please enter walletId :\t";
	std::cin >> walletId;
	std::cout << std::endl;
	api.Create_new_deposit_address(walletId);
}

//Get a list of deposits made by user
void STEX::Get_list_user_deposit()
{
	api.Get_list_user_deposit();
}

//Get deposit by id
void STEX::Get_deposit()
{
	std::string id;
	std::cout << "please enter id :\t";
	std::cin >> id;
	std::cout << std::endl;
	api.Get_deposit(id);
}

//Get a list of rewards obtained by user (e.g. in trading competitions)
void STEX::Get_list_rewards()
{
	api.Get_list_rewards();
}

//Get reward by id
void STEX::Get_reward()
{
	std::string id;
	std::cout << "please enter id :\t";
	std::cin >> id;
	std::cout << std::endl;
	api.Get_reward(id);
}

//Get a list of user address book items
void STEX::Get_list_user_address()
{
	api.Get_list_user_address();
}

//Single address book item
void STEX::Get_single_address()
{
	std::string itemId;
	std::cout << "please enter itemId :\t";
	std::cin >> itemId;
	std::cout << std::endl;
	api.Get_single_address(itemId);
}

//Deletes address book item
void STEX::Delete_address_book()
{
	std::string itemId;
	std::cout << "please enter itemId :\t";
	std::cin >> itemId;
	std::cout << std::endl;
	api.Delete_address_book(itemId);
}

//Disables the address book item
void STEX::Disable_address_book_item()
{
	std::string itemId;
	std::cout << "please enter itemId :\t";
	std::cin >> itemId;
	std::cout << std::endl;
	api.Disable_address_book_item(itemId);
}

//Enable the address book item
void STEX::Enable_address_book_item()
{
	std::string itemId;
	std::cout << "please enter itemId :\t";
	std::cin >> itemId;
	std::cout << std::endl;
	api.Enable_address_book_item(itemId);
}

//Restrict the withdrawals to only addresses that are active in addressbook
void STEX::Restrict_withdrawal_addressbook()
{
	api.Restrict_withdrawal_addressbook();
}

//Remove restriction to withdraw to only addresses that are active in addressbook. E.g. allow to withdraw to any address.
void STEX::Allow_withdrawal_addressbook()
{
	api.Allow_withdrawal_addressbook();
}

//Get a list of withdrawals made by user
void STEX::Get_list_withdrawal()
{
	api.Get_list_withdrawal();
}

//Get withdrawal by id
void STEX::Get_withdrawal()
{
	std::string id;
	std::cout << "please enter id :\t";
	std::cin >> id;
	std::cout << std::endl;
	api.Get_withdrawal(id);
}

//Create withdrawal request
void STEX::Create_withdrawal_request()
{
	std::string currencyId;
	std::cout << "please enter currencyId :\t";
	std::cin >> currencyId;
	std::cout << std::endl;
	std::string amount;
	std::cout << "please enter amount :\t";
	std::cin >> amount;
	std::cout << std::endl;
	std::string address;
	std::cout << "please enter address :\t";
	std::cin >> address;
	std::cout << std::endl;
	api.Create_withdrawal_request(currencyId, amount, address);
}

//Cancel unconfirmed withdrawal
void STEX::Cancel_unconfirmed_withdrawal()
{
	std::string withdrawalId;
	std::cout << "please enter withdrawalId :\t";
	std::cin >> withdrawalId;
	std::cout << std::endl;
	api.Cancel_unconfirmed_withdrawal(withdrawalId);
}

//Get notifications
void STEX::Get_notifications()
{
	api.Get_notifications();
}

//Get a list of active price alerts
void STEX::Get_list_active_price_alert()
{
	api.Get_list_active_price_alert();
}

//Create new price alert
void STEX::Create_new_price_alert()
{
	std::string currencyPairId;
	std::cout << "please enter currencyPairId :\t";
	std::cin >> currencyPairId;
	std::cout << std::endl;
	std::string comarison;
	std::cout << "please enter comarison ( 'GREATER' or 'LESS') :\t";
	std::cin >> comarison;
	std::cout << std::endl;
	std::string price;
	std::cout << "please enter price :\t";
	std::cin >> price;
	std::cout << std::endl;
	api.Create_new_price_alert(currencyPairId, comarison, price);
}

//Delete the price alert by ID
void STEX::Delete_price_alert()
{
	std::string priceAlertId;
	std::cout << "please enter priceAlertId :\t";
	std::cin >> priceAlertId;
	std::cout << std::endl;
	api.Delete_price_alert(priceAlertId);
}

//Create referral program
void STEX::Create_referral_program()
{
	api.Create_referral_program();
}

//Insert referral code
void STEX::Insert_referral_code()
{
	std::string code;
	std::cout << "please enter code :\t";
	std::cin >> code;
	std::cout << std::endl;
	api.Delete_price_alert(code);
}

//Transfer referral bonuses balance to main balance for given currency
void STEX::Transfer_referral_bonuses()
{
	std::string currencyId;
	std::cout << "please enter currencyId :\t";
	std::cin >> currencyId;
	std::cout << std::endl;
	api.Transfer_referral_bonuses(currencyId);
}

//Get favorite currency pairs
void STEX::Get_fav_currency_pair()
{
	api.Get_fav_currency_pair();
}

//Set favorite currency pairs
/*
    addPairIds  array[integer](query)	add ids of currency pairs to list
    removePairIds   array[integer](query)	remove ids of currency pairs from list
    show        boolean(query)
    */
void STEX::Set_fav_currency_pair()
{
	std::string addPairIds;
	std::cout << "please enter addPairIds(1,2,3...) :\t";
	std::cin >> addPairIds;
	std::cout << std::endl;
	api.Set_fav_currency_pair(addPairIds);
}

//Get current token scopes
void STEX::Get_current_token_scopes()
{
	api.Get_current_token_scopes();
}
