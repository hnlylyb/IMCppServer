class Client
{
    int sockfd;

  public:
    Client();
    ~Client();
    void Send(char *buf);
    int Recv(char *buf);
};