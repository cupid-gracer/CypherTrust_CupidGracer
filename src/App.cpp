
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



 

App::App(string con_setting_str, string type, string scope)
{
    try
    {
        util = Util();

        this->type = type;
        this->scope = scope;
        /* set Global values*/
        setGlobalValue(con_setting_str);
        /* set redis*/
        thread th(&App::redisMan, this);
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

void App::redisMan()
{
    try
    {
        auto redis = Redis(redisURL);
        
        // Create a Subscriber.
        auto sub = redis.subscriber();

        // Set callback functions.
        sub.on_message([this](string channel, string msg) {
            try{

                cout << msg << endl;
                auto redis = Redis(redisURL);

                Document d;
                Document d_pong;
                rapidjson::Document::AllocatorType &allocator = d_pong.GetAllocator();
                d_pong.SetObject();
                
                ParseResult result = d.Parse(msg.c_str());
                if (!result)
                {
                    std::cerr << "JSON parse error: " << msg << endl;
                    return;
                }
                if(!d.HasMember("type")) return;
                string type = d["type"].GetString();
                if(type == "pong") return;
                long seq = d["seq"].GetUint64();

                d_pong.AddMember("connector", Value().SetString(StringRef(addressID.c_str())), allocator);
                d_pong.AddMember("seq", seq, allocator);
                d_pong.AddMember("ts", util.GetNowTimestamp(), allocator);
                d_pong.AddMember("type", "pong", allocator);

                StringBuffer sb;
                Writer<StringBuffer> w(sb);
                d_pong.Accept(w);
                redis.publish(redisConnectorChannel, sb.GetString());
            }catch(exception e){
                cout << "subscribe error occur :" << e.what() << endl;
            }
        });

        // Subscribe to channels and patterns.

        sub.subscribe(redisConnectorChannel);
      
        // Consume messages in a loop.
        while (true) {
            try {
                sub.consume();

            }
            catch (const Error& err) {
                cout << "\033[1;31mRedis subscribe error occur!\033[0m => " << err.what() << endl;
                syslog(LOG_PERROR, "Redis subscribe error occur!");
            }
        }
    }
    catch (const Error &e)
    {
        cout << "\033[1;31mRedis error occur!\033[0m => " << e.what() << endl;
        syslog(LOG_PERROR, "Redis error occur!");
    }
}


void App::setGlobalValue(string res)
{
    //  get address id
    size_t pos = res.find("\"connector\"");

    string st = res.substr(pos, 40);
    vector<string> st_array;
    util.split(st, '"', st_array);
    addressID = st_array[2];

    string pre_channel = "cyphertrust_database_";

    Document d;
    d.Parse(res.c_str());

    // get redis info
    redisHost = d["bootstrap"]["redisHost"].GetString();
    redisPort = d["bootstrap"]["redisPort"].GetInt();
    redisPassword = d["bootstrap"]["redisPassword"].GetString();

    Value& val_connector = d["connector"][Value().SetString(StringRef(addressID.c_str()))];

    redisConnectorChannel = pre_channel + val_connector["channel"]["connector"].GetString();
    redisOrderBookChannel = pre_channel + val_connector["channel"]["orderbook"].GetString();
    

    // walletName      = val_connector["wallet"]["walletName"].GetString();
    // walletEnabled   = val_connector["wallet"]["walletEnabled"].GetBool();
    // exchangeSecret  = val_connector["wallet"]["exchangeSecret"].GetString();
    // exchangePassword = val_connector["wallet"]["exchangePassword"].GetString();
    // exchangeKey     = val_connector["wallet"]["exchangeKey"].GetString();
    exchangeApiUrl  = val_connector["wallet"]["exchangeApiUrl"].GetString();
    exchangeWsUrl   = val_connector["wallet"]["exchangeWsUrl"].GetString();
    // exchangeRedisOrderChannel = val_connector["wallet"]["exchangeRedisOrderChannel"].GetString();
    // portfolioName   = val_connector["wallet"]["portfolioName"].GetString();

    
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
    util.split(coins, ',', coin_included);
    // const Value &e = c[0]["market"]["included"];
    // for (int i = 0; i < e.Size(); i++)
    // {
    //     coin_included.push_back(e[i].GetString());
    // }
    redisURL = "tcp://" + redisPassword + "@" + redisHost + ":" + to_string(redisPort);
    redisChannel = "cyphertrust_database_" + addressID;

}

void App::run()
{
    // while(1){}
    //test
    type = "HITB";

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

        CBPR cbpr = CBPR(coin_included, api_uri, api_key, secret, passcode, redisURL, addressID);
        cbpr.run();
    }
    else if (type == "HITB")
    {
        string api_key = "wcvsKvAvq5Ta8ZRfk0R_bBi6nReTbMCb";
        string secret_key = "dOYGUFftjS5i--H8rNSoyu8rDRmIDWtH";
        string uri = "api.hitbtc.com/api/2";
        HITB hitb = HITB(coin_included, api_key, secret_key, uri, exchangeWsUrl, redisURL, addressID, redisConnectorChannel, redisOrderBookChannel);
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
        STEX stex = STEX(coin_included, access_token, uri, redisURL, addressID);
        stex.run();
    }
}
