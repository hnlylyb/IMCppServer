#include <unordered_map>
#include <string>
#include <list>
#include <thread>
#include <vector>
#include <sys/epoll.h>

using namespace std;

class BasicController;

class Server
{
public:
  Server(BasicController *ctrler, int listen_addr = 0, int listen_port = 10004, int max_connection_num = 10240);
  virtual ~Server();
  virtual void Accept();
  virtual void Init();

protected:
  int m_sockfd;
  int m_listen_addr;
  int m_listen_port;
  int m_max_connection_num;
  BasicController* m_ctrler;
  
};