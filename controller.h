#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include <map>

enum GameButton : uint16_t { A=304, B=305, X=307, Y=308, L=310, R=311, SELECT=314, START=315, LEFT=704, RIGHT=705, UP=706, DOWN=707  };

struct ButtonEvent
{
    char unk0[8];
    unsigned short unk1;
    GameButton button;
    unsigned short pressed;
    unsigned short zero;    
};

class Controller
{
private:
    int fd = -1;
    ButtonEvent buttonBuffer[10];
    std::map<GameButton,bool> buttons;
public:
    Controller();
    ~Controller();
    bool PeekButtonStatus(GameButton button);
    bool GetButtonStatus(GameButton button);
    void Update();
};

#endif
