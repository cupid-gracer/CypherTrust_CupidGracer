#include "App.h"
#include "API.h"
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
#include <cstdlib>

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

struct ticker
{
	string baseCurrency, quoteCurrency, bid, ask;
};

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

			// float total = 0;
			// int prev = -1;
			// for (int k : cycle)
			// {
			// 	if (prev == -1)
			// 	{
			// 		prev = k;
			// 		continue;
			// 	}
			// 	for (int j = 0; j < E; j++)
			// 	{
			// 		if (graph->edge[j].src == prev && graph->edge[j].dest == k)
			// 			total += graph->edge[j].weight;
			// 	}
			// 	prev = k;
			// }
			// if((exp(-total) - 1) * 100  < 0) continue;
			all_cycles.push_back(cycle);
		}
	}
}

App::App()
{
	std::string line;
	std::ifstream myfile("key.conf");
	std::string url = "";
	std::string api_key = "";
	std::string secret_key = "";
	std::string str_currency = "";
	Util util;

	
	
	// "base64_string" is a null terminated string that is an encoding of the
	// binary data pointed to by "data"


	if (myfile.is_open())
	{
		while (std::getline(myfile, line))
		{
			std::vector<std::string> temp;
			util.split(line, ':', temp);
			std::string key_name = temp[0];
			std::string key_value = temp[1];
			if (key_name == "url")
			{
				url = key_value;
			}
			if (key_name == "api_key")
			{
				api_key = key_value;
			}
			if (key_name == "secret_key")
			{
				secret_key = key_value;
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
	std::string uri = "https://" + url;
	if (api_key == "" || secret_key == "")
	{
		cout << "api, secret key error!";
		return;
	}
	string str = api_key + ":" + secret_key;
	const char* data = str.c_str();
	int data_length = strlen(data);
	int encoded_data_length = util.Base64encode_len(data_length);
	char* base64_string = (char*) malloc(encoded_data_length);

	util.Base64encode(base64_string, data, data_length);

	api.uri = uri;
	api.token = base64_string;
	util.split(str_currency, ',', myCoinList);
}

App::~App()
{
}

//Public
// Available symbols
void App::Get_list_symbols()
{
	api.Get_list_symbols();
}

//2 Get symbol info
void App::Get_symbol_info()
{
	string symbol = "";
	cout << "symbol ( ETHBTC or BETBTC ):\t";
	cin >> symbol;
	api.Get_symbol_info(symbol);
}

// Available currencies
//parameter  ex: ETHBTC,BETBTC...
void App::Get_list_currencies()
{
	string currencies = "";
	cout << "currencies ( ETHBTC,BETBTC ):\t";
	cin >> currencies;
	api.Get_list_currencies(currencies);
}

// Get currency info
void App::Get_currency_info()
{
	string currency = "";
	cout << "currency ( ETH, BTC ):\t";
	cin >> currency;
	api.Get_currency_info(currency);
}

// Get Tickers
void App::Get_list_tickers()
{
	api.Get_list_tickers();
}

// Get Tickers
void App::Get_ticker()
{
	string symbol = "";
	cout << "symbol ( ETHBTC or BETBTC ):\t";
	cin >> symbol;
	api.Get_ticker(symbol);
}

// Get Trades
void App::Get_list_trades()
{
	string symbols = "";
	cout << "symbols ( ETHBTC,BETBTC ):\t";
	cin >> symbols;
	api.Get_list_trades(symbols);
}


//Get Trade
void App::Get_trade()
{
	string symbol = "";
	cout << "symbol ( ETHBTC or BETBTC ):\t";
	cin >> symbol;
	api.Get_list_trades(symbol);
}

//Get list order
void App::Get_list_orderbook()
{
	api.Get_list_orderbook();
}

//Get order
void App::Get_orderbook()
{
	string symbol = "";
	cout << "symbol ( ETHBTC or BETBTC ):\t";
	cin >> symbol;
	api.Get_orderbook(symbol);
}

//Get Candles
void App::Get_list_candles()
{
	string symbols = "";
	cout << "symbols ( ETHBTC,BETBTC ):\t";
	cin >> symbols;
	api.Get_list_candles(symbols);
}

//Get Candles
void App::Get_candle()
{
	string symbol = "";
	cout << "symbol ( ETHBTC,BETBTC ):\t";
	cin >> symbol;
	api.Get_candle(symbol);
}

//Get Candles
void App::Get_list_orders()
{
	api.Get_list_orders();
}

void App::Get_arbitrage_opportunity()
{
	all_cycles.clear();
	bool isUse = true;
	string str;
	cout << "Will you use all currency pairs, or your currency pairs in conf file? [Y/n]\t";
	cin >> str;
	transform(str.begin(), str.end(), str.begin(), ::tolower);
	if (str == "y")
	{
		isUse = false;
	}
	else if (str == "n")
	{
		isUse = true;
	}
	else
		return;


	if(isUse && myCoinList.size() < 1) 
	{
		cout<< "No Coin in conf file!" << endl;
		return;
	} 
				// auto start = std::chrono::system_clock::now();
	string st = api.Get_list_tickers();
	string symbols = api.Get_list_symbols();
				// auto end = std::chrono::system_clock::now();

				// std::chrono::duration<double> elapsed_seconds = end - start;
				// std::time_t end_time = std::chrono::system_clock::to_time_t(end);

				// std::cout << " API time: " << elapsed_seconds.count() << "s\n";

	// cout << st;
	Document d;
	Document d_symbols;
	d.Parse(st.c_str());
	d_symbols.Parse(symbols.c_str());
	
	// Number of vertices in graph
	int V;
	// Number of edges in graph
	int E = 0;

	assert(d.IsArray());
	assert(d_symbols.IsArray());
	const Value &c = d;
	const Value &c_symbols = d_symbols;

				// start = std::chrono::system_clock::now();
	vector<ticker> tickers (c.Size());
	for (int i = 0; i < c.Size(); i++)
	{
		string bid = c[i]["bid"].GetString();
		string ask = c[i]["ask"].GetString();
		string symbol = c[i]["symbol"].GetString();
		tickers[i].bid = bid;
		tickers[i].ask = ask;
		string ccode = "";
		string mcode = "";
		bool f = false;
		for(int j = 0; j < c_symbols.Size(); j++){
			if(c_symbols[j]["id"].GetString() == symbol){
				tickers[i].baseCurrency = 	c_symbols[j]["baseCurrency"].GetString();
				tickers[i].quoteCurrency = c_symbols[j]["quoteCurrency"].GetString();
				ccode = c_symbols[j]["baseCurrency"].GetString();
				mcode = c_symbols[j]["quoteCurrency"].GetString();
				f =  true;
				break;
			}
		} 
		if(!f) break;
								
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

				// end = std::chrono::system_clock::now();
				// elapsed_seconds = end - start;
				// end_time = std::chrono::system_clock::to_time_t(end);
				// cout << " data initial time: " << elapsed_seconds.count() << "s\n";	


				// start = std::chrono::system_clock::now();

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

	for (int i = 0, j = 0; i < tickers.size(); i++)
	{
		string bid = tickers[i].bid;
		string ask = tickers[i].ask;
		if (bid == "" || ask == "")
			continue;
		if (stof(bid) <= 0 || stof(ask) <= 0)
			continue;

		string ccode = tickers[i].baseCurrency;
		string mcode = tickers[i].quoteCurrency;
		int u = searchIndex(ccode);
		int v = searchIndex(mcode);
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



				// end = std::chrono::system_clock::now();

				// elapsed_seconds = end - start;
				// end_time = std::chrono::system_clock::to_time_t(end);

				// std::cout << " finished time: " << elapsed_seconds.count() << "s\n";	
}

