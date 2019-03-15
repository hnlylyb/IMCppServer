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
  vector<thread*> m_threads;
  unordered_map<int, UserInfo> m_Infos;
  unordered_map<int, list<string>> m_BlockMessages;
  unordered_map<int, int> m_FdId;

public:
  Server(int threadnums);
  ~Server();
  void Accept();
  void Deal(int thread_id);
};