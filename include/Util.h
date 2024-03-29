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
#include <cmath>

using namespace std;

class Util
{
public:
    Util();
    ~Util();
    long startTimestamp, finishTimestamp;

    void permutation(std::string a, int l, int r);
    long GetNowTimestamp();
    int GetNumberCombination(int n, int r);
    void split(std::string &str, char delim, std::vector<std::string> &out);

    int Base64encode_len(int len);
    int Base64encode(char * coded_dst, const char *plain_src,int len_plain_src);

    int Base64decode_len(const char * coded_src);
    int Base64decode(char * plain_dst, const char *coded_src);

    long ConvertNanoseconds(string timestamp);

    string GetSHA1Hash(string str);

    void setStartTimestamp();
    void setFinishTimestamp();
    void publishLatency(string redisURL, string redisHeartbeatChannel, string object, string addressID, string status, long ts, long latency);
    int findIndex(vector<string> V, string s);
    bool isValueInVector(vector<string> V, string s);
    string uint64_to_string( uint64_t value );

    string getSHA256(string key, string data);
    string getAddressId(string res, string interface = "");

    //color
    string Red          = "\033[1;31m";
    string Green        = "\033[1;32m";
    string ResetColor   = "\033[0m";
    string Yellow       = "\033[1;33m";
    string Blue         = "\033[1;34m";
    string Magenta      = "\033[1;35m";
    string Cyan         = "\033[1;36m";
    string BRed         = "\033[1;91m";
    string BGreen       = "\033[1;92m";
    string BYellow      = "\033[1;93m";
    string BBlue        = "\033[1;94m";
    string BMagenta     = "\033[1;95m";
    string BCyan        = "\033[1;96m";
};

#endif // UTIL_H
