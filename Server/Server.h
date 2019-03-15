#include <unordered_map>
#include <string>
#include <list>
#include <thread>
#include <vector>

using namespace std;

struct UserInfo
{
  int user_id = 0;
  int fd = 0;
  bool write_ready = false;
};

class Server
{
  int m_mid;
  int m_sockfd;
  int m_epoll_fd;
  int m_thread_num;
  int m_listen_addr;
  int m_listen_port;
  int m_max_connection_num;
  vector<thread *> m_threads;
  unordered_map<int, UserInfo> m_Infos;
  unordered_map<int, list<string>> m_BlockMessages;
  unordered_map<int, int> m_FdId;

public:
  Server(int thread_num = 8, int listen_addr = 0, int listen_port = 10004, int max_connection_num = 10240);
  ~Server();
  void Accept();

  void Init();

protected:
  void Deal(int thread_id);
};