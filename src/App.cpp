
#include "API.h"
#include "App.h"
#include <vector>
#include <iostream>
#include "curl/curl.h"
#include <sw/redis++/redis++.h>
#include <syslog.h>
#include <csignal>
#include <thread>

/* rapidjson */
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

/* Exchange */
// #include "exchange/BNUS/BNUS.h"
// #include "exchange/BTFX/BTFX.h"
#include "exchange/CBPR/CBPR.h"
#include "exchange/HITB/HITB.h"
// #include "exchange/HUOB/HUOB.h"
// #include "exchange/KRKN/KRKN.h"
// #include "exchange/PLNX/PLNX.h"
#include "exchange/STEX/STEX.h"

#include "Util.h"

using namespace rapidjson;
using namespace sw::redis;
using namespace std;

App::App(string con_setting_str, string type)
{
    try
    {
        this->type = type;
        /* set Global values*/
        setGlobalValue(con_setting_str);
        /* set redis*/
        thread th(redisMan);
        th.detach();
    }
    catch (const Error &e)
    {
        cout << "\033[1;31mApp initial error occur!\033[0m => " << e.what() << endl;
        syslog(LOG_PERROR, "App initial error occur!");
    }
}

App::~App()
{
}

void App::setGlobalValue(string res)
{
    //  get address id
    size_t pos = res.find("connector");
    string st = res.substr(pos, 40);
    Util util = Util();
    vector<string> st_array;
    util.split(st, '"', st_array);
    address_id = st_array[2];

    Document d;
    d.Parse(res.c_str());

    // get redis info
    redisHeartbeatChannel = d["bootstrap"]["redisHeartbeatChannel"].GetString();
    redisOrderBookChannel = d["bootstrap"]["redisOrderBookChannel"].GetString();
    redisHost = d["bootstrap"]["redisHost"].GetString();
    redisPort = d["bootstrap"]["redisPort"].GetInt();
    logHost = d["bootstrap"]["logHost"].GetString();
    logPort = d["bootstrap"]["logPort"].GetInt();

    // const Value &c = d["connector"][address_id];

    // redisManagementChannel = d["address"]["redisManagementChannel"].GetString();
    // connectorID = d["address"]["id"].GetString();
    // expiration = d["address"]["expiration"].GetString();

    // walletName      = c[0]["wallet"]["walletName"].GetString();
    // walletEnabled   = c[0]["wallet"]["walletEnabled"].GetBool();
    // exchangeSecret  = c[0]["wallet"]["exchangeSecret"].GetString();
    // exchangePassword = c[0]["wallet"]["exchangePassword"].GetString();
    // exchangeKey     = c[0]["wallet"]["exchangeKey"].GetString();
    // exchangeApiUrl  = c[0]["wallet"]["exchangeApiUrl"].GetString();
    // exchangeWsUrl   = c[0]["wallet"]["exchangeWsUrl"].GetString();
    // exchangeRedisOrderChannel = c[0]["wallet"]["exchangeRedisOrderChannel"].GetString();
    // portfolioName   = c[0]["wallet"]["portfolioName"].GetString();
    walletName = "Coinbase Pro Sandbox Dev";
    walletEnabled = true;
    exchangeSecret = "dOYGUFftjS5i--H8rNSoyu8rDRmIDWtH";
    exchangePassword = "k7on2nlkl1s";
    exchangeKey = "wcvsKvAvq5Ta8ZRfk0R_bBi6nReTbMCb";
    exchangeApiUrl = "api.hitbtc.com/api/2";
    exchangeWsUrl = "ws-feed.pro.coinbase.com";
    exchangeRedisOrderChannel = "orders.cb.aldenburgh";
    portfolioName = "Aldenburgh";

    string coins = "ACU,ETH,ARDR,BTC,BTCZ,LTC,UFR,LNK,NXT,SHND,DOGE,SMART,USDT,CLO,ELLA,XSH,BCN,ZEUS,RSCT,PGC,PIRL,ORE,BCHA,EAG,IGNIS,FLG,ESH,NUKO,MSR,STAK,MFC,ONION,GBX,SLRM,REAK,DERO,B2B,BTCL,SPK,ETC,TENT,BLTG,RUPX,BHD,TSS,WAVES,ETG,BEET,FERMA,BITG,ZEL,BBK,DATO,ABS,AKA,DIC,BTCN,DASH,LCC,VIT,CROAT,TRX,XVG,SEM,ISC,NCP,BLAST,BCA,C2C,NEXO,EPLUS,RTH,BENZ,ACTN,ZEC,ELY,EIB,ATH,ETHO,PLTS,NILU,ZEN,TLR,MINTME,NANJ,BRF,SKB,DOW,THRN,ICR,EUNO,BTRL,BBR,QURO,GPKR,ITL,MNP,DVS,XTW,CDM,XSM,GPYX,DSC,MASH,BCI,ESBC,ADK,BEVERAGE,XRP,FA,XCASH,ATCC,BLOC,SIGN,XBI,XGK,MBC,SICC,D4RK,SIN,BYC,BAK,VRSC,CCX,DAPS,sBTC,TTP,LUNES,SCRIV,HTA,BZX,FREE,BDX,OHC,IZE,XNB,EGEM,X42,EthID,GIT,ILC,ZUM,CLM,TUP,HELP,DOGEC,BUL,BLE,HLIX,TUSD,VDL,TOC,BGX,INVC,AIB,PENG,DASHG,VESTX,JOOS,BHIG,VEST,QAC,OWC,ORM,ABET,ABST,PNY,CUT,OCUL,GVC,ELD,LTFG,SPRKL,ENJ,MNC,MAR,HBX,VJC,EUM,ANT,FUSION,REEC,RC20,SVR,OWO,CLOAK,VEIL,OBX,AER,PSD,CSC,ZCOR,PRKL,TSF,BITC,EXO,BCEO,CLASSY,BTH,ACRYL,BNY,BST,WEC,TELE,BZE,xEUR,EUR,ANON,GMCC,TVT,XLM,ZNN,HEDG,TEO,LOT,VRA,LONG,AMR,REV,AZT,SYO,XAC,EXOR,IDC,VITAE,BITSW,BURST,DYN,SEQ,SBE,SFX,ZANO,SINGH,BTNT,GPS,WCC,BNB,BCNA,CXC,PART,VTX,SWACE,NBX,DQI,OK,DGTX,DAB,COVAL,ARDX,GFG,N8V,VOLTZ,AGNT,UPX,ECR,AUDAX,XBG,ION,ECA,VRAB,PBC,JSC,BC,HTML,CBUCKS,PLF,RNDR,SURE,USDA,YFX,DSLA,CDEX,LST,ASY,DIVI,NYC,KICK,HMR,LMCH,DGB,WABI,CHBT,XDC,JUP,OCEAN,HYD,CRT,CSPN,STREAM,STP,HALO,INNBC,DMST,TNC,STS,XXA,CPTL,ZCL,CRS,DEB,GHOST,ZPAE,FLS,GIG,COMP,XRT,BAL,HIVE,XTZ,EOS,VLT,GET,BTX,ICH,IDX,PHNX,EVC,SRK,SFR,AKRO,NEC,GNO,TKN,MTA,MARS,TAN,CRDT,DRS,PLEX,OPM,ETHV,LINK,BAND,RDBX,SCC,USDC,CEEK,UMA,NOVO,UNI,WBTC,CR8,PAYT,DAI,SWFL,XZT,SNL,XCUR,RKN,NLG,YOLK,VBIT,BNZ,DOOS,EDC,DESH,JNTR,BLURT,MPH,S4F,DGW,CHESS,DXF,UCOIN,SMB,CRBN,DOT,WIFI,SIC,LOG";
    Util util = Util();
    util.split(coins, ',', coin_included);
    // const Value &e = c[0]["market"]["included"];
    // for (int i = 0; i < e.Size(); i++)
    // {
    //     coin_included.push_back(e[i].GetString());
    // }
    redisURL = "tcp://" + redisHost + ":" + to_string(redisPort);

   
    string str_s = auth_api.user + "" + auth_api.password + "" + exchange_type;
    scope = util.GetSHA1Hash(str_s);
}

