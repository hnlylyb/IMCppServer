#include "Server.h"

using namespace std;

int main()
{
    Server server(8);
    server.Accept();
    return 0;
}