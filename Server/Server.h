#include <unordered_map>
#include <string>
#include <list>
#include <thread>
#include <vector>
#include <sys/epoll.h>

using namespace std;

class Server
{
public:
  Server(int thread_num = 8, int listen_addr = 0, int listen_port = 10004, int max_connection_num = 10240);
  virtual ~Server();
  void Accept();
  void Init();

protected:
  int m_sockfd;
  int m_epoll_fd;
  int m_thread_num;
  int m_listen_addr;
  int m_listen_port;
  int m_max_connection_num;
  vector<thread *> m_threads;
  
  void Deal(int thread_id);
  virtual void HandleEvent(epoll_event &event);
  virtual void HandleAccept(int connfd);
  virtual void HandleEventRDHUP(epoll_event &event);
  virtual void HandleEventIN(epoll_event &event);
  virtual void HandleEventOUT(epoll_event &event);
};