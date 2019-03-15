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
#include <thread>

using namespace std;

int sock;
bool run = true;
void reveiver()
{
    char buf[1024];
    memset(buf, 0, 1024);
    while (run)
    {
        int n = recv(sock, buf, 1024, 0);
        if (n > 0)
            cout << buf << endl;
        memset(buf, 0, n);
    }
}

int main()
{

    sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;
    memset(&addr, 0, sizeof(sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(10004);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); //inet_addr("127.0.0.1");
    connect(sock, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
    string input;
    std::thread thr(reveiver);
    while (cin >> input)
    {
        Json::Value root;
        root["id"] = 1;
        root["message"] = input;
        string json = root.toStyledString();
        if (send(sock, json.c_str(), json.length(), 0) == -1)
        {
            if (errno == EWOULDBLOCK)
            {
                ;
            }
            else
            {
                cout << "error in send errno " << errno << "!" << endl;
            }
        }
        input.clear();
    }
    run = false;
    close(sock);
    thr.join();

    return 0;
}