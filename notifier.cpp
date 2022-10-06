#include "notifier.hpp"
#include <numeric>
#include <iostream>
#include <stdexcept>

#define MAX_EVENTS  1024
#define EVENT_SIZE  (sizeof(struct inotify_event))

notifier* notifier::instance()
{
    return _instance == nullptr ? new notifier() : _instance;  
}

void notifier::destroy()
{
    for(size_t index=0; index<m_watched_list.size(); ++index)
    {
        m_watched_list[index].m_watched_status = false;
        inotify_rm_watch(m_watched_list[index].m_file_descriptor, m_watched_list[index].m_watch_descriptor);
        close(m_watched_list[index].m_file_descriptor);
        std::cout << "Inotify Remove Watch for path: " << m_watched_list[index].m_path << "\n";
        m_watcher_list[index].join();
    }
    m_condition_var.notify_all();
}

bool notifier::add_watch_list(std::vector<std::string_view> _paths)
{
    int file_count=0;
    for(const auto& _path : _paths)
    {
        int file_descriptor = inotify_init();
        if (fcntl(file_descriptor, F_SETFL, O_NONBLOCK) < 0)
        {
            ++file_count;
            std::cout << "Inotify API Initialization FAIL for path: " << _path << '\n';
        }
        else
        {
            int watch_descriptor = inotify_add_watch(file_descriptor, _path.data(), 
                                IN_MODIFY | IN_CREATE | IN_DELETE);
            if(watch_descriptor == -1)
            {
                ++file_count;
                close(file_descriptor);
                std::cout << "Inotify Add Watch FAIL for path: " << _path << '\n';
            }
            else
            {
                std::cout << "Added path: " << _path  << "  to watched list\n";
                m_watched_list.emplace_back(watch_info_t{_path, file_descriptor, watch_descriptor, true});
            }
        }
    }

    return file_count == _paths.size() ? false : true;
}

int notifier::watch()
{
    for(auto& _info : m_watched_list)
    {
        std::thread _watcher([&_info]() mutable {
            return notifier::_watch(_info);
        });
        m_watcher_list.emplace_back(std::move(_watcher));
    }

    std::unique_lock<std::mutex> _lock(m_mutex);
    m_condition_var.wait(_lock, [](){
        return !std::accumulate(m_watched_list.begin(), m_watched_list.end(),
                    false, [](bool accumulate, watch_info_t _info){
                    return (accumulate || _info.m_watched_status);
                });
    });
    return 0;
}

int notifier::_watch(watch_info_t& _info)
{
    std::cout << "Observing started for " << _info.m_path << '\n';
    struct inotify_event* _event_list = new struct inotify_event[MAX_EVENTS];

    while(_info.m_watched_status)
    {
        int length = read(_info.m_file_descriptor, _event_list, EVENT_SIZE*MAX_EVENTS);
        for(int i = 0;i<length;) 
        {
            struct inotify_event *event = &_event_list[i];
            if(event->wd == _info.m_watch_descriptor)
            {
                if(event->mask & IN_CREATE)  
                {
                    std::cout<<event->name<<" was created.\n";
                }
                else if(event->mask & IN_DELETE)
                {
                    std::cout<<event->name<<" was deleted.\n";
                }
                else if(event->mask & IN_MODIFY)
                {
                    std::cout<<event->name<<" was modified.\n";
                }
                else if (event->mask & IN_OPEN)
                {
                    std::cout<<event->name<<" was open.\n";
                }
                else if (event->mask & IN_MOVE)
                {
                    std::cout<<event->name<<" was moved.\n";
                }
                else if (event->mask & IN_DELETE_SELF)
                {
                    std::cout<<"Observed file/directory was deleted.\n"
                    "Therefore application will be closed\n";
                    _info.m_watched_status = false;
                    m_condition_var.notify_all();
                }
                else if (event->mask & IN_IGNORED)
                {
                    std::cout<<event->name<<" was ignored.\n";
                    _info.m_watched_status = false;
                    m_condition_var.notify_all();
                }
            }
            i += EVENT_SIZE + event->len;
        }
    }
    delete [] _event_list;
    return 0;
}

notifier::notifier()
{
    //std::signal(SIGINT, notifier::_handle_signal);
}

notifier::~notifier()
{
    delete _instance;
}

void notifier::_handle_signal(int _signal)
{
    std::cout << "SIGINT signal captured to be handled\n";
    notifier::destroy();
    exit(0);
}