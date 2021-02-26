// Util.h
#ifndef UTIL_H
#define UTIL_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <algorithm>

class Util
{
public:
    Util();
    ~Util();

    void permutation(std::string a, int l, int r);
    std::string GetTimestamp();
    int GetNumberCombination(int n, int r);
    void split(std::string &str, char delim, std::vector<std::string> &out);
};

#endif // UTIL_H
