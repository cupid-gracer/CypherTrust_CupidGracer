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


#include "App.h"
#include "Util.h"

#define DAEMON_NAME "CypherTrust"

using namespace std;
using namespace std::chrono;

/* Functions Declare */
void signal_handler(int sig);
void daemonize(const char *rundir, const char *pidfile);
void daemonShutdown();
void startThread();
void stopThread();
bool getConsoleValue();
void releaseAddress();
bool Bootstraping();
Util util;

//Daemon variable
string con_setting_str = "";
bool exitdaemon = false;
static bool isConnectorDone = false;
int pidFilehandle;
string  userName,
        password,
        exchangeName,
        walletInterface,
        exchangeType,
        addressId,
        scope;

bool    isConfig = false,
        isLAN = false,
        isHelp = false,
        isDaemon = false;

API auth_api;
App *app;
int signal_status;

//Daemon Processing
void signal_handler(int sig)
{
    openlog(DAEMON_NAME, LOG_CONS | LOG_PERROR, LOG_USER);
        cout << util.Green << "\n - Signal processing..." << util.ResetColor << endl;

    switch (sig)
    {
    case SIGHUP:
        syslog(LOG_INFO, "Terminal was closed");
        auth_api.LoggingEvent(addressId, "INFO", "storage", "connector stopped because 'SIGHUP' occurs.");
        signal_status = 1;
        stopThread();
        releaseAddress();
        Bootstraping();
        startThread();
        break;
    case SIGSTOP:
        syslog(LOG_INFO, "Execution paused");
        auth_api.LoggingEvent(addressId, "INFO", "storage", "connector stopped because 'SIGSTOP' occurs.");
        signal_status = 2;
        stopThread();
        break;
    case SIGCONT:
        syslog(LOG_INFO, "Execution continued after pause");
        auth_api.LoggingEvent(addressId, "INFO", "storage", "connector continued because 'SIGCONT' occurs.");
        startThread();
        signal_status = 3;
        break;
    case SIGTERM:
        syslog(LOG_INFO, "Graceful termination");
        auth_api.LoggingEvent(addressId, "INFO", "storage", "connector stopped because 'SIGTERM' occurs.");
        stopThread();
        releaseAddress();
        exitdaemon = true;
        signal_status = 4;
        break;
    case SIGQUIT:
        syslog(LOG_INFO, "Forced termination");
        auth_api.LoggingEvent(addressId, "INFO", "storage", "connector stopped because 'SIGQUIT' occurs.");
        stopThread();
        releaseAddress();
        exitdaemon = true;
        signal_status = 5;
        break;
    case SIGKILL:
        syslog(LOG_INFO, "Forced termination with core dump");
        auth_api.LoggingEvent(addressId, "INFO", "storage", "connector stopped because 'SIGKILL' occurs.");
        signal_status = 6;
        break;
    case SIGINT:
        auth_api.LoggingEvent(addressId, "INFO", "storage", "connector stopped because 'SIGINT' occurs.");
        stopThread();
        releaseAddress();
        exitdaemon = true;
        signal_status = 4;
        break;
    }
}

void signal_callback_handler(int signum) {
   cout << "Caught signal " << signum << endl;
   // Terminate program
   exit(signum);
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
    sigaddset(&newSigSet, SIGCHLD);           /* ignore child - i.e. we don't need to wait for it */
    sigaddset(&newSigSet, SIGTSTP);           /* ignore Tty stop signals */
    sigaddset(&newSigSet, SIGTTOU);           /* ignore Tty background writes */
    sigaddset(&newSigSet, SIGTTIN);           /* ignore Tty background reads */
    sigprocmask(SIG_BLOCK, &newSigSet, NULL); /* Block the above specified signals */

    /* Set up a signal handler */
    newSigAction.sa_handler = signal_handler;
    sigemptyset(&newSigAction.sa_mask);
    newSigAction.sa_flags = 0;

    /* Signals to handle */
    sigaction(SIGHUP,  &newSigAction, NULL);  /* Stop and redo the bootstrapping process */
    sigaction(SIGKILL, &newSigAction, NULL); /* Immediate termination of the connector */
    sigaction(SIGQUIT, &newSigAction, NULL); /* Forced termination of the connector */
    sigaction(SIGTERM, &newSigAction, NULL); /* Graceful termination of the connector */
    sigaction(SIGCONT, &newSigAction, NULL); /* Continue the order book streaming */
    sigaction(SIGSTOP, &newSigAction, NULL); /* Pause the order book streaming  */
    sigaction(SIGINT, &newSigAction, NULL); /* Keyboard event catch (ctrl + c)  */

    if(!isDaemon) return;
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
        printf("Child process created: %d\n", pid);
        exit(EXIT_SUCCESS);
    }

    /* Child continues */


    /* Get a new process group */
    sid = setsid();

    if (sid < 0)
    {
        exit(EXIT_FAILURE);
    }

     umask(0); /* Set file permissions 750 */
    // umask(027); /* Set file permissions 750 */

    /* Change the working directory to the root directory */
    /* or another appropriated directory */
    chdir("/");


    /* close all descriptors */
    for (i = getdtablesize(); i >0; --i)
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
}

void daemonShutdown()
{
    openlog(DAEMON_NAME, LOG_CONS | LOG_PERROR, LOG_USER);
    cout << util.Green << " - CypherTrust finished" << util.ResetColor << endl;
    close(pidFilehandle);
}

