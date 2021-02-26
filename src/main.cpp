/* CypherTrust */

#include "API.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <boost/algorithm/string.hpp>
#include <iomanip>
#include <sw/redis++/redis++.h>
#include <syslog.h>
#include <csignal>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

/* Exchange */
#include "exchange/STEX/STEX.h"
#include "exchange/CBPR/CBPR.h"
#include "exchange/HITB/HITB.h"

/* rapidjson */
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"


#define DAEMON_NAME "CypherTrust"

using namespace rapidjson;
using namespace sw::redis;
using namespace std;

/* Functions Declare */
void redisMan();
void setGlobalValue(string res);
void signal_handler(int sig);
void daemonize(const char *rundir, const char *pidfile);
void daemonShutdown();

/* Variable Declare */
// bootstrap variables
string  redisManagementChannel,
        redisHeartbeatChannel,
        redisOrderBookChannel,
        redisHost,
        walletName,
        exchangeSecret,
        exchangePassword,
        exchangeKey,
        exchangeApiUrl,
        exchangeWsUrl,
        exchangeRedisOrderChannel,
        portfolioName;
int redisPort;
bool walletEnabled;
vector<string> coin_included;

//Daemon variable
bool exitdaemon = false;
int pidFilehandle;

//Daemon Processing
void signal_handler(int sig)
{
    openlog(DAEMON_NAME, LOG_CONS | LOG_PERROR, LOG_USER);
    switch(sig)
    {
        case SIGHUP:
            syslog(LOG_INFO, "Terminal was closed");
            break;
        case SIGTERM:
            syslog(LOG_INFO, "Graceful termination");
            exitdaemon = true;
            break;
        case SIGKILL:
            syslog(LOG_INFO, "Forced termination with core dump");
            break;
        case SIGQUIT:
            syslog(LOG_INFO, "Forced termination");
            break;
        case SIGCONT:
            syslog(LOG_INFO, "Execution continued after pause");
            break;
        case SIGSTOP:
            syslog(LOG_INFO, "Execution paused");
            break;
    }
}

void daemonize(const char *rundir, const char *pidfile)
{
    int pid, sid, i;
    char str[10];
    struct sigaction newSigAction;
    sigset_t newSigSet;

    /* Check if parent process id is set */
    if (getppid() == 1)
    {
        /* PPID exists, therefore we are already a daemon */
        return;
    }

    /* Set signal mask - signals we want to block */
    sigemptyset(&newSigSet);
    sigaddset(&newSigSet, SIGCHLD);  /* ignore child - i.e. we don't need to wait for it */
    sigaddset(&newSigSet, SIGTSTP);  /* ignore Tty stop signals */
    sigaddset(&newSigSet, SIGTTOU);  /* ignore Tty background writes */
    sigaddset(&newSigSet, SIGTTIN);  /* ignore Tty background reads */
    sigprocmask(SIG_BLOCK, &newSigSet, NULL);   /* Block the above specified signals */

    /* Set up a signal handler */
    newSigAction.sa_handler = signal_handler;
    sigemptyset(&newSigAction.sa_mask);
    newSigAction.sa_flags = 0;

    /* Signals to handle */
    sigaction(SIGHUP,  &newSigAction, NULL);     /* Stop and redo the bootstrapping process */
    sigaction(SIGKILL, &newSigAction, NULL);     /* Immediate termination of the connector */
    sigaction(SIGQUIT, &newSigAction, NULL);     /* Forced termination of the connector */
    sigaction(SIGTERM, &newSigAction, NULL);     /* Graceful termination of the connector */
    sigaction(SIGCONT, &newSigAction, NULL);     /* Continue the order book streaming */
    sigaction(SIGSTOP, &newSigAction, NULL);     /* Pause the order book streaming  */


    /* Fork*/
    pid = fork();

    if (pid < 0)
    {
        /* Could not fork */
        exit(EXIT_FAILURE);
    }

    if (pid > 0)
    {
        /* Child created ok, so exit parent process */
        //printf("Child process created: %d\n", pid);
        exit(EXIT_SUCCESS);
    }

    /* Child continues */

    umask(027); /* Set file permissions 750 */

    /* Get a new process group */
    sid = setsid();

    if (sid < 0)
    {
        exit(EXIT_FAILURE);
    }

    /* close all descriptors */
    for (i = getdtablesize(); i >= 0; --i)
    {
        close(i);
    }

    /* Route I/O connections */

    /* Open STDIN */
    i = open("/dev/null", O_RDWR);

    /* STDOUT */
    dup(i);

    /* STDERR */
    dup(i);

    chdir(rundir); /* change running directory */

    /* Ensure only one copy */
    pidFilehandle = open(pidfile, O_RDWR|O_CREAT, 0600);

    if (pidFilehandle == -1 )
    {
        /* Couldn't open lock file */
        syslog(LOG_INFO, "Could not open PID lock file %s, exiting", pidfile);
        exit(EXIT_FAILURE);
    }

    /* Try to lock file */
    if (lockf(pidFilehandle,F_TLOCK,0) == -1)
    {
        /* Couldn't get lock on lock file */
        syslog(LOG_INFO, "Could not lock PID lock file %s, exiting", pidfile);
        exit(EXIT_FAILURE);
    }


    /* Get and format PID */
    sprintf(str,"%d\n",getpid());

    /* write pid to lockfile */
    write(pidFilehandle, str, strlen(str));
}

