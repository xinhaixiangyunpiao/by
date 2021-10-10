#ifndef WEBSERVER
#define WEBSERVER

#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <fstream>
#include <ctime>
#include <string>
#include <string.h>
#include "context.h"
#include "strategy.h"
#include "scheduler.h"

class Context;
class Strategy;
class Scheduler;

const int port = 5700;

class webServer{
private:
    int sock;
    int connfd;
    struct sockaddr_in sever_address;
    Context* context;
    Strategy* strategy;
    Scheduler* scheduler;
    std::ofstream logger;
    std::ofstream fout;
public:
    webServer(Context* context, Strategy* strategy, Scheduler* scheduler);
    ~webServer();
    void updateHtmlFile();
};

#endif