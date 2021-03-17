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
    AC(vector<string> specific_coinList);
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
    void Get_arbitrage_opportunity(Document data);
};

#endif // AC_H
