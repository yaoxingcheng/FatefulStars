#include <algorithm>
#include <string>
#include "game.h"
#include "shooter.h"
#include "input.h"
#include "planet.h"
#include <iostream>
using namespace std;

Shooter::Shooter(Game* game, ShooterPosition pos, float ball_radius, float ball_dense) : game(game), pos(pos), ball_radius(ball_radius), ball_dense(ball_dense) {
    holded_body_id = -1;
}

Shooter::~Shooter() {

}

float Shooter::getX() {
    int L = std::min(game->screenWidth, game->screenHeight);
    InputController* input = game->GetInput();
    float cx = pos == UP ? input->GetCursorX() : 0.5f * L;
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
    cout << "created body at " << cx << ", " << cy << endl;
    PhysicsBody body = CreatePhysicsBodyCircle((Vector2){cx, cy}, ball_radius, ball_dense);
    body->enabled = false;
    holded_body_id = body->id;
}

void AddLocalGravity(PhysicsBody body, PhysicsBody anchor, float force){
    if (body != NULL && anchor != NULL){
        body->useGravity = false;
        body->useLocalGravity = true;
        body->anchor = anchor->position;
        body->anchorForce = force;
    }
}

void Shooter::Update() {
    if (pos == DOWN && !game->IsMultiPlayer()) return;
    UpdatePhysics(); 
    if (pos == UP) {
        InputController* input = game->GetInput();
        input->SetEnergy(std::min(input->GetEnergy(), 500));
        int energy = input->GetEnergy();

        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
            PhysicsBody body = GetPhysicsBody(holded_body_id);
            Planet* planet = game->GetPlanet();
            body->enabled = true;
            float velocity_direction = pos == UP ? 1 : -1;
            body->velocity = (Vector2){0, velocity_direction * sqrtf(float(energy) * body->inverseMass)};
            AddLocalGravity(body, planet->GetBody(), 1);
            createNewBody();
        }
        if (holded_body_id == -1) createNewBody();
    }
    
}

void Shooter::destroyBody() {
    int planet_id = game->GetPlanet()->GetID();
    // Destroy physics bodies that out of boundary
    int bodiesCount = GetPhysicsBodiesCount();
    for (int i = bodiesCount - 1; i >= 0; i--)
        if (i != planet_id) {
            PhysicsBody body = GetPhysicsBody(i);
            bool out_of_box = (body->position.y > game->screenHeight*2 || body->position.y < -1*game->screenHeight || body->position.x > game->screenWidth*2 || body->position.x < game->screenWidth * -1);
            if (out_of_box) {
                cout << "out of box !!!!! " << body->position.x << ", " << body->position.y << endl;
            }
            if (body != NULL && out_of_box) DestroyPhysicsBody(body);
        }
}

void Shooter::drawBody() {
    destroyBody();
    int planet_id = game->GetPlanet()->GetID();
    int bodiesCount = GetPhysicsBodiesCount();
    bodiesCount = GetPhysicsBodiesCount();
    for (int i = 0; i < bodiesCount; i++)
        if (i != planet_id){
            PhysicsBody body = GetPhysicsBody(i);
            if (i == holded_body_id) body->position = getHoldPosition();
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

                    DrawLineV(vertexA, vertexB, DARKBLUE);     // Draw a line between two vertex positions
                }
            }
        }
}

void Shooter::Draw() {
    if (pos == DOWN && !game->IsMultiPlayer()) return;

    int L = std::min(game->screenWidth, game->screenHeight);

    InputController* input = game->GetInput();
    float cx = pos == UP ? input->GetCursorX() : 0.5f * L;
    float startY = (pos == UP ? 0.03f : 0.97f) * L;
    float endY = (pos == UP ? 0.1f : 0.9f) * L;
    DrawLineEx((Vector2){cx - 0.1f * L, startY}, (Vector2){cx - 0.02f * L, endY}, 3, BLACK);
    DrawLineEx((Vector2){cx + 0.1f * L, startY}, (Vector2){cx + 0.02f * L, endY}, 3, BLACK);
    drawBody();

    if (pos == UP) {
        DrawText(std::to_string(input->GetEnergy()).c_str(), 0.88f * L, 0.02f * L, 48, BLACK);
    }
}
