#include "Thread.h"

Thread::Thread(void *(*a)(void *), void *arg)
{
    routine = a;
    m_thread = new pthread_t;
    pthread_create(m_thread, nullptr, routine, arg);
}

Thread::~Thread()
{
    pthread_join(*m_thread, nullptr);
    delete m_thread;
}

void Thread::Join()
{
    pthread_join(*m_thread, nullptr);
}