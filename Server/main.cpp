#include "Server.h"

using namespace std;

int main()
{
    Server server;
    server.Init();
    server.Accept();
    return 0;
}