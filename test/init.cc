#include "game.h"

Game::Game() {
}

//Initialize game variables
void Game::InitGame(void)
{
    framesCounter = 0;
    gameOver = false;
    pause = false;
    //fruit = Food{ 0 };
    //snake[0] = { 0 };
    snakePosition[0] = { 0 };
    allowMove = false;
    offset = { 0 };
    counterTail = 0;

    //camera = {0};

    counterTail = 1;
    allowMove = false;

    offset.x = screenWidth%SQUARE_SIZE;
    offset.y = screenHeight%SQUARE_SIZE;

    for (int i = 0; i < SNAKE_LENGTH; i++)
    {
        snake[i].position = (Vector2){ offset.x/2, offset.y/2 };
        snake[i].size = (Vector2){ SQUARE_SIZE, SQUARE_SIZE };
        snake[i].speed = (Vector2){ SQUARE_SIZE, 0 };

        if (i == 0) 
            snake[i].color = DARKBLUE;
        else snake[i].color = BLUE;
    }

    for (int i = 0; i < SNAKE_LENGTH; i++)
    {
        snakePosition[i] = (Vector2){ 0.0f, 0.0f };
    }

    fruit.size = (Vector2){ SQUARE_SIZE, SQUARE_SIZE };
    fruit.color = SKYBLUE;
    fruit.active = false;

    camera.position = (Vector3){0.0f, 10.0f, 10.0f};
    camera.target = (Vector3){1.0f, 0.0f, 0.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
}
