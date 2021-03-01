/*  Arbitrage Cycle Class   */
// AC.h
#ifndef AC_H
#define AC_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <math.h>
#include <sstream>
#include <fstream>
#include <algorithm>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;
using namespace std;

class AC
{


private:
    vector<string> coinList;
    vector<string> myCoinList;
    vector<vector<int>> all_cycles;
    bool isUse;

public:
    AC(bool isUse, vector<string> specific_coinList)
    {
        this->isUse = isUse;
        this->myCoinList = specific_coinList;
    }
    ~AC();
    vector<vector<string>> arbitrage_cycles;

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

                reverse(cycle.begin(), cycle.end());
                all_cycles.push_back(cycle);
            }
        }
    }

    /*
    parameters
    isUse(bool):        if true, will use all symbols, if false, will use specific symbols which be came from CypherTrust REST API
    data(Document):  
                        [
                            {
                                "c_code": "BTC",
                                "m_code": "ETH",
                                "bid"   : "0.0041"
                                "ask"   : "0.0040"
                            }...
                        ]
    */
    void Get_arbitrage_opportunity(Document data)
    {
        // clear all values
        all_cycles.clear();
        arbitrage_cycles.clear();
        coinList.clear();
        myCoinList.clear();

        if (isUse && myCoinList.size() < 1)
        {
            cout << "No Coin in conf file!" << endl;
            return;
        }

        // Number of vertices in graph
        int V;
        // Number of edges in graph
        int E = 0;

        assert(data.IsArray());
        const Value &tickers = data;

        // set coin list
        for (int i = 0; i < tickers.Size(); i++)
        {
            string bid = tickers[i]["bid"].GetString();
            string ask = tickers[i]["ask"].GetString();
            string ccode = tickers[i]["c_code"].GetString();
            string mcode = tickers[i]["m_code"].GetString();
            if (bid != "" && ask != "")
                if (stof(bid) > 0 && stof(ask) > 0)
                {
                    if (isUse)
                    {
                        auto ic = find(myCoinList.begin(), myCoinList.end(), ccode) - myCoinList.begin();
                        auto im = find(myCoinList.begin(), myCoinList.end(), mcode) - myCoinList.begin();
                        if (ic != myCoinList.size() && im != myCoinList.size())
                        { // if ic, im are in my coin list, insert to coinList
                            insertCoin(ccode);
                            insertCoin(mcode);
                            E++;
                        }
                        else
                            continue;
                    }
                    else
                    {
                        insertCoin(ccode);
                        insertCoin(mcode);
                        E++;
                    }
                }
        }

        E = 2 * E;
        V = coinList.size();

        string table[V][V];
        for (int i = 0; i < V; i++)
        {
            for (int j = 0; j < V; j++)
                table[i][j] = "";
        }
        struct Graph *graph = createGraph(V, E);

        for (int i = 0, j = 0; i < tickers.Size(); i++)
        {
            string bid = tickers[i]["bid"].GetString();
            string ask = tickers[i]["ask"].GetString();

            int u = searchIndex(tickers[i]["c_code"].GetString());
            int v = searchIndex(tickers[i]["m_code"].GetString());
            if (u == coinList.size() || v == coinList.size()) // check whether there isn't the c_code or m_code in coin list
                continue;
            float weight1 = -log(stof(bid));
            float weight2 = -log(1 / stof(ask));

            stringstream stream;
            stream << fixed << (1 / stof(ask));
            string s_ask = stream.str();

            table[u][v] = s_ask;
            table[v][u] = bid;
            // stringstream stream1;
            // stream1 << fixed << weight2;
            // s_ask = stream1.str();
            // table[u][v] = s_ask;
            // stringstream stream2;
            // stream2 << fixed << weight1;
            // s_ask = stream2.str();
            // table[v][u] = s_ask;

            graph->edge[j].src = u;
            graph->edge[j].dest = v;
            graph->edge[j].weight = weight2;
            graph->edge[j + 1].src = v;
            graph->edge[j + 1].dest = u;
            graph->edge[j + 1].weight = weight1;
            j += 2;
        }

        // write csv file
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
        vector<vector<string>> result;
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
                vector<string> temp_str;
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
                    temp_str.push_back(coinList.at(k));

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
                stringstream stream;
                stream << fixed << (exp(-total) - 1) * 100;
                string total_str = stream.str();
                temp_str.push_back(total_str + "%");
                result.push_back(total_str);
                cout << "]  \033[1;33m" << (exp(-total) - 1) * 100 << "% \033[0m" << endl;
            }
        }
        if (result.size() == 0)
            cout << "\033[1;33mNo arbitrage opportunity!\033[0m" << endl;
        else{
                arbitrage_cycles = result;
            }
    }
}

#endif // AC_H
