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
            system("hakchi uipause");
            system(("xz -dc " + path + "etc/hibernate_mod/1.png.xz > /dev/fb0").c_str());
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
    system(("xz -dc " + path + "etc/hibernate_mod/hibernating.png.xz > /dev/fb0").c_str());

    system("echo Initiating shutdown followed by hibernate mode...");

    system("echo 0 > /sys/devices/virtual/disp/disp/attr/lcd"); //Kill the Screen

    system("echo Initialising hibernation mode...");

    //system("echo Core Temperature at the time of hibernation: $(hakchi hwmon)c");
    system("echo Hibernation mode active! Awaiting input...");
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
            system("echo Shutting down console out from hibernation mode...");
            system("hakchi uiresume && sync && reboot &");
            exit(0);
        }
        else if(pw.buttonPress())
        {
            system("hakchi uiresume &");
            exit(0);
        }
        std::this_thread::sleep_until(nextUpdateTime);
        nextUpdateTime+=fpsTime;
    }
}

void Standby()
{
    system(("xz -dc " + path + "etc/hibernate_mod/hibernating.png.xz > /dev/fb0").c_str());

    system("echo Initiating shutdown followed by standby mode...");

    system("udevadm control --stop-exec-queue"); //Stop execution of events
    system("killall udevd"); //Stop execution of events
    system("modprobe -r clvcon"); // Kill Clvcon -- Don't think this works but doesn't harm shutdown so leave it in just in case.
    system("hakchi uistop"); //Kill everything including retroarch
    system("echo 0 > /sys/devices/virtual/disp/disp/attr/lcd"); //Kill the Screen
    system("[ -b '/dev/sda1' ] && umount /dev/sda1"); //Unmount USB drive -- don't know if this works try it out

    system("echo Initialising standby mode...");

    //system("echo Core Temperature at the time of standby: $(hakchi hwmon)c");
    system("echo Standby mode active! Awaiting input...");
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
    path = argv[0];
    path = path.substr(0, path.find_last_of('/')+1);

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
        system("hakchi uiresume");
    }
}
