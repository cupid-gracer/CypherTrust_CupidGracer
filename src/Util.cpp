
#include "Util.h"
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <algorithm>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include "SHA1.h"

#include <sstream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/date_time.hpp>

#include <sw/redis++/redis++.h>

#include <json/json.h>
#include <json/value.h>

using namespace std;
using namespace sw::redis;


static string convertToString(char* a, int size) 
{ 
    int i; 
    string s = ""; 
    for (i = 0; i < size; i++) { 
        s = s + a[i]; 
    } 
    return s; 
} 


long Util::GetNowTimestamp()
{
    time_t t = time(0);
    auto now = std::chrono::system_clock::now();
    auto now_ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(now);
    auto epoch = now_ns.time_since_epoch();
    auto value = std::chrono::duration_cast<std::chrono::nanoseconds>(epoch);
    long result = value.count();
    return result;
}

// Function to print permutations of string
// This function takes three parameters:
// 1. String
// 2. Starting index of the string
// 3. Ending index of the string.
void Util::permutation(string a, int l, int r)
{
    // Base case
    if (l == r)
        std::cout << a << std::endl;
    else
    {
        // Permutations made
        for (int i = l; i <= r; i++)
        {

            // Swapping done
            std::swap(a[l], a[i]);

            // Recursion called
            permutation(a, l + 1, r);

            //backtrack
            std::swap(a[l], a[i]);
        }
    }
}

void Util::split(string &str, char delim, std::vector<string> &out)
{
    size_t start;
    size_t end = 0;

    while ((start = str.find_first_not_of(delim, end)) != string::npos)
    {
        end = str.find(delim, start);
        out.push_back(str.substr(start, end - start));
    }
}

/* aaaack but it's fast and const should make it shared text page. */
static const unsigned char pr2six[256] =
    {
        /* ASCII table */
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
        64, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
        64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64};

int Util::Base64decode_len(const char *bufcoded)
{
    int nbytesdecoded;
     const unsigned char *bufin;
    // register int nprbytes;
    int nprbytes;

    bufin = (const unsigned char *)bufcoded;
    while (pr2six[*(bufin++)] <= 63)
        ;

    nprbytes = (bufin - (const unsigned char *)bufcoded) - 1;
    nbytesdecoded = ((nprbytes + 3) / 4) * 3;

    return nbytesdecoded + 1;
}

