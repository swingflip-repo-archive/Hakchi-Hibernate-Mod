#ifndef POWERWATCH_H_
#define POWERWATCH_H_

struct PowerWatch
{
    int powerFd = -1;

    PowerWatch();
    ~PowerWatch();
    bool buttonPress();
};
#endif
