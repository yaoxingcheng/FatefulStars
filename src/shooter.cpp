#include <algorithm>
#include <string>
#include "game.h"
#include "shooter.h"
#include "input.h"
#include "planet.h"
#include <iostream>
#include <cmath>
using namespace std;

Shooter::Shooter(Game* game, ShooterPosition pos, float ball_radius, float ball_dense) : game(game), pos(pos), ball_radius(ball_radius), ball_dense(ball_dense) {
    holded_body = NULL;
    int L = std::min(game->screenWidth, game->screenHeight);
    float cx = 0.5f * L;
    float cy = (pos == UP ? 0.065f : 0.935f) * L;
    shooter_body = CreatePhysicsBodyCircle((Vector2){cx, cy}, game->shooter_radius, game->shooter_dense);
    shooter_body->enabled = false;
    shooter_body->holded = true;
    cursorX = int(0.5f * L);
    energy = 0;
    released = 0;
    next_sides = GetRandomValue(game->min_sides, game->max_sides);
}

Shooter::~Shooter() {

}

float Shooter::getX() {
    int L = std::min(game->screenWidth, game->screenHeight);
    InputController* input = game->GetInput();
    float cx = pos == UP ? input->GetCursorX() : cursorX;
    return cx;
}

float Shooter::getY() {
    int L = std::min(game->screenWidth, game->screenHeight);
    return (pos == UP ? 0.065f : 0.935f) * L;
}

Vector2 Shooter::getHoldPosition() {
    float cx = getX();
    float cy = getY();
    return (Vector2){cx, cy};
}
void Shooter::createNewBody() {
    float cx = getX();
    float cy = getY();
    holded_body = CreatePhysicsBodyPolygon((Vector2){cx, cy}, ball_radius, next_sides, ball_dense);
    if (pos == UP) next_sides = GetRandomValue(game->min_sides, game->max_sides);
    holded_body->enabled = false;
    holded_body->breakable = false;
    holded_body->holded = true;
}

void AddLocalGravity(PhysicsBody body, PhysicsBody anchor, float force){
    if (body != NULL && anchor != NULL){
        body->useGravity = false;
        body->useLocalGravity = true;
        body->anchor = anchor->position;
        body->anchorMass = anchor->mass;
        body->anchorForce = force;
    }
}

void Shooter::Update() {
    if (pos == DOWN && !game->IsMultiPlayer()) return;
    float cx = getX();
    float cy = getY();
    shooter_body->position = (Vector2){cx, cy};
    UpdatePhysics(); 
    if (holded_body == NULL) createNewBody();
    InputController* input = game->GetInput();
    if (pos == UP) input->SetEnergy(std::min(input->GetEnergy(), 500));
    int last_energy = pos == UP ? input->GetEnergy():energy;
    int is_released = pos == UP ? IsMouseButtonReleased(MOUSE_LEFT_BUTTON):released;
    if (is_released != 0) {
        PhysicsBody body = GetPhysicsBodyByID(holded_body->id);
        Planet* planet = game->GetPlanet();
        body->enabled = true;
        float velocity_direction = pos == UP ? 1 : -1;
        body->position.y += (game->shooter_radius + 0.1) * velocity_direction;
        body->velocity = (Vector2){0, velocity_direction * sqrtf(float(energy) * body->inverseMass)};
        std::cout<<"if this holded body is breakable: "<<body->breakable<<std::endl;
        AddLocalGravity(body, planet->GetBody(), game->pull_coef);
        createNewBody();

        body->breakable = true;
        body->holded = false;
    }
    
}

void Shooter::destroyBody() {
    int planet_id = game->GetPlanet()->GetID();
    // Destroy physics bodies that out of boundary
    int bodiesCount = GetPhysicsBodiesCount();
    for (int i = bodiesCount - 1; i >= 0; i--) {
            PhysicsBody body = GetPhysicsBody(i);
            if (!body->enabled) continue;
            bool out_of_box = (body->position.y > game->screenHeight*2 || body->position.y < -1*game->screenHeight || body->position.x > game->screenWidth*2 || body->position.x < game->screenWidth * -1);
            if (body != NULL && out_of_box) DestroyPhysicsBody(body);
        }

}

void Shooter::drawBody() {
    destroyBody();
    int planet_id = game->GetPlanet()->GetID();
    int bodiesCount = GetPhysicsBodiesCount();
    for (int i = 0; i < bodiesCount; i++) {
            PhysicsBody body = GetPhysicsBody(i);
            if (!body->enabled && body->id != holded_body->id) continue;
            if (body->id == holded_body->id) body->position = getHoldPosition();
            if (body != NULL)
            {
                int vertexCount = GetPhysicsShapeVerticesCount(i);
                for (int j = 0; j < vertexCount; j++)
                {
                    // Get physics bodies shape vertices to draw lines
                    // Note: GetPhysicsShapeVertex() already calculates rotation transformations
                    Vector2 vertexA = GetPhysicsShapeVertex(body, j);

                    int jj = (((j + 1) < vertexCount) ? (j + 1) : 0);   // Get next vertex or first to close the shape
                    Vector2 vertexB = GetPhysicsShapeVertex(body, jj);

                    DrawLineEx(vertexA, vertexB, 3, DARKBLUE);     // Draw a line between two vertex positions
                }
            }
        }
}

void Shooter::drawShooter() {
    float cx = getX();
    float cy = getY();
    DrawCircleV((Vector2){cx,cy}, game->shooter_radius, WHITE); 
}

void Shooter::drawText() {
    InputController* input = game->GetInput();
    int L = std::min(game->screenWidth, game->screenHeight);
    if (pos == UP) {
        DrawText(std::to_string(input->GetEnergy()).c_str(), 0.02f * L, 0.02f * L, 48, WHITE);
    } else {
        DrawText(std::to_string(energy).c_str(), 0.98f * L, 0.98f * L, 48, WHITE);
    }
}

void Shooter::Draw() {
    if (pos == DOWN && !game->IsMultiPlayer()) return;
    drawShooter();
    if (pos == UP) drawBody();
    drawText();
}

void Shooter::SetCursorX(int value) {
    cursorX = value;
}

void Shooter::SetEnergy(int value) {
    energy = value;
}

void Shooter::SetReleased(int value) {
    released = value;
}

void Shooter::SetNextSides(int value) {
    next_sides = value;
}