#ifndef __INPUT_H__
#define __INPUT_H__

class InputController {
public:
    InputController(Game* game);
    ~InputController();

    void Update();

    int GetCursorX();
    int GetEnergy();
    void SetEnergy(int value);

private:
    Game* game;
    int energy;
};

#endif