void App::redisMan()
{
    try
    {

        cout << "tcp://" + redisHost + ":" + to_string(redisPort) << endl;
        string url = "tcp://" + redisHost + ":" + to_string(redisPort);

        auto redis = Redis("tcp://127.0.0.1:6379");
        // auto redis = Redis(url);
        
        // Create a Subscriber.
        auto sub = redis.subscriber();
        string channelName = "connector." + scope + "." + address_id;

        // Set callback functions.
        sub.on_message([](string channel, string msg) {
            Document d_pong;
            d_pong.Parse(msg.c_str());
            uint64_t ts = d_pong["ts"].GetUint64();

            Document d_ping;
            d_ping.SetObject();
            AllocatorType& allocator = d_ping.GetAllocator();
            d_ping.AddMember("connector", Value().SetString(StringRef(address_id.c_str())), allocator);
            d_ping.AddMember("ts", ts, allocator);
            d_ping.AddMember("type", Value().SetString(StringRef("ping".c_str())), allocator);
            StringBuffer sb;
            Writer<StringBuffer> w(sb);
            d_ping.Accept(w);

        });

        // Subscribe to channels and patterns.
        sub.subscribe(channelName);

        // Consume messages in a loop.
        while (true) {
            try {
                sub.consume();
            }
            catch (const Error& err) {
                // Handle exceptions.
            }
        }
    }
    catch (const Error &e)
    {
        cout << "\033[1;31mRedis error occur!\033[0m => " << e.what() << endl;
        syslog(LOG_PERROR, "Redis error occur!");
    }
}

