#include <json/json.h>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#include "Server.h"
#include "BasicController.h"

Server::Server(BasicController *ctrler, int listen_addr, int listen_port, int max_connection_num)
{
    m_listen_addr = listen_addr;
    m_listen_port = listen_port;
    max_connection_num = max_connection_num;
    m_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (m_sockfd != -1)
    {
        cout << "socket fd:" << m_sockfd << std::endl;
    }
    else
    {
        throw;
    }

    if (ctrler != nullptr)
    {
        m_ctrler = ctrler;
    }
    else
    {
       m_ctrler = new BasicController();
    }
}

Server::~Server()
{
    close(m_sockfd);
    delete m_ctrler;
}

void Server::Init()
{
    sockaddr_in addr;
    memset(&addr, 0, sizeof(sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_listen_port);
    addr.sin_addr.s_addr = htonl(m_listen_addr); //inet_addr("0.0.0.0");

    int keep_alive = 1;
    setsockopt(m_sockfd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keep_alive, sizeof(keep_alive));

    if (bind(m_sockfd, reinterpret_cast<sockaddr *>(&addr), sizeof(sockaddr)) != 0)
    {
        cout << "error bind." << errno << endl;
        throw;
    }
    if (listen(m_sockfd, 10240) != 0)
    {
        cout << "error listen." << errno << endl;
    }
    int connfd;
}

void Server::Accept()
{
    int connfd;
    while (true)
    {
        if ((connfd = accept(m_sockfd, (struct sockaddr *)NULL, NULL)) == -1)
        {
            printf("accpet socket error: %s errno :%d\n", strerror(errno), errno);
            continue;
        }
        printf("accpet socket:%d\n", connfd);
        m_ctrler->AddSocketfd(connfd);
        //Add BasicController here
    }
}