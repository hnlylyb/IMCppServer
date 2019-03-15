#include <pthread.h>

class Thread
{
    pthread_t* m_thread;
    void* (*routine)(void*);
public:
    Thread(void* (*a)(void*),void* arg);
    ~Thread();
};