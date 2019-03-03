#include <sys/socket.h>
#include <sys/epoll.h>
#include <memory.h>
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <json/json.h>

using namespace std;

struct ThreadArg
{
    int epoll_fd = 0;
    int connnections = 0;
    int thread_id = 0;
};

struct UserInfo
{
    int user_id = 0;
    int fd = 0;
    bool write_ready = false;
};
unordered_map<int,UserInfo> g_Infos;
int g_mid = 1;

class Thread
{
    pthread_t* m_thread;
    void* (*routine)(void*);
public:
    Thread(void* (*a)(void*),void* arg)
    {
        routine = a;
        m_thread = new pthread_t;
        pthread_create(m_thread,nullptr,routine,arg);
    }
    ~Thread()
    {
        pthread_join(*m_thread,nullptr);
        delete m_thread;
    }
};

void* deal(void* arg)
{
    ThreadArg& targ = *reinterpret_cast<ThreadArg*>(arg);
    printf ("thread: %d created.\n",targ.thread_id);
    int epollfd = targ.epoll_fd;
    epoll_event events[128];
    char buf[1024];
    while(true)
    {
        int nfds = epoll_wait(epollfd, events, 128, -1);
        if (nfds == -1)
        {
            cout << "epoll_wait error." << std::endl;
        }
        for (int i=0;i!=nfds;i++)
        {
            printf ("thread: %d, connections: %d, event: %x\n",targ.thread_id,targ.connnections,events[i].events);
            if ((events[i].events & EPOLLRDHUP) != 0)
            {
                epoll_event ev;
                ev = events[i];
                epoll_ctl(epollfd,EPOLL_CTL_DEL,events[i].data.fd,&ev);
                targ.connnections--;
                cout << "disconnected by clinet." << std::endl;
            }
            else if ((events[i].events & EPOLLIN) != 0)
            {
                cout << "EPOLLIN!" << std::endl;
                int num = recv(events[i].data.fd,buf,1024,0);
                buf[num] = 0;
                if (strcmp(buf,"bye\n") == 0 || strcmp(buf,"bye\r\n") == 0)
                {
                    epoll_event ev;
                    ev = events[i];
                    epoll_ctl(epollfd,EPOLL_CTL_DEL,events[i].data.fd,&ev);
                    close(events[i].data.fd);
                    targ.connnections--;
                    break;
                }
                else
                {
                    Json::Reader reader;
                    Json::Value root;
                    string str(buf);
                    reader.parse(str,root);
                    int id = root["id"].asInt();
                    string messages = root["message"].asString();
                    send(g_Infos[id].fd,messages.c_str(),messages.length(),0);
                }
            }
            else if ((events[i].events & EPOLLOUT) != 0)
            {
                cout << "user_id "<< g_mid << " login." << std::endl;
                g_Infos[g_mid].fd = events[i].data.fd;
                g_Infos[g_mid].user_id = g_mid;
                g_Infos[g_mid].write_ready = true;
                g_mid++;
            }
        }
    }
    return nullptr;
}


int main()
{
    int sock = socket(AF_INET,SOCK_STREAM,0);
    cout << "socket fd:" << sock << std::endl;
    sockaddr_in addr;
    memset(&addr, 0, sizeof(sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(10004);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr("0.0.0.0");
    epoll_event ev;
    
    ThreadArg arg[8];
    for (int i = 0;i!= 8;i++)
    {
        arg[i].epoll_fd = epoll_create1(0);
        arg[i].thread_id = i;
        if (arg[i].epoll_fd == -1)
        {
            cout << "create epoll failed." << errno << endl;
            return 1;
        }
    }
    
    if (bind(sock,reinterpret_cast<sockaddr*>(&addr),sizeof(sockaddr)) != 0)
    {
        cout << "error bind." << errno << endl;
        return 1;
    }
    if (listen(sock,10240) != 0)
    {
        cout << "error listen."<< errno << endl;
    }
    int connfd;
    Thread *threads[8];
    for (int i = 0;i!= 8;i++)
    {
        threads[i] = new Thread(deal,&arg[i].epoll_fd);
    }
    while(true)
    {
        if((connfd = accept(sock,(struct sockaddr*)NULL,NULL)) == -1)
        {
            printf("accpet socket error: %s errno :%d\n",strerror(errno),errno);
            continue;
        }
        ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLOUT;
        ev.data.fd = connfd;
        int idle_thread = 0;
        for (int i = 0; i != 8; i++)
        {
            if (arg[i].connnections < arg[idle_thread].connnections)
                idle_thread = i;
        }
        if (epoll_ctl(arg[idle_thread].epoll_fd, EPOLL_CTL_ADD, connfd, &ev) == -1) 
        {
            cout << "error epoll_ctl."<< errno << endl;
        }
        else
        {
            arg[idle_thread].connnections++;
        }
        
    }
    close(sock);
    return 0;
}