bool getConsoleValue(int argc, char *argv[])
{
    if(argc < 7) {
        cout << util.Red  << "Number of parameters is not correct! It needs parameters such as -u, -p, -e, -i, -c, -lan, -h, -D" << endl;
        return false;
    }

    for(int i = 0; i < argc; i++)
    {
        string s = argv[i];
        if(s == "-u")
        {
            userName = argv[i+1];
        }
        if(s == "-p")
        {
            password = argv[i+1];
        }
        if(s == "-t")
        {
            exchangeType = argv[i+1];
            if(!(exchangeType == "stream" || exchangeType == "trade"))
            {
                cout << util.Red  << "Exchange type is wrong!( -t "<< exchangeType << " ) please confirm it." << util.ResetColor  << endl;
                return false;
            }
        }
        if(s == "-e")
        {
            exchangeName = argv[i+1];
            string origin = exchangeName;
            transform(exchangeName.begin(), exchangeName.end(), exchangeName.begin(), ::toupper);
            if(!(exchangeName == "HITB" || exchangeName == "CBPR" || exchangeName == "BNUS" || exchangeName == "EXMO"))
            {
                cout << util.Red  << "Exchange name is wrong!( -e "<< origin << " ) please confirm it." << util.ResetColor  << endl;
                return false;
            }
        }
        if(s == "-i")
        {
            walletInterface = argv[i+1];
            string origin = walletInterface;
            transform(walletInterface.begin(), walletInterface.end(), walletInterface.begin(), ::tolower);
        }
        if(s == "-c")
        {
            isConfig = true;
        }
        if(s == "-lan")
        {
            isLAN = true;
        }
        if(s == "-h")
        {
            isHelp = true;
        }
        if(s == "-D")
        {
            isDaemon = true;
        }
    }
    if(userName == "" || password == "" || exchangeName == "" )
    {
        cout << util.Red << "The parameters you inputed are wrong!" << util.ResetColor  << endl;
        return false;
    }
    return true;
}
/* Bootstrap Processing */
bool Bootstraping()
{
    auth_api.url = "https://hub.cyphertrust.eu/v1";

    auth_api.user = userName;
    auth_api.password = password;
    auth_api.key = exchangeName;
    auth_api.type = exchangeType;

    string res = auth_api.auth();
    if (res == "error")
    {
        cout << util.Red << "Authentication is failed!" << util.ResetColor << endl;
        return false;
    }

    if(isConfig)
    {
        stringstream input(res);

        Json::CharReaderBuilder charReaderBuilder;
        Json::Value obj;
        string errs;
        bool isParse = parseFromStream(charReaderBuilder, input, &obj, &errs);

        if (!isParse) return false;

        Json::StreamWriterBuilder streamWriterBuilder;
        string out = writeString(streamWriterBuilder, obj);
        cout << util.Yellow << out << util.ResetColor << endl;
    }
    con_setting_str = res;

    addressId = util.getAddressId(res, walletInterface);
    if(addressId == "noAddressId")
    {
        cout << util.Red << "The interface you inputed is no! Please input correct interface again." << util.ResetColor << endl;
    }
    if(addressId == "no")
    {
        cout << util.Red << "Error occurs while bootstrapping. Please try again!" << util.ResetColor << endl;
    }



    auth_api.LoggingEvent(addressId, "INFO", "auth", "The Connector " + addressId + " 's bootstrapping is done.");
    string str_s = auth_api.user + "" + auth_api.password + "" +  exchangeName ;
    scope = util.GetSHA1Hash(str_s);

    return true;
}

void releaseAddress()
{
    auth_api.del_address(addressId);
}

void startThread()
{
    //app create
    app = new App(&signal_status, con_setting_str, exchangeName, scope, auth_api, isLAN, walletInterface, &exitdaemon);
    app->run(true);  
}

void stopThread()
{
    app->~App();
}

void daemonThreadStart()
{


    // /* Logging */
    setlogmask(LOG_UPTO(LOG_INFO));
    openlog(DAEMON_NAME, LOG_CONS | LOG_PERROR, LOG_USER);


    /* Deamonize */
    const char *daemonpid = "/home/cupid/srv_test.pid";
    const char *daemonpath = "/";

    daemonize(daemonpath, daemonpid);
}

void daemonThreadShutdown()
{
    int i = 0;
    while (1)
        {
            if (i++ > 5)
                break;
            this_thread::sleep_for(chrono::seconds(3));
        }

    while (!exitdaemon)
    {
        sleep(10);
    }

    daemonShutdown();
}

void test(){
    while(true){
        this_thread::sleep_for(chrono::seconds(1));
        if(signal_status >= 1)
        {
            daemonThreadStart();
            break;
        }
    }
}

/* main */
int main(int argc, char *argv[])
{
    util = Util();
    /* get arguments from console */
    if(!getConsoleValue(argc, argv))
    {
        return 0;
    }

    cout << util.Green << "\n\n\tCypherTrust Connector Daemon " << util.ResetColor << "v1.0\n\t(c) 2021, " << util.Green << "Aldenburgh BV -Project \"CypherTrust\"\n" << util.ResetColor << endl;

    /* Bootstraping... */
    if (!Bootstraping())
    {
        return 0;
    }

    cout << util.Green << " - Bootstrapping connectord: Done" << util.ResetColor << endl;
    /* daemon Thread start */
    daemonThreadStart();
    startThread();

    daemonThreadShutdown();
    return 0;
}