void App::run()
{
    //test
    type = "STEX";
    connectorID = "e0b3223caee87c25c44c0f1ff6a1ff81a4c21c5e";
    redisURL = "tcp://127.0.0.1:6379";

    if (type == "BNUS")
    {
        // BNUS bnus = BNUS();
        // bnus.run();
    }
    else if (type == "BTFX")
    {
        // BTFX btfx = BTFX();
        // btfx.run();
    }
    else if (type == "CBPR")
    {
        string api_uri = "api-public.sandbox.pro.coinbase.com";
        string api_key = "11dfbdd50299d30a03ea46736da2cb73";
        string secret = "Aw4LtSL8CPTciYXVqV7s1ZZyiFdWgIu0nDeHWuGqK5LvUgAi1ACPPyiJY4uN65+7DgF9D0QzAVGFp4FaVHmWxw==";
        string passcode = "k7on2nlkl1s";

        CBPR cbpr = CBPR(coin_included, api_uri, api_key, secret, passcode, redisURL, connectorID);
        cbpr.run();
    }
    else if (type == "HITB")
    {
        string api_key = "wcvsKvAvq5Ta8ZRfk0R_bBi6nReTbMCb";
        string secret_key = "dOYGUFftjS5i--H8rNSoyu8rDRmIDWtH";
        string uri = "api.hitbtc.com/api/2";
        HITB hitb = HITB(coin_included, api_key, secret_key, uri, redisURL, connectorID);
        hitb.run();
    }
    else if (type == "HUOB")
    {
        // HUOB huob = HUOB();
        // huob.run();
    }
    else if (type == "KRKN")
    {
        // KRKN krkn = KRKN();
        // krkn.run();
    }
    else if (type == "PLNX")
    {
        // PLNX plnx = PLNX();
        // plnx.run();
    }
    else if (type == "STEX")
    {
        string uri = "api3.stex.com";
        string access_token = "eyJ0eXAiOiJKV1QiLCJhbGciOiJSUzI1NiJ9.eyJhdWQiOiIxIiwianRpIjoiYjE5ZmJmNzUwNTg1ZjgyNzgxN2M4NTMxNzc5ZDY1ODIyNGVhYmVkZTExNjc1NWQ1MTc5ODFmYTQxMjZkZTJiNDRhYjZkMmNlNDUwZGQ1ZjkiLCJpYXQiOjE2MTIyNTE0OTAsIm5iZiI6MTYxMjI1MTQ5MCwiZXhwIjoxNjQzNzg3NDkwLCJzdWIiOiI0MjcyODAiLCJzY29wZXMiOlsicHJvZmlsZSIsInRyYWRlIiwid2l0aGRyYXdhbCIsInJlcG9ydHMiLCJwdXNoIiwic2V0dGluZ3MiXX0.SdzP9kwOb73vEyVGr7hcnF39eWTSbTLjr5Bv1UrwUCfP5sG-_haaK-_wF_OPt2fOhMbSi_JQ9y50LCefBX-TsRzcX38P0O4eoRqFhQpTNQwSDJ4VISpdL4MvMybDXaHTsDwgSvHRS-f8Ji3JjfO2aCqEmFOkKM-s4Cc5AaiIUM7-UVvGGv_5hkmQsr0Q1ssO21v7qGa1nHagn4CJsSBVjEK6yKls0IaargV8MbkMVgRErbcVyPdxi6bBa_eOh2sqGEyh_ghdE60hDjapaYqT0PA7WLNCULdt23q65bUEIgo3yLehvpM4kUdGMwEadB3d4FZj5Dcgu1Kc7R7InrpWGA4rd3sWLWssx7o_oT1UgD0q_G_AlAB70foBSYa77W9W64ZOge0Gqnq-gvkBRj33V0Nd3eJ6-JbWXhUEguf6xcxqmnVE_sdTAtEw-BV--pbAK14dgcs83MUREIaVhKqgp-AkIYtgU3-nZf0Rip2Q7wfkBYhvZaD2CTcBkKJjL8oxIry1kIGaV2R3mTuB4b4UhI_KsATbTnGem7jEQXU3U6R2W98hoAaqZazBc44hgtJ7rSs4s6LZJ4CZ8j6wT260-gK-ahwMpO1d03GMx9LlIC5aqXIgOB5kIf1_63xMOu5NS0QnuHpJAscpAHZEwTCBkPwM4gjQFSnVtOPnsn68m_U";
        STEX stex = STEX(coin_included, access_token, uri, redisURL, connectorID);
        stex.run();
    }
}
