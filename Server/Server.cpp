#include <json/json.h>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#include "Server.h"

Server::Server(int thread_num, int listen_addr, int listen_port, int max_connection_num)
{
    m_thread_num = thread_num;
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

    m_epoll_fd = epoll_create1(0);
    if (m_epoll_fd != -1)
    {
        ;
    }
    else
    {
        throw;
    }
}

Server::~Server()
{
    for (auto t : m_threads)
    {
        t->join();
        delete t;
    }
    close(m_sockfd);
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

    int keepIdle = 120;
    int keepInterval = 10;
    int keepCount = 5;
    setsockopt(m_sockfd, SOL_TCP, TCP_KEEPIDLE, (void *)&keepIdle, sizeof(keepIdle));
    setsockopt(m_sockfd, SOL_TCP, TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval));
    setsockopt(m_sockfd, SOL_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount));

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

    for (int i = 0; i != m_thread_num; i++)
    {
        m_threads.emplace_back(new thread(&Server::Deal, this, i));
    }
}

void Server::Deal(int thread_id)
{
    printf("thread: %d created.\n", thread_id);
    epoll_event events[1024];

    while (true)
    {
        int nfds = epoll_wait(m_epoll_fd, events, 1024, -1);
        if (nfds == -1)
        {
            cout << "epoll_wait error." << std::endl;
        }
        for (int i = 0; i != nfds; i++)
        {
            printf("thread: %d, event: %x\n", thread_id, events[i].events);
            HandleEvent(events[i]);
        }
    }
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
        HandleAccept(connfd);
        epoll_event ev;
        ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLOUT;
        ev.data.fd = connfd;
        if (epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, connfd, &ev) == -1)
        {
            cout << "error epoll_ctl." << errno << endl;
        }
    }
}

void Server::HandleAccept(int connfd)
{
}

void Server::HandleEvent(epoll_event &event)
{
    char buf[1024];
    if ((event.events & EPOLLRDHUP) != 0)
    {
        epoll_event ev;
        ev = event;
        epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, event.data.fd, &ev);

        HandleEventRDHUP(event);
        cout << "disconnected by clinet." << std::endl;
    }
    else if ((event.events & EPOLLIN) != 0)
    {
        HandleEventIN(event);
    }
    else if ((event.events & EPOLLOUT) != 0)
    {
        HandleEventOUT(event);
    }
}

void Server::HandleEventRDHUP(epoll_event &event)
{
}

void Server::HandleEventIN(epoll_event &event)
{
}

void Server::HandleEventOUT(epoll_event &event)
{
}