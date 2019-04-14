#include <unordered_map>
#include <string>
#include <list>
#include <thread>
#include <vector>
#include <sys/epoll.h>

#include "BasicController.h"

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
  int m_listen_addr;
  int m_listen_port;
  int m_max_connection_num;
  BasicController* m_ctrler;
  
};