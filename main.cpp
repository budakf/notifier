#include <iostream>
#include "notifier.hpp"

std::vector<std::string_view> get_list(int argc, char** argv)
{
    std::vector<std::string_view> path_list;
    for(int  index=1; index<argc; ++index)
    {
        path_list.push_back(argv[index]);
    }
    return path_list;
}

int main(int argc, char** argv)
{
    if(argc>=2)
    {
        auto path_list = get_list(argc,argv);
        auto _notifier = notifier::instance();
        if(_notifier->add_watch_list(path_list))
        {
            _notifier->watch();
        }
        notifier::destroy();
    }
    else
    {
        std::cout << "Usage:\n\t./notifier $FILE_PATH_1 $FILE_PATH_2 ...\n";
    }
    return 0;    
}
