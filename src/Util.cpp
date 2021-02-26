
#include "Util.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <algorithm>

// using namespace rapidjson;

std::string Util::GetTimestamp()
{
    time_t t = time(0);
    return std::to_string(t);
}

// Function to print permutations of string 
// This function takes three parameters: 
// 1. String 
// 2. Starting index of the string 
// 3. Ending index of the string. 
void Util::permutation(std::string a, int l, int r) 
{ 
    // Base case 
    if (l == r)
        std::cout<<a<<std::endl;
    else
    {
        // Permutations made 
        for (int i = l; i <= r; i++)
        {

            // Swapping done
            std::swap(a[l], a[i]);

            // Recursion called
            permutation(a, l+1, r);

            //backtrack
            std::swap(a[l], a[i]);
        }
    }
}


void Util::split(std::string &str, char delim, std::vector<std::string> &out)
{
	size_t start;
	size_t end = 0;

	while ((start = str.find_first_not_of(delim, end)) != std::string::npos)
	{
		end = str.find(delim, start);
		out.push_back(str.substr(start, end - start));
	}
}


Util::Util()
{
}

Util::~Util()
{
}
