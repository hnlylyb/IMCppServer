#include <sys/socket.h>
#include <sys/epoll.h>
#include <memory.h>
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <algorithm>
#include <vector>
#include <list>
#include <unordered_map>
#include <json/json.h>

using namespace std;

struct ThreadArg
{
    int epoll_fd = 0;
    int thread_id = 0;
};

struct UserInfo
{
    int user_id = 0;
    int fd = 0;
    bool write_ready = false;
};

unordered_map<int,UserInfo> g_Infos;
unordered_map<int,list<std::string> > g_BlockMessages;
unordered_map<int,int> g_FdId;

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
            printf ("thread: %d, event: %x\n",targ.thread_id,events[i].events);
            if ((events[i].events & EPOLLRDHUP) != 0)
            {
                epoll_event ev;
                ev = events[i];
                epoll_ctl(epollfd,EPOLL_CTL_DEL,events[i].data.fd,&ev);
                cout << "disconnected by clinet." << std::endl;
            }
            else if ((events[i].events & EPOLLIN) != 0)
            {
                cout << "EPOLLIN! thread id:" << targ.thread_id << std::endl;
                int num = recv(events[i].data.fd,buf,1024,0);
                buf[num] = 0;
                if (strcmp(buf,"bye\n") == 0 || strcmp(buf,"bye\r\n") == 0)
                {
                    epoll_event ev;
                    ev = events[i];
                    epoll_ctl(epollfd,EPOLL_CTL_DEL,events[i].data.fd,&ev);
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
                        if (!reader.parse(str,root))
                        {
                            cout << "parse failed! thread id:" << targ.thread_id << std::endl;
                            continue;
                        }
   

                        int id = root["id"].asInt();
                        string messages = root["message"].asString();
                        if(send(g_Infos[id].fd,messages.c_str(),messages.length(),0) == -1)
                        {
                            if (errno == EWOULDBLOCK)
                            {
                                g_Infos[id].write_ready = false;
                                g_BlockMessages[id].emplace_front(messages);
                            }
                            else
                            {
                                cout << g_Infos[id].fd << " " << id << " error in 107 send errno " << errno << "!" << endl;
                            }
                        }
                    }
                    catch(const std::exception& e)
                    {
                        std::cerr << e.what() << '\n';
                        continue;
                    }
                }
            }
            else if ((events[i].events & EPOLLOUT) != 0)
            {
                if (g_BlockMessages.find(g_FdId[events[i].data.fd]) != g_BlockMessages.end())
                {
                    int id = g_FdId[events[i].data.fd];
                    if (g_Infos[id].write_ready == false)
                    {
                        g_Infos[id].write_ready = true;
                        while (g_BlockMessages[id].begin() != g_BlockMessages[id].end())
                        if(send(g_Infos[id].fd,g_BlockMessages[id].back().c_str(),g_BlockMessages[id].back().length(),0) == -1)
                        {
                            if (errno == EAGAIN || errno == EWOULDBLOCK)
                            {
                                g_Infos[id].write_ready = false;
                                break;
                            }
                            else
                            {
                                cout << g_Infos[id].fd << " " << id << "error in 131 send errno " << errno << "!" << endl;
                            }
                        }
                        else
                        {
                            g_BlockMessages[id].pop_back();
                        }
                        
                    }
                    else
                    {
                        cout << "error! g_BlockMessages.find(g_FdId[events[i].data.fd]) != g_BlockMessages.end() but g_Infos[g_FdId[events[i].data.fd]].write_ready != false" << endl;
                    }
                    
                }
                else
                {
                    ;
                }
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
    int epoll_fd = epoll_create1(0);
    
    ThreadArg arg[8];
    for (int i = 0;i!= 8;i++)
    {
        arg[i].epoll_fd = epoll_fd;
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
        cout << "user_id " << g_mid << " login." << std::endl;
        g_Infos[g_mid].fd = connfd;
        g_Infos[g_mid].user_id = g_mid;
        g_Infos[g_mid].write_ready = true;
        g_FdId[connfd] = g_mid;
        g_mid++;
        cout << "accept a new connection fd:" << connfd << endl;
        string tmp;
        tmp = "welcome!";
        send (connfd,tmp.c_str(),tmp.length(),0);
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connfd, &ev) == -1) 
        {
            cout << "error epoll_ctl."<< errno << endl;
        }       
    }
    close(sock);
    return 0;
}