int Util::Base64decode(char *bufplain, const char *bufcoded)
{
    int nbytesdecoded;
    const unsigned char *bufin;
    // register const unsigned char *bufin;
    unsigned char *bufout;
    // register unsigned char *bufout;
    int nprbytes;
    // register int nprbytes;

    bufin = (const unsigned char *)bufcoded;
    while (pr2six[*(bufin++)] <= 63)
        ;
    nprbytes = (bufin - (const unsigned char *)bufcoded) - 1;
    nbytesdecoded = ((nprbytes + 3) / 4) * 3;

    bufout = (unsigned char *)bufplain;
    bufin = (const unsigned char *)bufcoded;

    while (nprbytes > 4)
    {
        *(bufout++) =
            (unsigned char)(pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
        *(bufout++) =
            (unsigned char)(pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
        *(bufout++) =
            (unsigned char)(pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
        bufin += 4;
        nprbytes -= 4;
    }

    /* Note: (nprbytes == 1) would be an error, so just ingore that case */
    if (nprbytes > 1)
    {
        *(bufout++) =
            (unsigned char)(pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
    }
    if (nprbytes > 2)
    {
        *(bufout++) =
            (unsigned char)(pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
    }
    if (nprbytes > 3)
    {
        *(bufout++) =
            (unsigned char)(pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
    }

    *(bufout++) = '\0';
    nbytesdecoded -= (4 - nprbytes) & 3;
    return nbytesdecoded;
}

static const char basis_64[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int Util::Base64encode_len(int len)
{
    return ((len + 2) / 3 * 4) + 1;
}

int Util::Base64encode(char *encoded, const char *string, int len)
{
    int i;
    char *p;

    p = encoded;
    for (i = 0; i < len - 2; i += 3)
    {
        *p++ = basis_64[(string[i] >> 2) & 0x3F];
        *p++ = basis_64[((string[i] & 0x3) << 4) |
                        ((int)(string[i + 1] & 0xF0) >> 4)];
        *p++ = basis_64[((string[i + 1] & 0xF) << 2) |
                        ((int)(string[i + 2] & 0xC0) >> 6)];
        *p++ = basis_64[string[i + 2] & 0x3F];
    }
    if (i < len)
    {
        *p++ = basis_64[(string[i] >> 2) & 0x3F];
        if (i == (len - 1))
        {
            *p++ = basis_64[((string[i] & 0x3) << 4)];
            *p++ = '=';
        }
        else
        {
            *p++ = basis_64[((string[i] & 0x3) << 4) |
                            ((int)(string[i + 1] & 0xF0) >> 4)];
            *p++ = basis_64[((string[i + 1] & 0xF) << 2)];
        }
        *p++ = '=';
    }

    *p++ = '\0';
    return p - encoded;
}

long Util::ConvertNanoseconds(string timestamp)
{
    using boost::posix_time::ptime;
    ptime pt;
    {
        std::istringstream iss(timestamp);
        auto *f = new boost::posix_time::time_input_facet("%Y-%m-%dT%H:%M:%S%f");

        std::locale loc(std::locale(""), f);
        iss.imbue(loc);
        iss >> pt;
    }
    long result = (pt - ptime{{1970, 1, 1}, {}}).total_nanoseconds();
    return result;
}

string Util::GetSHA1Hash(string str)
{
    return sha1(str);
}





void Util::setStartTimestamp()
{
    startTimestamp = GetNowTimestamp();
}

void Util::setFinishTimestamp()
{
    finishTimestamp = GetNowTimestamp();
}

void Util::publishLatency(string redisURL, string redisConnectorChannel, string object, string addressID, string status, long ts, long latency)
{
    Json::Value obj;

    obj["type"]    = "heartbeat";
    obj["object"]  = object;
    obj["address"] = addressID;
    obj["status"]  = status;
    obj["ts"]      = ts;
    obj["latency"] = latency;

    Json::StreamWriterBuilder streamWriterBuilder;
    streamWriterBuilder["indentation"] = "";
    string out = writeString(streamWriterBuilder, obj);
    out.erase(remove(out.begin(), out.end(), '\n'), out.end());

    using Attrs = vector<pair<string, string>>;
    Attrs attrs = { {"heartbeat", out }};
    Redis redis = Redis(redisURL);
    redis.xadd(redisConnectorChannel, "*", attrs.begin(), attrs.end());
}


int Util::findIndex(vector<string> V, string s)
{
    return find(V.begin(), V.end(), s) - V.begin();
}

bool Util::isValueInVector(vector<string> V, string s)
{
    int nowIndex = find(V.begin(), V.end(), s) - V.begin();
    if(nowIndex < V.size())
        return true;
    else
        return false;
}


string Util::uint64_to_string( uint64_t value ) {
    std::ostringstream os;
    os << value;
    return os.str();
}

string getSHA256(string key, string msg)
{
    // SHA256();
    return "test";
}

string Util::getAddressId(string res, string interface)
{
    stringstream input(res);
    Json::CharReaderBuilder charReaderBuilder;
    Json::Value obj;
    string errs;
    bool isParse = parseFromStream(charReaderBuilder, input, &obj, &errs);

    if (!isParse) return "no";

    Json::Value connectors = obj["connector"];

    if(connectors.size() == 1 || interface == "")
    {
        return connectors[0]["id"].asString();
    }

    for(int i = 0; i < connectors.size(); i++)
    {
        string walletId = connectors[i]["wallet"]["walletId"].asString();
        transform(walletId.begin(), walletId.end(), walletId.begin(), ::tolower);
        if(walletId == interface)
        {
            return connectors[i]["id"].asString(); 
        }
    }

    return "noAddressId";
}

Util::Util()
{
}

Util::~Util()
{
}
 