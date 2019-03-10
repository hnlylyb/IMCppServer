#include <sys/socket.h>
#include <memory.h>
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include <algorithm>
#include <vector>
#include <string>
#include <list>
#include <unordered_map>
#include <json/json.h>

using namespace std;

int main()
{
    Json::Value root;
    root["id"] = 1;
    root["message"] = "hello world\n";
    string json = root.toStyledString();
    int sock = socket(AF_INET,SOCK_STREAM,0);
    sockaddr_in addr;
    memset(&addr, 0, sizeof(sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(10004);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);//inet_addr("127.0.0.1");
    // int fd = 
    connect(sock,reinterpret_cast<sockaddr*>(&addr),sizeof(addr));
    if(send(sock,json.c_str(),json.length(),MSG_DONTWAIT) == -1)
    {
        if (errno == EWOULDBLOCK)
        {
            ;
        }
        else
        {
            cout << "error in 36 send errno " << errno << "!" << endl;
        }
    }
    char buf[1024];
    memset (buf,0,1024);
    while(int n = recv(sock,buf,1024,0))
    {
        if (n > 0)
            cout << buf << endl;
    }
    
    return 0;
}