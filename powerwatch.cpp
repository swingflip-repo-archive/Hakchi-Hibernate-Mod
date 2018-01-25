#include "powerwatch.h"

#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>

PowerWatch::PowerWatch()
{
    if(auto dir = opendir("/dev/input/"))
    {
        while(auto entry = readdir(dir))
        {
            std::string name(entry->d_name);
            if(name.find("event") != std::string::npos)
            {
                std::ifstream in("/sys/class/input/"+name+"/device/name");
                if(in.good())
                {
                    std::string temp;
                    std::getline(in, temp);
                    in.close();
                    if(temp == "sunxi-knob")
                        powerFd = open(("/dev/input/"+name).c_str(), O_RDONLY | O_NONBLOCK);
                }
            }
        }
        closedir(dir);
    }
    else
    {
        std::cerr << "Cannot open input folder.\n";
        exit(1);
    }

    if(powerFd == -1)
    {
        std::cerr << "Cannot access power button.\n";
        exit(1);
    }
}

PowerWatch::~PowerWatch()
{
    if(powerFd != -1)
        close(powerFd);
}

bool PowerWatch::buttonPress()
{
    static char c[16];
    if(read(powerFd, c, 16) > 0)
        return true;
    return false;
}
