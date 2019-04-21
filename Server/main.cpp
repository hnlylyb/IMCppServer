#include "Server.h"
#include "IMController.h"

using namespace std;

int main()
{
    Server server(new IMController());
    server.Init();
    server.Accept();
    return 0;
}