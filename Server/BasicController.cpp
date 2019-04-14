#include <json/json.h>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#include "BasicController.h"

BasicController::BasicController(int thread_num)
{
    m_thread_num = thread_num;
    m_endflag = false;
    m_epoll_fd = epoll_create1(0);
    if (m_epoll_fd != -1)
    {
        ;
    }
    else
    {
        throw;
    }

    for (int i = 0; i != m_thread_num; i++)
    {
        m_threads.emplace_back(new thread(&BasicController::Deal, this, i));
    }
}

BasicController::~BasicController()
{
    for (auto t : m_threads)
    {
        t->join();
        delete t;
    }
}

void BasicController::AddSocketfd(int fd)
{
    epoll_event ev;
    ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLOUT;
    ev.data.fd = fd;
    if (epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1)
    {
        printf("error epoll_ctl. %d\n", errno);
    }
}

void BasicController::Deal(int thread_id)
{
    printf("thread: %d created.\n", thread_id);
    epoll_event events[1024];

    while (!m_endflag)
    {
        int nfds = epoll_wait(m_epoll_fd, events, 1024, 1000);
        if (nfds == -1)
        {
            printf("epoll wait error!\n");
        }
        for (int i = 0; i != nfds; i++)
        {
            printf("thread: %d, event: %x\n", thread_id, events[i].events);
            HandleEvent(events[i]);
        }
    }
}

void BasicController::HandleEvent(epoll_event &event)
{
    char buf[1024];
    if ((event.events & EPOLLRDHUP) != 0)
    {
        epoll_event ev;
        ev = event;
        epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, event.data.fd, &ev);

        HandleEventRDHUP(event);
        printf("disconnected by clinet.\n"); 
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

void BasicController::HandleEventRDHUP(epoll_event &event)
{
}

void BasicController::HandleEventIN(epoll_event &event)
{
    char buf[1024];
    int n = recv(event.data.fd,buf,1024,0);
    send(event.data.fd,buf,n,0);
}

void BasicController::HandleEventOUT(epoll_event &event)
{
    printf("buffer ready\n");
}