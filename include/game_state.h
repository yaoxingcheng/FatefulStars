#ifndef __GAME_STATE_H__
#define __GAME_STATE_H__

struct GameState {
    int cursorX;
    int energy;
    int num_shot; 
    int next_sides;
    bool finished;
};

#endif