void daemonShutdown()
{
    openlog(DAEMON_NAME, LOG_CONS | LOG_PERROR, LOG_USER);
    syslog(LOG_INFO, "Daemon exiting");
    close(pidFilehandle);
}


/* Bootstrap Processing */

bool bootstrap(int argc, char *argv[])
{
    string user, password, type, key = "9xds0be661dc5a42e08cdca3b0bbd4";
    if(argc < 7) {
        cout << "Number of parameters is not correct!" << endl;
        return false;
    }

    for(int i = 0; i < argc; i++)
    {
        string s = argv[i];
        if(s == "-u")
        {
            user = argv[i+1];
        }
        if(s == "-p")
        {
            password = argv[i+1];
        }
        if(s == "-t")
        {
            type = argv[i+1];
        }
    }
    if(user == "" || password == "" || type == "")
    {
        cout << "The parameters you inputed are wrong!" << endl;
        return false;
    }


    API auth_api;
    auth_api.auth_url = "http://auth.dev.cyphertrust.eu/v1/connector/auth";

    //test
    user =  "fmaertens";
    password = "x2NGo87ympHowidyPMhn";

    string res = auth_api.auth(user, password, key);
    if(res == "error"){
        cout << "Authentication is failed!" << endl;
        return false;
    }
    
    /* set Global values*/
    setGlobalValue(res);
    /* set redis*/
    redisMan();

    return true;
}

void setGlobalValue(string res)
{
    Document d;
    d.Parse(res.c_str());

    redisManagementChannel = d["bootstrap"]["redisManagementChannel"].GetString();
    redisHeartbeatChannel = d["bootstrap"]["redisHeartbeatChannel"].GetString();
    redisOrderBookChannel = d["bootstrap"]["redisOrderBookChannel"].GetString();
    redisHost = d["bootstrap"]["redisHost"].GetString();
    redisPort = d["bootstrap"]["redisPort"].GetInt();

    const Value &c = d["connector"];

    walletName = c[0]["wallet"]["walletName"].GetString();
    walletEnabled = c[0]["wallet"]["walletEnabled"].GetBool();
    exchangeSecret = c[0]["wallet"]["exchangeSecret"].GetString();
    exchangePassword = c[0]["wallet"]["exchangePassword"].GetString();
    exchangeKey = c[0]["wallet"]["exchangeKey"].GetString();
    exchangeApiUrl = c[0]["wallet"]["exchangeApiUrl"].GetString();
    exchangeWsUrl = c[0]["wallet"]["exchangeWsUrl"].GetString();
    exchangeRedisOrderChannel = c[0]["wallet"]["exchangeRedisOrderChannel"].GetString();
    portfolioName = c[0]["wallet"]["portfolioName"].GetString();

    const Value &e = c[0]["market"]["included"];
    for(int i = 0; i < e.Size(); i++)
    {
        coin_included.push_back(e[i].GetString());
    }
}


void redisMan()
{
    try{
        cout << "tcp://" + redisHost + ":" + to_string(redisPort) << endl; 
        string url = "tcp://" + redisHost + ":" + to_string(redisPort);
        auto redis = Redis("127.0.0.1:6379");
        // cout << redis.ping("");
        // auto redis = sw::redis::Redis(("tcp://" + redisHost + ":" + to_string(redisPort)).c_str());
        // redis.set("key", "value");
        // auto val = redis.get("key");    // val is of type OptionalString. See 'API Reference' section for details.
        // if (val) {
        //     // Dereference val to get the returned value of std::string type.
        //     std::cout << *val << std::endl;
        // }   // else key doesn't exist.
    }catch(const Error &e){
        syslog(LOG_PERROR, "Redis error occur!");
    }
}


/* main */
int main(int argc, char *argv[])
{
    /* Bootstraping... */
    if(!bootstrap(argc, argv)){
        return 0;
    }

    /* Logging */
    setlogmask(LOG_UPTO(LOG_INFO));
    openlog(DAEMON_NAME, LOG_CONS | LOG_PERROR, LOG_USER);

    syslog(LOG_INFO, "CrypherTrust Daemon starting up!");

    /* Deamonize */
    // const char* daemonpid = "/var/run/srv_test.pid";
    const char* daemonpid = "/home/cupid/srv_test.pid";
    const char* daemonpath = "/";
    daemonize(daemonpath, daemonpid);

    syslog(LOG_INFO, "CrypherTrust Daemon running");

    while (!exitdaemon)
    {
        sleep(10);
    }

    daemonShutdown();

    return 0;
}



