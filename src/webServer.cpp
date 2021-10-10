#include "webServer.h"

webServer::webServer(Context* context, Strategy* strategy, Scheduler* scheduler){
    // 绑定context,strategy,scheduler
    this->context = context;
    this->strategy = strategy;
    this->scheduler = scheduler;
    // 初始化日志
    time_t now = time(0);
    char name[8] = "../log/";
    char* dt = ctime(&now);
    strcat(name,dt);
    logger.open(name);
    // 进行sever_address赋值
    bzero(&sever_address,sizeof(sever_address));
    sever_address.sin_family = PF_INET;
    sever_address.sin_addr.s_addr = htons(INADDR_ANY);
    sever_address.sin_port = htons(port);
    // 初始化socket
    sock = socket(AF_INET,SOCK_STREAM,0);
    assert(sock >= 0);
    // 绑定addr与socket
    int ret = bind(sock, (struct sockaddr*)&sever_address, sizeof(sever_address));
    assert(ret != -1);
    // 开始监听
    ret = listen(sock, 1);
    assert(ret != -1);
    std::cout << "Starting Web Service." << std::endl;
    while(true){
        struct sockaddr_in client;
        socklen_t client_addrlength = sizeof(client);
        connfd = accept(sock, (struct sockaddr*)&client, &client_addrlength);
        if(connfd < 0){
            logger << "errno" << std::endl;
        }
        else{
            char request[1024];
            recv(connfd, request, 1024, 0);
            request[strlen(request)+1]='\0';
            for(int i = 0; i < strlen(request); i++)
                logger << request[i];
            logger << std::endl;
            logger << "successeful!" << std::endl;
            char buf[520]="HTTP/1.1 200 ok\r\nconnection: close\r\n\r\n"; // HTTP响应
            int s = send(connfd, buf, strlen(buf), 0);        // 发送响应
            updateHtmlFile();
            int fd = open("../www/main.html", O_RDONLY);      // 消息体
            sendfile(connfd, fd, NULL, 2500);                 // 零拷贝发送消息体
            close(fd);
            close(connfd);
        }
    }
}

webServer::~webServer(){
    logger.close();
    fout.close();
}

void webServer::updateHtmlFile(){
    if(context == nullptr)
        return;
    fout.open("../www/main.html");
    fout << "<html>" << std::endl;
    fout << "    <body bgcolor=\"white\">" << std::endl;
    fout << "            DisPlay System Status." << std::endl;
    fout << "        <hr>" << std::endl;
    fout << "            <p>Node Num: " << context->getNodesNum() << "</p>" << std::endl;
    fout << "            <p>App Num: " << context->getAppsNum() << "</p>" << std::endl;
    fout << "            <p>TaskInfo Num: " << context->getTaskInfosNum() << "</p>" << std::endl;
    fout << "            <p>Task Num: " << context->getTasksNum() << "</p>" << std::endl;
    fout << "            <p>" << context->getTasksDetail() << "</p>" << std::endl;
    fout << "            <p>Waiting Queue Num: " << strategy->getWaitingQueuesNum() << "</p>" << std::endl;
    fout << "            <p>" << strategy->getWaitingQueuesDetail() << "</p>" << std::endl;
    fout << "            <p>Ready Queue Size: " << scheduler->getReadyQueueSize() << "</p>" << std::endl;
    fout << "            <p>" << scheduler->getReadyQueueTop() << "</p>" << std::endl;
    fout << "        <br>" << std::endl;
    fout << "    </body>" << std::endl;
    fout << "</html>" << std::endl;
    fout.close();
}