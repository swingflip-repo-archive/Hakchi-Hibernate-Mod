#include "controller.h"
#include "powerwatch.h"

#include <thread>
#include <chrono>
#include <cstdlib>
#include <unistd.h>
#include <sys/reboot.h>
#include <linux/reboot.h>

Controller c;
PowerWatch pw;
const auto fpsTime = std::chrono::milliseconds(200);
auto nextUpdateTime = std::chrono::system_clock::now()+fpsTime;
std::string path;

int GetState()
{
    for(;;)
    {
        c.Update();
        if(c.PeekButtonStatus(L) && c.PeekButtonStatus(R) && c.GetButtonStatus(UP))
        {
            system("standby DisplayMenu");
            auto fpsTime = std::chrono::milliseconds(33); // Check for input faster
            //system("echo DEBUG: Displaying hibernate menu...");
            for(;;)
            {
                c.Update();
                if(c.PeekButtonStatus(A)) //Accept (Hibernate)
                {
                    //system("echo DEBUG: Accepted hibernate...");
                    return 1;
                }
                else if(c.PeekButtonStatus(X)) //Accept (Standby)
                {
                    //system("echo DEBUG: Accepted standby...");
                    return 2;
                }
                else if(c.PeekButtonStatus(Y)) //Cancel
                {
                    //system("echo DEBUG: Cancelled hibernate...");
                    return 3;
                }
                else if(pw.buttonPress())
                {
                    return 3;
                }
                std::this_thread::sleep_until(nextUpdateTime);
                nextUpdateTime+=fpsTime;
            }
        }
        else
        {
            std::this_thread::sleep_until(nextUpdateTime);
            nextUpdateTime+=fpsTime;
        }
    }
}

void Hibernate()
{
    system("standby Hibernate");

    for(;;)
    {
        c.Update();
        if(c.PeekButtonStatus(L) && c.PeekButtonStatus(R) && c.GetButtonStatus(UP)) //L+R+SELECT = Hibernate / Reboot from Hibernate
        {
            //system("echo Core Temperature at the time of reboot out of hibernation: $(hakchi hwmon)c");
            system("echo 1 > /sys/devices/virtual/disp/disp/attr/lcd");
            break;
        }
        else if(c.PeekButtonStatus(L) && c.PeekButtonStatus(R) && c.GetButtonStatus(DOWN))
        {
            //system("echo Core Temperature at the time of shutdown out of hibernation: $(hakchi hwmon)c");
            system("standby HibernateReboot &");
            exit(0);
        }
        else if(pw.buttonPress())
        {
            system("standby Resume &");
            exit(0);
        }
        std::this_thread::sleep_until(nextUpdateTime);
        nextUpdateTime+=fpsTime;
    }
}

void Standby()
{
    system("standby Standby");

    for(;;)
    {
        c.Update();
        if(c.PeekButtonStatus(L) && c.PeekButtonStatus(R) && c.GetButtonStatus(UP)) //L+R+SELECT = Hibernate / Reboot from Hibernate
        {
            //system("echo Core Temperature at the time of reboot out of standby: $(hakchi hwmon)c");
            system("echo Rebooting console out from standby mode...");
            sync();
            setuid(0);
            reboot(RB_AUTOBOOT);
            exit(0);
        }
        if(c.PeekButtonStatus(L) && c.PeekButtonStatus(R) && c.GetButtonStatus(DOWN))
        {
            //system("echo Core Temperature at the time of shutdown out of standby: $(hakchi hwmon)c");
            system("echo Shutting down console out from standby mode...");
            sync();
            setuid(0);
            reboot(LINUX_REBOOT_CMD_POWER_OFF);
            exit(0);
        }
        std::this_thread::sleep_until(nextUpdateTime);
        nextUpdateTime+=fpsTime;
    }
}

int main(int argc, char * argv[])
{

    for(;;)
    {
        switch(GetState())
        {
        case 1:
            Hibernate();
            break;
        case 2:
            Standby();
            break;
        }
        system("standby Resume");
    }
}
