#include "game.h"
#include "common.h"

const int NUM_FRAMES = 3;

Button::Button(Game* game, const char* textureFile) : game(game) {
    texture = LoadTexture(textureFile);
    width = texture.width;
    height = texture.height / NUM_FRAMES;
    state = NORMAL;
    hold = false;
}

Button::~Button() {
    UnloadTexture(texture);
}

bool Button::Clicked() {
    bool action = false;
    if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){pos.x, pos.y, (float)width, (float)height})) {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            state = PRESSED;
        } else {
            state = HOVER;
        }
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            action = true;
        }
    } else {
        state = NORMAL;
    }
    if (hold) {
        state = PRESSED;
    }
    return action;
}

void Button::SetPosition(Vector2 newPos) {
    pos = newPos;
}

void Button::SetHold(bool value) {
    hold = value;
}

void Button::Draw() {
    DrawTextureRec(texture, (Rectangle){0, height * float(state), (float)width, (float)height}, pos, WHITE);
}


TextInput::TextInput(Game* game, Rectangle bounds) : game(game), bounds(bounds) {
    active = false;
    readOnly = false;
}

TextInput::~TextInput() {

}

string TextInput::GetText() {
    return value;
}

void TextInput::SetText(string newValue) {
    value = newValue;
}

string TextInput::TextReadyChange() {
    if (!readOnly) {
        if (CheckCollisionPointRec(GetMousePosition(), bounds)) {
            if (!active) {
                SetMouseCursor(MOUSE_CURSOR_IBEAM);
            }
            active = true;
        } else {
            if (active) {
                SetMouseCursor(MOUSE_CURSOR_DEFAULT);
            }
            active = false;
        }
    } else {
        if (active) {
            SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        }
        active = false;
    }
    string now = value;
    if (active) {
        int key = GetCharPressed();
        while (key > 0) {
            if (key >= 65000) {
                key -= 65248;
            }
            if ((key >= 32) && (key <= 125)) {
                now += (char)key;
            }
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE) && now.length() > 0) {
            now = now.substr(0, now.length() - 1);
        }
    } else {
        
    }
    return now;
}

void TextInput::SetPlaceholder(string value) {
    placeholder = value;
}

void TextInput::SetReadOnly(bool value) {
    readOnly = value;
}

void TextInput::Draw() {
    DrawRectangleRec(bounds, active ? WHITE : LIGHTGRAY);
    string text = value;
    Color color = BLACK;
    if (text != "" && active) {
        if ((game->framesCounter >> 5) & 1) {
            text += "_";
        }
    }
    if (text == "" && active) {
        text = placeholder;
        color = GRAY;
    }
    if (text != "") {
        DrawText(text.c_str(), bounds.x + 10, int(bounds.y + bounds.height * 0.3), int(bounds.height * 0.65), color);
    }
}
