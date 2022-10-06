#ifndef __notifier_h__
#define __notifier_h__

#include <csignal>
#include <cstdlib>
#include <cstring>

#include <string>
#include <thread>
#include <vector>
#include <string_view>
#include <condition_variable>

#include <sys/inotify.h>
#include <unistd.h>
#include <fcntl.h>

class notifier
{

    struct watch_info_t
    {
        std::string_view m_path;
        int m_file_descriptor{-1};
        int m_watch_descriptor{-1};
        bool m_watched_status{false};
    };

public:
    static notifier* instance();
    static void destroy();
    bool add_watch_list(std::vector<std::string_view> _paths);
    int watch();

private:
    notifier();
    ~notifier();
    static int _watch(watch_info_t& _info);
    static void _handle_signal(int _signal);

    static inline notifier* _instance{nullptr};
    static inline std::vector<watch_info_t> m_watched_list{};
    static inline std::vector<std::thread> m_watcher_list{};

    static inline std::condition_variable m_condition_var{};
    static inline std::mutex m_mutex{};
};

#endif