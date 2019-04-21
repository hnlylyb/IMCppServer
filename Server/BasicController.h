#include <unordered_map>
#include <string>
#include <list>
#include <thread>
#include <vector>
#include <sys/epoll.h>

using namespace std;

class BasicController
{
  void Deal(int thread_id);

public:
  BasicController(int thread_num = 8);
  virtual ~BasicController();
  virtual void AddSocketfd(int fd);

protected:
  int m_epoll_fd;
  int m_thread_num;
  bool m_endflag;
  vector<thread *> m_threads;

  virtual void HandleEvent(epoll_event &event);
  virtual void HandleEventRDHUP(epoll_event &event);
  virtual void HandleEventIN(epoll_event &event);
  virtual void HandleEventOUT(epoll_event &event);
};