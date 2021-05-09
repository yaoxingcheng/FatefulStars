#ifndef __COMMON_H__
#define __COMMON_H__

#include <string>
using std::string;

enum ButtonState {
    NORMAL,
    HOVER,
    PRESSED,
};

class Button {
public:
    Button(Game* game, const char* filename);
    ~Button();

    void SetPosition(Vector2 pos);
    void SetHold(bool);

    bool Clicked();
    void Draw();

private:
    Game* game;
    Texture2D texture;
    Vector2 pos;
    bool hold;
    int width;
    int height;
    int state;
};

class TextInput {
public:
    TextInput(Game *game, Rectangle);
    ~TextInput();

    string TextReadyChange();
    string GetText();
    void SetText(string value);
    void SetPlaceholder(string value);
    void SetReadOnly(bool);
    void Draw();

private:
    Game *game;
    Rectangle bounds;
    string value;
    string placeholder;
    bool readOnly;
    bool active;
};

#endif
