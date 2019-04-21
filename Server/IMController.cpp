#include <json/json.h>
#include <iostream>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>

#include "IMController.h"

IMController::IMController(int threadnum) : BasicController(threadnum)
{
    m_mid = 1;
}

IMController::~IMController() {}

void IMController::HandleEventRDHUP(epoll_event &event)
{
    m_Infos.erase(m_FdId[event.data.fd]);
    m_FdId.erase(event.data.fd);
}

void IMController::HandleEventIN(epoll_event &event)
{
    char buf[1024];
    int num = recv(event.data.fd, buf, 1024, 0);
    buf[num] = 0;

    Json::Reader reader;
    Json::Value root;
    string str(buf);
    try
    {
        if (!reader.parse(str, root))
        {
            cout << "parse failed!" << std::endl;
            return;
        }

        int id = root["id"].asInt();
        string messages = root["message"].asString();
        if (m_Infos.find(id) == m_Infos.end())
        {
            string error = "invalid target id.";
            send(event.data.fd, error.c_str(), error.length(), 0);
        }
        else
        {
            if (send(m_Infos[id].fd, messages.c_str(), messages.length(), 0) == -1)
            {
                if (errno == EWOULDBLOCK)
                {
                    m_Infos[id].write_ready = false;
                    m_BlockMessages[id].emplace_front(messages);
                }
                else
                {
                    cout << m_Infos[id].fd << " " << id << " error in " << __LINE__ << "send errno " << errno << "!" << endl;
                }
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return;
    }
}

void IMController::HandleEventOUT(epoll_event &event)
{
    if (m_BlockMessages.find(m_FdId[event.data.fd]) != m_BlockMessages.end())
    {
        int id = m_FdId[event.data.fd];
        if (m_Infos[id].write_ready == false)
        {
            m_Infos[id].write_ready = true;
            while (m_BlockMessages[id].begin() != m_BlockMessages[id].end())
                if (send(m_Infos[id].fd, m_BlockMessages[id].back().c_str(), m_BlockMessages[id].back().length(), 0) == -1)
                {
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                    {
                        m_Infos[id].write_ready = false;
                        break;
                    }
                    else
                    {
                        cout << m_Infos[id].fd << " " << id << "error in " << __LINE__ << "send errno " << errno << "!" << endl;
                    }
                }
                else
                {
                    m_BlockMessages[id].pop_back();
                }
        }
        else
        {
            cout << "error! m_BlockMessages.find(m_FdId[event.data.fd]) != m_BlockMessages.end() but m_Infos[m_FdId[event.data.fd]].write_ready != false" << endl;
        }
    }
    else
    {
        ;
    }
}

void IMController::AddSocketfd(int fd)
{
    BasicController::AddSocketfd(fd);
    cout << "user_id " << m_mid << " login." << std::endl;
    m_Infos[m_mid].fd = fd;
    m_Infos[m_mid].user_id = m_mid;
    m_Infos[m_mid].write_ready = true;
    m_FdId[fd] = m_mid;
    m_mid++;
    cout << "accept a new connection fd:" << fd << endl;
    string tmp;
    tmp = "welcome!";
    send(fd, tmp.c_str(), tmp.length(), 0);
}