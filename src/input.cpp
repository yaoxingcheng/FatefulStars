#include "game.h"
#include "input.h"

InputController::InputController(Game* game) : game(game) {}

InputController::~InputController() {

}

void InputController::Update() {
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        energy ++;
    }
}

int InputController::GetEnergy() {
    return energy;
}

void InputController::SetEnergy(int value) {
    energy = value;
}

int InputController::GetCursorX() {
    return GetMouseX();
}
