#include <json/json.h>
#include <sys/epoll.h>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

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

    m_mid = 1;
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
    char buf[1024];
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
            if ((events[i].events & EPOLLRDHUP) != 0)
            {
                epoll_event ev;
                ev = events[i];
                epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, &ev);
                cout << "disconnected by clinet." << std::endl;
            }
            else if ((events[i].events & EPOLLIN) != 0)
            {
                cout << "EPOLLIN! thread id:" << thread_id << std::endl;
                int num = recv(events[i].data.fd, buf, 1024, 0);
                buf[num] = 0;
                if (strcmp(buf, "bye\n") == 0 || strcmp(buf, "bye\r\n") == 0)
                {
                    epoll_event &ev = events[i];
                    epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, &ev);
                    close(events[i].data.fd);
                    break;
                }
                else
                {
                    Json::Reader reader;
                    Json::Value root;
                    string str(buf);
                    try
                    {
                        if (!reader.parse(str, root))
                        {
                            cout << "parse failed! thread id:" << thread_id << std::endl;
                            continue;
                        }

                        int id = root["id"].asInt();
                        string messages = root["message"].asString();
                        if (send(m_Infos[id].fd, messages.c_str(), messages.length(), 0) == -1)
                        {
                            if (errno == EWOULDBLOCK)
                            {
                                m_Infos[id].write_ready = false;
                                m_BlockMessages[id].emplace_front(messages);
                            }
                            else
                            {
                                cout << m_Infos[id].fd << " " << id << " error in 107 send errno " << errno << "!" << endl;
                            }
                        }
                    }
                    catch (const std::exception &e)
                    {
                        std::cerr << e.what() << '\n';
                        continue;
                    }
                }
            }
            else if ((events[i].events & EPOLLOUT) != 0)
            {
                if (m_BlockMessages.find(m_FdId[events[i].data.fd]) != m_BlockMessages.end())
                {
                    int id = m_FdId[events[i].data.fd];
                    if (m_Infos[id].write_ready == false)
                    {
                        m_Infos[id].write_ready = true;
                        while (m_BlockMessages[id].begin() != m_BlockMessages[id].end())
                            if (send(m_Infos[id].fd, m_BlockMessages[id].back().c_str(), m_BlockMessages[id].back().length(), 0) == -1)
                            {
                                if (errno == EAGAIN || errno == EWOULDBLOCK)
                                {
                                    m_Infos[id].write_ready = false;
                                    break;
                                }
                                else
                                {
                                    cout << m_Infos[id].fd << " " << id << "error in 131 send errno " << errno << "!" << endl;
                                }
                            }
                            else
                            {
                                m_BlockMessages[id].pop_back();
                            }
                    }
                    else
                    {
                        cout << "error! m_BlockMessages.find(m_FdId[events[i].data.fd]) != m_BlockMessages.end() but m_Infos[m_FdId[events[i].data.fd]].write_ready != false" << endl;
                    }
                }
                else
                {
                    ;
                }
            }
        }
    }
}

void Server::Accept()
{
    epoll_event ev;
    int connfd;
    while (true)
    {
        if ((connfd = accept(m_sockfd, (struct sockaddr *)NULL, NULL)) == -1)
        {
            printf("accpet socket error: %s errno :%d\n", strerror(errno), errno);
            continue;
        }
        ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLOUT;
        ev.data.fd = connfd;
        cout << "user_id " << m_mid << " login." << std::endl;
        m_Infos[m_mid].fd = connfd;
        m_Infos[m_mid].user_id = m_mid;
        m_Infos[m_mid].write_ready = true;
        m_FdId[connfd] = m_mid;
        m_mid++;
        cout << "accept a new connection fd:" << connfd << endl;
        string tmp;
        tmp = "welcome!";
        send(connfd, tmp.c_str(), tmp.length(), 0);
        if (epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, connfd, &ev) == -1)
        {
            cout << "error epoll_ctl." << errno << endl;
        }
    }
}