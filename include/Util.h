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

using namespace std;

class Util
{
public:
    Util();
    ~Util();

    void permutation(std::string a, int l, int r);
    long GetNowTimestamp();
    int GetNumberCombination(int n, int r);
    void split(std::string &str, char delim, std::vector<std::string> &out);

    int Base64encode_len(int len);
    int Base64encode(char * coded_dst, const char *plain_src,int len_plain_src);

    int Base64decode_len(const char * coded_src);
    int Base64decode(char * plain_dst, const char *coded_src);

    long GetMicroseconds(string timestamp);

    string GetSHA1Hash(string str);

};

#endif // UTIL_H
