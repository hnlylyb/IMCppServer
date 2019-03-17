#include "IMServer.h"

using namespace std;

int main()
{
    IMServer server;
    server.Init();
    server.Accept();
    return 0;
}