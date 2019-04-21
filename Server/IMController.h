#include "BasicController.h"

struct UserInfo
{
    int user_id = 0;
    int fd = 0;
    bool write_ready = false;
};

class IMController : public BasicController
{
    int m_mid;
    unordered_map<int, UserInfo> m_Infos;
    unordered_map<int, list<string>> m_BlockMessages;
    unordered_map<int, int> m_FdId;

  public:
    IMController(int thread_num = 8);
    virtual ~IMController();
    virtual void AddSocketfd(int fd);

  protected:
    virtual void HandleEventRDHUP(epoll_event &event);
    virtual void HandleEventIN(epoll_event &event);
    virtual void HandleEventOUT(epoll_event &event);
};