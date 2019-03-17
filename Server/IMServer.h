#include "Server.h"

struct UserInfo
{
    int user_id = 0;
    int fd = 0;
    bool write_ready = false;
};

class IMServer : public Server
{
    int m_mid;
    unordered_map<int, UserInfo> m_Infos;
    unordered_map<int, list<string>> m_BlockMessages;
    unordered_map<int, int> m_FdId;

  public:
    IMServer();
    ~IMServer();

  protected:
    virtual void HandleAccept(int connfd);
    virtual void HandleEventRDHUP(epoll_event &event);
    virtual void HandleEventIN(epoll_event &event);
    virtual void HandleEventOUT(epoll_event &event);
};