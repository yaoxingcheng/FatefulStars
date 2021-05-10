#include "raylib.h"
#include "physac.h"
#include <stdio.h>
#include "game.h"
#include "planet.h"
#include "shooter.h"

// Support TRACELOG macros
#if defined(PHYSAC_DEBUG)
    #include <stdio.h>              // Required for: printf()
    #define TRACELOG(...) printf(__VA_ARGS__)
#else
    #define TRACELOG(...) (void)0;
#endif

#include <stdlib.h>                 // Required for: malloc(), calloc(), free()
#include <math.h>                   // Required for: cosf(), sinf(), fabs(), sqrtf()

#if !defined(PHYSAC_AVOID_TIMMING_SYSTEM)
    // Time management functionality
    #include <time.h>                   // Required for: time(), clock_gettime()
    #if defined(_WIN32)
        // Functions required to query time on Windows
        int __stdcall QueryPerformanceCounter(unsigned long long int *lpPerformanceCount);
        int __stdcall QueryPerformanceFrequency(unsigned long long int *lpFrequency);
    #endif
    #if defined(__linux__) || defined(__FreeBSD__)
        #if _POSIX_C_SOURCE < 199309L
            #undef _POSIX_C_SOURCE
            #define _POSIX_C_SOURCE 199309L // Required for CLOCK_MONOTONIC if compiled with c99 without gnu ext.
        #endif
        #include <sys/time.h>           // Required for: timespec
    #endif
    #if defined(__APPLE__)              // macOS also defines __MACH__
        #include <mach/mach_time.h>     // Required for: mach_absolute_time()
    #endif
#endif

// NOTE: MSVC C++ compiler does not support compound literals (C99 feature)
// Plain structures in C++ (without constructors) can be initialized from { } initializers.
#if defined(__cplusplus)
    #define CLITERAL(type)      type
#else
    #define CLITERAL(type)      (type)
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define PHYSAC_MIN(a,b)         (((a)<(b))?(a):(b))
#define PHYSAC_MAX(a,b)         (((a)>(b))?(a):(b))
#define PHYSAC_FLT_MAX          3.402823466e+38f
#define PHYSAC_EPSILON          0.000001f
#define PHYSAC_K                1.0f/3.0f
#define PHYSAC_VECTOR_ZERO      CLITERAL(Vector2){ 0.0f, 0.0f }

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static double deltaTime = 1.0/60.0/10.0 * 1000;             // Delta time in milliseconds used for physics steps

#if !defined(PHYSAC_AVOID_TIMMING_SYSTEM)
// Time measure variables
static double baseClockTicks = 0.0;                         // Offset clock ticks for MONOTONIC clock
static unsigned long long int frequency = 0;                // Hi-res clock frequency
static double startTime = 0.0;                              // Start time in milliseconds
static double currentTime = 0.0;                            // Current time in milliseconds
#endif

// Physics system configuration
static PhysicsBody bodies[PHYSAC_MAX_BODIES];               // Physics bodies pointers array
static unsigned int physicsBodiesCount = 0;                 // Physics world current bodies counter
static PhysicsManifold contacts[PHYSAC_MAX_MANIFOLDS];      // Physics bodies pointers array
static unsigned int physicsManifoldsCount = 0;              // Physics world current manifolds counter

static Vector2 gravityForce = { 0.0f, 9.81f };              // Physics world gravity force

// Utilities variables
static unsigned int usedMemory = 0;                         // Total allocated dynamic memory

//----------------------------------------------------------------------------------
// Module Internal Functions Declaration
//----------------------------------------------------------------------------------
#if !defined(PHYSAC_AVOID_TIMMING_SYSTEM)
// Timming measure functions
static void InitTimer(void);                                                                                // Initializes hi-resolution MONOTONIC timer
static unsigned long long int GetClockTicks(void);                                                          // Get hi-res MONOTONIC time measure in mseconds
static double GetCurrentTime(void);                                                                         // Get current time measure in milliseconds
#endif

static void UpdatePhysicsStep(void);                                                                        // Update physics step (dynamics, collisions and position corrections)

static int FindAvailableBodyIndex();                                                                        // Finds a valid index for a new physics body initialization
static int FindAvailableManifoldIndex();                                                                    // Finds a valid index for a new manifold initialization
static PhysicsVertexData CreateDefaultPolygon(float radius, int sides);                                     // Creates a random polygon shape with max vertex distance from polygon pivot
static PhysicsVertexData CreateRectanglePolygon(Vector2 pos, Vector2 size);                                 // Creates a rectangle polygon shape based on a min and max positions

static void InitializePhysicsManifolds(PhysicsManifold manifold);                                           // Initializes physics manifolds to solve collisions
static PhysicsManifold CreatePhysicsManifold(PhysicsBody a, PhysicsBody b);                                 // Creates a new physics manifold to solve collision
static void DestroyPhysicsManifold(PhysicsManifold manifold);                                               // Unitializes and destroys a physics manifold

static void SolvePhysicsManifold(PhysicsManifold manifold);                                                 // Solves a created physics manifold between two physics bodies
static void SolveCircleToCircle(PhysicsManifold manifold);                                                  // Solves collision between two circle shape physics bodies
static void SolveCircleToPolygon(PhysicsManifold manifold);                                                 // Solves collision between a circle to a polygon shape physics bodies
static void SolvePolygonToCircle(PhysicsManifold manifold);                                                 // Solves collision between a polygon to a circle shape physics bodies
static void SolvePolygonToPolygon(PhysicsManifold manifold);                                                // Solves collision between two polygons shape physics bodies
static void IntegratePhysicsForces(PhysicsBody body);                                                       // Integrates physics forces into velocity
static void IntegratePhysicsVelocity(PhysicsBody body);                                                     // Integrates physics velocity into position and forces
static void IntegratePhysicsImpulses(PhysicsManifold manifold);                                             // Integrates physics collisions impulses to solve collisions
static void CorrectPhysicsPositions(PhysicsManifold manifold);                                              // Corrects physics bodies positions based on manifolds collision information
static void FindIncidentFace(Vector2 *v0, Vector2 *v1, PhysicsShape ref, PhysicsShape inc, int index);      // Finds two polygon shapes incident face
static float FindAxisLeastPenetration(int *faceIndex, PhysicsShape shapeA, PhysicsShape shapeB);            // Finds polygon shapes axis least penetration

// Math required functions
static Vector2 MathVector2Product(Vector2 vector, float value);                                             // Returns the product of a vector and a value
static float MathVector2CrossProduct(Vector2 v1, Vector2 v2);                                               // Returns the cross product of two vectors
static float MathVector2SqrLen(Vector2 vector);                                                             // Returns the len square root of a vector
static float MathVector2DotProduct(Vector2 v1, Vector2 v2);                                                 // Returns the dot product of two vectors
static inline float MathVector2SqrDistance(Vector2 v1, Vector2 v2);                                         // Returns the square root of distance between two vectors
static void MathVector2Normalize(Vector2 *vector);                                                          // Returns the normalized values of a vector
static Vector2 MathVector2Add(Vector2 v1, Vector2 v2);                                                      // Returns the sum of two given vectors
static Vector2 MathVector2Subtract(Vector2 v1, Vector2 v2);                                                 // Returns the subtract of two given vectors
static Matrix2x2 MathMatFromRadians(float radians);                                                         // Returns a matrix 2x2 from a given radians value
static inline Matrix2x2 MathMatTranspose(Matrix2x2 matrix);                                                 // Returns the transpose of a given matrix 2x2
static inline Vector2 MathMatVector2Product(Matrix2x2 matrix, Vector2 vector);                              // Returns product between matrix 2x2 and vector
static int MathVector2Clip(Vector2 normal, Vector2 *faceA, Vector2 *faceB, float clip);                     // Returns clipping value based on a normal and two faces
static Vector2 MathTriangleBarycenter(Vector2 v1, Vector2 v2, Vector2 v3);                                  // Returns the barycenter of a triangle given by 3 points

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Initializes physics values, pointers and creates physics loop thread
PHYSACDEF void InitPhysics(void)
{
#if !defined(PHYSAC_AVOID_TIMMING_SYSTEM)
    // Initialize high resolution timer
    InitTimer();
#endif

    TRACELOG("[PHYSAC] Physics module initialized successfully\n");
}

// Sets physics global gravity force
PHYSACDEF void SetPhysicsGravity(float x, float y)
{
    gravityForce.x = x;
    gravityForce.y = y;
}

// Creates a new circle physics body with generic parameters
PHYSACDEF PhysicsBody CreatePhysicsBodyCircle(Vector2 pos, float radius, float density)
{
    PhysicsBody body = CreatePhysicsBodyPolygon(pos, radius, PHYSAC_DEFAULT_CIRCLE_VERTICES, density);
    return body;
}

// Creates a new rectangle physics body with generic parameters
PHYSACDEF PhysicsBody CreatePhysicsBodyRectangle(Vector2 pos, float width, float height, float density)
{
    // NOTE: Make sure body data is initialized to 0
    PhysicsBody body = (PhysicsBody)PHYSAC_CALLOC(sizeof(PhysicsBodyData), 1);
    usedMemory += sizeof(PhysicsBodyData);

    int id = FindAvailableBodyIndex();
    if (id != -1)
    {
        // Initialize new body with generic values
        body->id = id;
        body->enabled = true;
        body->position = pos;
        body->sides = 4;
        body->density = density;
        body->shape.type = PHYSICS_POLYGON;
        body->shape.body = body;
        body->shape.transform = MathMatFromRadians(0.0f);
        body->shape.vertexData = CreateRectanglePolygon(pos, CLITERAL(Vector2){ width, height });

        // Calculate centroid and moment of inertia
        Vector2 center = { 0.0f, 0.0f };
        float area = 0.0f;
        float inertia = 0.0f;

        for (unsigned int i = 0; i < body->shape.vertexData.vertexCount; i++)
        {
            // Triangle vertices, third vertex implied as (0, 0)
            Vector2 p1 = body->shape.vertexData.positions[i];
            unsigned int nextIndex = (((i + 1) < body->shape.vertexData.vertexCount) ? (i + 1) : 0);
            Vector2 p2 = body->shape.vertexData.positions[nextIndex];

            float D = MathVector2CrossProduct(p1, p2);
            float triangleArea = D/2;

            area += triangleArea;

            // Use area to weight the centroid average, not just vertex position
            center.x += triangleArea*PHYSAC_K*(p1.x + p2.x);
            center.y += triangleArea*PHYSAC_K*(p1.y + p2.y);

            float intx2 = p1.x*p1.x + p2.x*p1.x + p2.x*p2.x;
            float inty2 = p1.y*p1.y + p2.y*p1.y + p2.y*p2.y;
            inertia += (0.25f*PHYSAC_K*D)*(intx2 + inty2);
        }

        center.x *= 1.0f/area;
        center.y *= 1.0f/area;

        // Translate vertices to centroid (make the centroid (0, 0) for the polygon in model space)
        // Note: this is not really necessary
        for (unsigned int i = 0; i < body->shape.vertexData.vertexCount; i++)
        {
            body->shape.vertexData.positions[i].x -= center.x;
            body->shape.vertexData.positions[i].y -= center.y;
        }

        body->mass = density*area;
        body->inverseMass = ((body->mass != 0.0f) ? 1.0f/body->mass : 0.0f);
        body->inertia = density*inertia;
        body->inverseInertia = ((body->inertia != 0.0f) ? 1.0f/body->inertia : 0.0f);
        body->staticFriction = 0.4f;
        body->dynamicFriction = 0.2f;
        body->restitution = 0.0f;
        body->useGravity = true;
        body->isGrounded = false;
        body->freezeOrient = false;

        // Add new body to bodies pointers array and update bodies count
        bodies[physicsBodiesCount] = body;
        physicsBodiesCount++;

        TRACELOG("[PHYSAC] Physic body created successfully (id: %i)\n", body->id);
    }
    else TRACELOG("[PHYSAC] Physic body could not be created, PHYSAC_MAX_BODIES reached\n");

    return body;
}

// Creates a new polygon physics body with generic parameters
PHYSACDEF PhysicsBody CreatePhysicsBodyPolygon(Vector2 pos, float radius, int sides, float density)
{
    PhysicsBody body = (PhysicsBody)PHYSAC_MALLOC(sizeof(PhysicsBodyData));
    usedMemory += sizeof(PhysicsBodyData);

    int id = FindAvailableBodyIndex();
    if (id != -1)
    {
        // Initialize new body with generic values
        body->id = id;
        body->density = density;
        body->sides = sides;
        body->enabled = true;
        body->position = pos;
        body->velocity = PHYSAC_VECTOR_ZERO;
        body->force = PHYSAC_VECTOR_ZERO;
        body->angularVelocity = 0.0f;
        body->torque = 0.0f;
        body->orient = 0.0f;
        body->shape.type = PHYSICS_POLYGON;
        body->shape.body = body;
        body->shape.radius = radius;
        body->shape.transform = MathMatFromRadians(0.0f);
        body->shape.vertexData = CreateDefaultPolygon(radius, sides);
        body->newlyadded = true;
        body->breakable = true;

        // Calculate centroid and moment of inertia
        Vector2 center = { 0.0f, 0.0f };
        float area = 0.0f;
        float inertia = 0.0f;

        for (unsigned int i = 0; i < body->shape.vertexData.vertexCount; i++)
        {
            // Triangle vertices, third vertex implied as (0, 0)
            Vector2 position1 = body->shape.vertexData.positions[i];
            unsigned int nextIndex = (((i + 1) < body->shape.vertexData.vertexCount) ? (i + 1) : 0);
            Vector2 position2 = body->shape.vertexData.positions[nextIndex];

            float cross = MathVector2CrossProduct(position1, position2);
            float triangleArea = cross/2;

            area += triangleArea;

            // Use area to weight the centroid average, not just vertex position
            center.x += triangleArea*PHYSAC_K*(position1.x + position2.x);
            center.y += triangleArea*PHYSAC_K*(position1.y + position2.y);

            float intx2 = position1.x*position1.x + position2.x*position1.x + position2.x*position2.x;
            float inty2 = position1.y*position1.y + position2.y*position1.y + position2.y*position2.y;
            inertia += (0.25f*PHYSAC_K*cross)*(intx2 + inty2);
        }

        center.x *= 1.0f/area;
        center.y *= 1.0f/area;

        // Translate vertices to centroid (make the centroid (0, 0) for the polygon in model space)
        // Note: this is not really necessary
        for (unsigned int i = 0; i < body->shape.vertexData.vertexCount; i++)
        {
            body->shape.vertexData.positions[i].x -= center.x;
            body->shape.vertexData.positions[i].y -= center.y;
        }

        body->mass = density*area;
        body->inverseMass = ((body->mass != 0.0f) ? 1.0f/body->mass : 0.0f);
        body->inertia = density*inertia;
        body->inverseInertia = ((body->inertia != 0.0f) ? 1.0f/body->inertia : 0.0f);
        body->staticFriction = 0.4f;
        body->dynamicFriction = 0.2f;
        body->restitution = 0.0f;
        body->useGravity = true;
        body->isGrounded = false;
        body->freezeOrient = false;

        // Add new body to bodies pointers array and update bodies count
        bodies[physicsBodiesCount] = body;
        physicsBodiesCount++;

        TRACELOG("[PHYSAC] Physic body created successfully (id: %i)\n", body->id);
    }
    else TRACELOG("[PHYSAC] Physics body could not be created, PHYSAC_MAX_BODIES reached\n");

    body->tobeDeleted = false;
    return body;
}

// Adds a force to a physics body
PHYSACDEF void PhysicsAddForce(PhysicsBody body, Vector2 force)
{
    if (body != NULL) body->force = MathVector2Add(body->force, force);
}

// Adds an angular force to a physics body
PHYSACDEF void PhysicsAddTorque(PhysicsBody body, float amount)
{
    if (body != NULL) body->torque += amount;
}

// Shatters a polygon shape physics body to little physics bodies with explosion force
PHYSACDEF void PhysicsShatter(PhysicsBody body, Vector2 position, float force)
{
    if (body != NULL)
    {
        if (body->shape.type == PHYSICS_POLYGON)
        {
            PhysicsVertexData vertexData = body->shape.vertexData;
            bool collision = false;

            for (unsigned int i = 0; i < vertexData.vertexCount; i++)
            {
                Vector2 positionA = body->position;
                Vector2 positionB = MathMatVector2Product(body->shape.transform, MathVector2Add(body->position, vertexData.positions[i]));
                unsigned int nextIndex = (((i + 1) < vertexData.vertexCount) ? (i + 1) : 0);
                Vector2 positionC = MathMatVector2Product(body->shape.transform, MathVector2Add(body->position, vertexData.positions[nextIndex]));

                // Check collision between each triangle
                float alpha = ((positionB.y - positionC.y)*(position.x - positionC.x) + (positionC.x - positionB.x)*(position.y - positionC.y))/
                              ((positionB.y - positionC.y)*(positionA.x - positionC.x) + (positionC.x - positionB.x)*(positionA.y - positionC.y));

                float beta = ((positionC.y - positionA.y)*(position.x - positionC.x) + (positionA.x - positionC.x)*(position.y - positionC.y))/
                             ((positionB.y - positionC.y)*(positionA.x - positionC.x) + (positionC.x - positionB.x)*(positionA.y - positionC.y));

                float gamma = 1.0f - alpha - beta;

                if ((alpha > 0.0f) && (beta > 0.0f) & (gamma > 0.0f))
                {
                    collision = true;
                    break;
                }
            }

            if (collision)
            {
                int count = vertexData.vertexCount;
                Vector2 bodyPos = body->position;
                Vector2 *vertices = (Vector2 *)PHYSAC_MALLOC(sizeof(Vector2)*count);
                Matrix2x2 trans = body->shape.transform;
                for (int i = 0; i < count; i++) vertices[i] = vertexData.positions[i];

                // Destroy shattered physics body
                DestroyPhysicsBody(body);

                for (int i = 0; i < count; i++)
                {
                    int nextIndex = (((i + 1) < count) ? (i + 1) : 0);
                    Vector2 center = MathTriangleBarycenter(vertices[i], vertices[nextIndex], PHYSAC_VECTOR_ZERO);
                    center = MathVector2Add(bodyPos, center);
                    Vector2 offset = MathVector2Subtract(center, bodyPos);

                    PhysicsBody body = CreatePhysicsBodyPolygon(center, 10, 3, 10);     // Create polygon physics body with relevant values

                    PhysicsVertexData vertexData = { 0 };
                    vertexData.vertexCount = 3;

                    vertexData.positions[0] = MathVector2Subtract(vertices[i], offset);
                    vertexData.positions[1] = MathVector2Subtract(vertices[nextIndex], offset);
                    vertexData.positions[2] = MathVector2Subtract(position, center);

                    // Separate vertices to avoid unnecessary physics collisions
                    vertexData.positions[0].x *= 0.95f;
                    vertexData.positions[0].y *= 0.95f;
                    vertexData.positions[1].x *= 0.95f;
                    vertexData.positions[1].y *= 0.95f;
                    vertexData.positions[2].x *= 0.95f;
                    vertexData.positions[2].y *= 0.95f;

                    // Calculate polygon faces normals
                    for (unsigned int j = 0; j < vertexData.vertexCount; j++)
                    {
                        unsigned int nextVertex = (((j + 1) < vertexData.vertexCount) ? (j + 1) : 0);
                        Vector2 face = MathVector2Subtract(vertexData.positions[nextVertex], vertexData.positions[j]);

                        vertexData.normals[j] = CLITERAL(Vector2){ face.y, -face.x };
                        MathVector2Normalize(&vertexData.normals[j]);
                    }

                    // Apply computed vertex data to new physics body shape
                    body->shape.vertexData = vertexData;
                    body->shape.transform = trans;

                    // Calculate centroid and moment of inertia
                    center = PHYSAC_VECTOR_ZERO;
                    float area = 0.0f;
                    float inertia = 0.0f;

                    for (unsigned int j = 0; j < body->shape.vertexData.vertexCount; j++)
                    {
                        // Triangle vertices, third vertex implied as (0, 0)
                        Vector2 p1 = body->shape.vertexData.positions[j];
                        unsigned int nextVertex = (((j + 1) < body->shape.vertexData.vertexCount) ? (j + 1) : 0);
                        Vector2 p2 = body->shape.vertexData.positions[nextVertex];

                        float D = MathVector2CrossProduct(p1, p2);
                        float triangleArea = D/2;

                        area += triangleArea;

                        // Use area to weight the centroid average, not just vertex position
                        center.x += triangleArea*PHYSAC_K*(p1.x + p2.x);
                        center.y += triangleArea*PHYSAC_K*(p1.y + p2.y);

                        float intx2 = p1.x*p1.x + p2.x*p1.x + p2.x*p2.x;
                        float inty2 = p1.y*p1.y + p2.y*p1.y + p2.y*p2.y;
                        inertia += (0.25f*PHYSAC_K*D)*(intx2 + inty2);
                    }

                    center.x *= 1.0f/area;
                    center.y *= 1.0f/area;

                    body->mass = area;
                    body->inverseMass = ((body->mass != 0.0f) ? 1.0f/body->mass : 0.0f);
                    body->inertia = inertia;
                    body->inverseInertia = ((body->inertia != 0.0f) ? 1.0f/body->inertia : 0.0f);

                    // Calculate explosion force direction
                    Vector2 pointA = body->position;
                    Vector2 pointB = MathVector2Subtract(vertexData.positions[1], vertexData.positions[0]);
                    pointB.x /= 2.0f;
                    pointB.y /= 2.0f;
                    Vector2 forceDirection = MathVector2Subtract(MathVector2Add(pointA, MathVector2Add(vertexData.positions[0], pointB)), body->position);
                    MathVector2Normalize(&forceDirection);
                    forceDirection.x *= force;
                    forceDirection.y *= force;

                    // Apply force to new physics body
                    PhysicsAddForce(body, forceDirection);
                }

                PHYSAC_FREE(vertices);
            }
        }
    }
    else TRACELOG("[PHYSAC] WARNING: PhysicsShatter: NULL physic body\n");
}

// Returns the current amount of created physics bodies
PHYSACDEF int GetPhysicsBodiesCount(void)
{
    return physicsBodiesCount;
}

// Returns a physics body of the bodies pool at a specific index
PHYSACDEF PhysicsBody GetPhysicsBodyByID(int index)
{
    PhysicsBody body = NULL;
    int id = -1;
    for (unsigned int i = 0; i < physicsBodiesCount; i++)
    {
        if (bodies[i]->id == index)
        {
            id = i;
            break;
        }
    }

    if (id >= 0)
    {
        body = bodies[id];

        if (body == NULL) TRACELOG("[PHYSAC] WARNING: GetPhysicsBody: NULL physic body\n");
    }
    else TRACELOG("[PHYSAC] WARNING: Physic body index is out of bounds\n");

    return body;
}

// Returns a physics body of the bodies pool at a specific index
PHYSACDEF PhysicsBody GetPhysicsBody(int index)
{
    PhysicsBody body = NULL;

    if (index < (int)physicsBodiesCount)
    {
        body = bodies[index];

        if (body == NULL) TRACELOG("[PHYSAC] WARNING: GetPhysicsBody: NULL physic body\n");
    }
    else TRACELOG("[PHYSAC] WARNING: Physic body index is out of bounds\n");

    return body;
}

// Returns the physics body shape type (PHYSICS_CIRCLE or PHYSICS_POLYGON)
PHYSACDEF int GetPhysicsShapeType(int index)
{
    int result = -1;

    if (index < (int)physicsBodiesCount)
    {
        PhysicsBody body = bodies[index];

        if (body != NULL) result = body->shape.type;
        else TRACELOG("[PHYSAC] WARNING: GetPhysicsShapeType: NULL physic body\n");
    }
    else TRACELOG("[PHYSAC] WARNING: Physic body index is out of bounds\n");

    return result;
}

// Returns the amount of vertices of a physics body shape
PHYSACDEF int GetPhysicsShapeVerticesCount(int index)
{
    int result = 0;

    if (index < (int)physicsBodiesCount)
    {
        PhysicsBody body = bodies[index];

        if (body != NULL)
        {
            switch (body->shape.type)
            {
                case PHYSICS_CIRCLE: result = PHYSAC_DEFAULT_CIRCLE_VERTICES; break;
                case PHYSICS_POLYGON: result = body->shape.vertexData.vertexCount; break;
                default: break;
            }
        }
        else TRACELOG("[PHYSAC] WARNING: GetPhysicsShapeVerticesCount: NULL physic body\n");
    }
    else TRACELOG("[PHYSAC] WARNING: Physic body index is out of bounds\n");

    return result;
}

// Returns transformed position of a body shape (body position + vertex transformed position)
PHYSACDEF Vector2 GetPhysicsShapeVertex(PhysicsBody body, int vertex)
{
    Vector2 position = { 0.0f, 0.0f };

    if (body != NULL)
    {
        switch (body->shape.type)
        {
            case PHYSICS_CIRCLE:
            {
                position.x = body->position.x + cosf(360.0f/PHYSAC_DEFAULT_CIRCLE_VERTICES*vertex*PHYSAC_DEG2RAD)*body->shape.radius;
                position.y = body->position.y + sinf(360.0f/PHYSAC_DEFAULT_CIRCLE_VERTICES*vertex*PHYSAC_DEG2RAD)*body->shape.radius;
            } break;
            case PHYSICS_POLYGON:
            {
                PhysicsVertexData vertexData = body->shape.vertexData;
                position = MathVector2Add(body->position, MathMatVector2Product(body->shape.transform, vertexData.positions[vertex]));
            } break;
            default: break;
        }
    }
    else TRACELOG("[PHYSAC] WARNING: GetPhysicsShapeVertex: NULL physic body\n");

    return position;
}

// Sets physics body shape transform based on radians parameter
PHYSACDEF void SetPhysicsBodyRotation(PhysicsBody body, float radians)
{
    if (body != NULL)
    {
        body->orient = radians;

        if (body->shape.type == PHYSICS_POLYGON) body->shape.transform = MathMatFromRadians(radians);
    }
}

// Unitializes and destroys a physics body
PHYSACDEF void DestroyPhysicsBody(PhysicsBody body)
{
    if (body != NULL)
    {
        int id = body->id;
        int index = -1;

        for (unsigned int i = 0; i < physicsBodiesCount; i++)
        {
            if (bodies[i]->id == id)
            {
                index = i;
                break;
            }
        }

        if (index == -1)
        {
            TRACELOG("[PHYSAC] WARNING: Requested body (id: %i) can not be found\n", id);
            return;     // Prevent access to index -1
        }

        // Free body allocated memory
//        PHYSAC_FREE(body);
//        usedMemory -= sizeof(PhysicsBodyData);
//        bodies[index] = NULL;

        // Reorder physics bodies pointers array and its catched index
        for (unsigned int i = index; i < physicsBodiesCount; i++)
        {
            if ((i + 1) < physicsBodiesCount) bodies[i] = bodies[i + 1];
        }

        // Update physics bodies count
        physicsBodiesCount--;

        TRACELOG("[PHYSAC] Physic body destroyed successfully (id: %i)\n", id);
    }
    else TRACELOG("[PHYSAC] WARNING: DestroyPhysicsBody: NULL physic body\n");
}

// Destroys created physics bodies and manifolds and resets global values
PHYSACDEF void ResetPhysics(void)
{
    if (physicsBodiesCount > 0)
    {
        // Unitialize physics bodies dynamic memory allocations
        for (int i = physicsBodiesCount - 1; i >= 0; i--)
        {
            PhysicsBody body = bodies[i];

            if (body != NULL)
            {
                PHYSAC_FREE(body);
                bodies[i] = NULL;
                usedMemory -= sizeof(PhysicsBodyData);
            }
        }

        physicsBodiesCount = 0;
    }

    if (physicsManifoldsCount > 0)
    {
        // Unitialize physics manifolds dynamic memory allocations
        for (int i = physicsManifoldsCount - 1; i >= 0; i--)
        {
            PhysicsManifold manifold = contacts[i];

            if (manifold != NULL)
            {
                PHYSAC_FREE(manifold);
                contacts[i] = NULL;
                usedMemory -= sizeof(PhysicsManifoldData);
            }
        }

        physicsManifoldsCount = 0;
    }

    TRACELOG("[PHYSAC] Physics module reseted successfully\n");
}

// Unitializes physics pointers and exits physics loop thread
PHYSACDEF void ClosePhysics(void)
{
    // Unitialize physics manifolds dynamic memory allocations
    if (physicsManifoldsCount > 0)
    {
        for (unsigned int i = physicsManifoldsCount - 1; i >= 0; i--)
            DestroyPhysicsManifold(contacts[i]);
    }
    
    // Unitialize physics bodies dynamic memory allocations
    if (physicsBodiesCount > 0)
    {
        for (unsigned int i = physicsBodiesCount - 1; i >= 0; i--)
            DestroyPhysicsBody(bodies[i]);
    }

    // Trace log info
    if ((physicsBodiesCount > 0) || (usedMemory != 0)) 
    {
        TRACELOG("[PHYSAC] WARNING: Physics module closed with unallocated bodies (BODIES: %i, MEMORY: %i bytes)\n", physicsBodiesCount, usedMemory);
    }
    else if ((physicsManifoldsCount > 0) || (usedMemory != 0)) 
    {
        TRACELOG("[PHYSAC] WARNING: Pysics module closed with unallocated manifolds (MANIFOLDS: %i, MEMORY: %i bytes)\n", physicsManifoldsCount, usedMemory);
    }
    else TRACELOG("[PHYSAC] Physics module closed successfully\n");
}

//----------------------------------------------------------------------------------
// Module Internal Functions Definition
//----------------------------------------------------------------------------------
// Finds a valid index for a new physics body initialization
static int FindAvailableBodyIndex()
{
    int index = -1;
    for (int i = 0; i < PHYSAC_MAX_BODIES; i++)
    {
        int currentId = i;

        // Check if current id already exist in other physics body
        for (unsigned int k = 0; k < physicsBodiesCount; k++)
        {
            if (bodies[k]->id == currentId)
            {
                currentId++;
                break;
            }
        }

        // If it is not used, use it as new physics body id
        if (currentId == (int)i)
        {
            index = (int)i;
            break;
        }
    }

    return index;
}

// Creates a default polygon shape with max vertex distance from polygon pivot
static PhysicsVertexData CreateDefaultPolygon(float radius, int sides)
{
    PhysicsVertexData data = { 0 };
    data.vertexCount = sides;

    // Calculate polygon vertices positions
    for (unsigned int i = 0; i < data.vertexCount; i++)
    {
        data.positions[i].x = (float)cosf(360.0f/sides*i*PHYSAC_DEG2RAD)*radius;
        data.positions[i].y = (float)sinf(360.0f/sides*i*PHYSAC_DEG2RAD)*radius;
    }

    // Calculate polygon faces normals
    for (int i = 0; i < (int)data.vertexCount; i++)
    {
        int nextIndex = (((i + 1) < sides) ? (i + 1) : 0);
        Vector2 face = MathVector2Subtract(data.positions[nextIndex], data.positions[i]);

        data.normals[i] = CLITERAL(Vector2){ face.y, -face.x };
        MathVector2Normalize(&data.normals[i]);
    }

    return data;
}

// Creates a rectangle polygon shape based on a min and max positions
static PhysicsVertexData CreateRectanglePolygon(Vector2 pos, Vector2 size)
{
    PhysicsVertexData data = { 0 };
    data.vertexCount = 4;

    // Calculate polygon vertices positions
    data.positions[0] = CLITERAL(Vector2){ pos.x + size.x/2, pos.y - size.y/2 };
    data.positions[1] = CLITERAL(Vector2){ pos.x + size.x/2, pos.y + size.y/2 };
    data.positions[2] = CLITERAL(Vector2){ pos.x - size.x/2, pos.y + size.y/2 };
    data.positions[3] = CLITERAL(Vector2){ pos.x - size.x/2, pos.y - size.y/2 };

    // Calculate polygon faces normals
    for (unsigned int i = 0; i < data.vertexCount; i++)
    {
        int nextIndex = (((i + 1) < data.vertexCount) ? (i + 1) : 0);
        Vector2 face = MathVector2Subtract(data.positions[nextIndex], data.positions[i]);

        data.normals[i] = CLITERAL(Vector2){ face.y, -face.x };
        MathVector2Normalize(&data.normals[i]);
    }

    return data;
}

// Update physics step (dynamics, collisions and position corrections)
void UpdatePhysicsStep(void)
{
    // Clear previous generated collisions information
    for (int i = (int)physicsManifoldsCount - 1; i >= 0; i--)
    {
        PhysicsManifold manifold = contacts[i];
        if (manifold != NULL) DestroyPhysicsManifold(manifold);
    }

    // Reset physics bodies grounded state
    for (unsigned int i = 0; i < physicsBodiesCount; i++)
    {
        PhysicsBody body = bodies[i];
        body->isGrounded = false;
    }

    for (unsigned int i = 0; i < physicsBodiesCount; i++){
        bodies[i]->newlyadded = false;
    }

    // Generate new collision information
    for (unsigned int i = 0; i < physicsBodiesCount; i++)
    {
        PhysicsBody bodyA = bodies[i];

        if(bodyA->newlyadded)
            break;

        if (bodyA != NULL)
        {
            for (unsigned int j = i + 1; j < physicsBodiesCount; j++)
            {
                PhysicsBody bodyB = bodies[j];
                if(bodyB->newlyadded)
                    break;

                if (bodyB != NULL)
                {
                    if ((bodyA->inverseMass == 0) && (bodyB->inverseMass == 0)) continue;

                    PhysicsManifold manifold = CreatePhysicsManifold(bodyA, bodyB);
                    SolvePhysicsManifold(manifold);

                    if (manifold->contactsCount > 0)
                    {

                        PhysicsBody BreakBody = bodyB;

                        if(!bodyA->holded and !bodyB->holded and bodyA->id != 0 and bodyB->id != 0 and bodyA->sides == bodyB->sides) {
                            if(bodyA->twinID != bodyB->id) {
                                bodyA->tobeDeleted = true;
                                bodyB->tobeDeleted = true;
                                bodyA->holded = true;
                                bodyB->holded = true;
                            }
                        }


                        if(bodyA->holded != true and bodyB->holded != true and BreakBody->breakable and BreakBody->id != 0 and bodyA->id != 0){

                            PhysicsBody body = CreatePhysicsBodyPolygon(BreakBody->position, BreakBody->shape.radius, BreakBody->sides, BreakBody->density);
                            body->twinID = BreakBody->id;
                            BreakBody->twinID = body->id;
                            BreakBody->breakable = false;
                            body->breakable = false;
                            body->holded = false;
                            body->tobeDeleted = false;
                            bodyB->breakable = false;

                            if (body != NULL){
                                body->useGravity = false;
                                body->useLocalGravity = true;
                                int L = std::min(Game::screenWidth, Game::screenHeight);
                                float x_position = L / 2;
                                float y_position = L / 2;
                                float radius = 110;
                                body->anchor = (Vector2){ x_position, y_position};
                                body->anchorMass = radius*radius*3.1415926 * 100;
                                body->anchorForce = Game::pull_coef;
                            }
                        }

                        // Create a new manifold with same information as previously solved manifold and add it to the manifolds pool last slot
                        PhysicsManifold manifold = CreatePhysicsManifold(bodyA, bodyB);
                        manifold->penetration = manifold->penetration;
                        manifold->normal = manifold->normal;
                        manifold->contacts[0] = manifold->contacts[0];
                        manifold->contacts[1] = manifold->contacts[1];
                        manifold->contactsCount = manifold->contactsCount;
                        manifold->restitution = manifold->restitution;
                        manifold->dynamicFriction = manifold->dynamicFriction;
                        manifold->staticFriction = manifold->staticFriction;
                    }
                }
            }
        }
    }


    for (int i = physicsBodiesCount-1; i >=0 ; i--)
        if (bodies[i]->tobeDeleted)
            DestroyPhysicsBody(bodies[i]);

    // Integrate forces to physics bodies
    for (unsigned int i = 0; i < physicsBodiesCount; i++)
    {
        PhysicsBody body = bodies[i];
        if (body != NULL) IntegratePhysicsForces(body);
    }

    // Initialize physics manifolds to solve collisions
    for (unsigned int i = 0; i < physicsManifoldsCount; i++)
    {
        PhysicsManifold manifold = contacts[i];
        if (manifold != NULL) InitializePhysicsManifolds(manifold);
    }

    // Integrate physics collisions impulses to solve collisions
    for (unsigned int i = 0; i < PHYSAC_COLLISION_ITERATIONS; i++)
    {
        for (unsigned int j = 0; j < physicsManifoldsCount; j++)
        {
            PhysicsManifold manifold = contacts[i];
            if (manifold != NULL) IntegratePhysicsImpulses(manifold);
        }
    }

    // Integrate velocity to physics bodies
    for (unsigned int i = 0; i < physicsBodiesCount; i++)
    {
        PhysicsBody body = bodies[i];
        if (body != NULL) IntegratePhysicsVelocity(body);
    }

    // Correct physics bodies positions based on manifolds collision information
    for (unsigned int i = 0; i < physicsManifoldsCount; i++)
    {
        PhysicsManifold manifold = contacts[i];
        if (manifold != NULL) CorrectPhysicsPositions(manifold);
    }

    // Clear physics bodies forces
    for (unsigned int i = 0; i < physicsBodiesCount; i++)
    {
        PhysicsBody body = bodies[i];
        if (body != NULL)
        {
            body->force = PHYSAC_VECTOR_ZERO;
            body->torque = 0.0f;
        }
    }
}

// Update physics system
// Physics steps are launched at a fixed time step if enabled
PHYSACDEF void UpdatePhysics(void)
{
#if !defined(PHYSAC_AVOID_TIMMING_SYSTEM)
    static double deltaTimeAccumulator = 0.0;

    // Calculate current time (ms)
    currentTime = GetCurrentTime();

    // Calculate current delta time (ms)
    const double delta = currentTime - startTime;

    // Store the time elapsed since the last frame began
    deltaTimeAccumulator += delta;

    // Fixed time stepping loop
    while (deltaTimeAccumulator >= deltaTime)
    {
        UpdatePhysicsStep();
        deltaTimeAccumulator -= deltaTime;
    }

    // Record the starting of this frame
    startTime = currentTime;
#else
    UpdatePhysicsStep();
#endif
}

PHYSACDEF void SetPhysicsTimeStep(double delta)
{
    deltaTime = delta;
}

// Finds a valid index for a new manifold initialization
static int FindAvailableManifoldIndex()
{
    int index = -1;
    for (int i = 0; i < PHYSAC_MAX_MANIFOLDS; i++)
    {
        int currentId = i;

        // Check if current id already exist in other physics body
        for (unsigned int k = 0; k < physicsManifoldsCount; k++)
        {
            if (contacts[k]->id == currentId)
            {
                currentId++;
                break;
            }
        }

        // If it is not used, use it as new physics body id
        if (currentId == i)
        {
            index = i;
            break;
        }
    }

    return index;
}

// Creates a new physics manifold to solve collision
static PhysicsManifold CreatePhysicsManifold(PhysicsBody a, PhysicsBody b)
{
    PhysicsManifold manifold = (PhysicsManifold)PHYSAC_MALLOC(sizeof(PhysicsManifoldData));
    usedMemory += sizeof(PhysicsManifoldData);

    int id = FindAvailableManifoldIndex();
    if (id != -1)
    {
        // Initialize new manifold with generic values
        manifold->id = id;
        manifold->bodyA = a;
        manifold->bodyB = b;
        manifold->penetration = 0;
        manifold->normal = PHYSAC_VECTOR_ZERO;
        manifold->contacts[0] = PHYSAC_VECTOR_ZERO;
        manifold->contacts[1] = PHYSAC_VECTOR_ZERO;
        manifold->contactsCount = 0;
        manifold->restitution = 0.0f;
        manifold->dynamicFriction = 0.0f;
        manifold->staticFriction = 0.0f;

        // Add new body to bodies pointers array and update bodies count
        contacts[physicsManifoldsCount] = manifold;
        physicsManifoldsCount++;
    }
    else TRACELOG("[PHYSAC] Physic manifold could not be created, PHYSAC_MAX_MANIFOLDS reached\n");

    return manifold;
}

// Unitializes and destroys a physics manifold
static void DestroyPhysicsManifold(PhysicsManifold manifold)
{
    if (manifold != NULL)
    {
        int id = manifold->id;
        int index = -1;

        for (unsigned int i = 0; i < physicsManifoldsCount; i++)
        {
            if (contacts[i]->id == id)
            {
                index = i;
                break;
            }
        }

        if (index == -1) return;     // Prevent access to index -1

        // Free manifold allocated memory
        PHYSAC_FREE(manifold);
        usedMemory -= sizeof(PhysicsManifoldData);
        contacts[index] = NULL;

        // Reorder physics manifolds pointers array and its catched index
        for (unsigned int i = index; i < physicsManifoldsCount; i++)
        {
            if ((i + 1) < physicsManifoldsCount) contacts[i] = contacts[i + 1];
        }

        // Update physics manifolds count
        physicsManifoldsCount--;
    }
    else TRACELOG("[PHYSAC] WARNING: DestroyPhysicsManifold: NULL physic manifold\n");
}

// Solves a created physics manifold between two physics bodies
static void SolvePhysicsManifold(PhysicsManifold manifold)
{
    switch (manifold->bodyA->shape.type)
    {
        case PHYSICS_CIRCLE:
        {
            switch (manifold->bodyB->shape.type)
            {
                case PHYSICS_CIRCLE: SolveCircleToCircle(manifold); break;
                case PHYSICS_POLYGON: SolveCircleToPolygon(manifold); break;
                default: break;
            }
        } break;
        case PHYSICS_POLYGON:
        {
            switch (manifold->bodyB->shape.type)
            {
                case PHYSICS_CIRCLE: SolvePolygonToCircle(manifold); break;
                case PHYSICS_POLYGON: SolvePolygonToPolygon(manifold); break;
                default: break;
            }
        } break;
        default: break;
    }

    // Update physics body grounded state if normal direction is down and grounded state is not set yet in previous manifolds
    if (!manifold->bodyB->isGrounded) manifold->bodyB->isGrounded = (manifold->normal.y < 0);
}

// Solves collision between two circle shape physics bodies
static void SolveCircleToCircle(PhysicsManifold manifold)
{
    PhysicsBody bodyA = manifold->bodyA;
    PhysicsBody bodyB = manifold->bodyB;

    if ((bodyA == NULL) || (bodyB == NULL)) return;

    // Calculate translational vector, which is normal
    Vector2 normal = MathVector2Subtract(bodyB->position, bodyA->position);

    float distSqr = MathVector2SqrLen(normal);
    float radius = bodyA->shape.radius + bodyB->shape.radius;

    // Check if circles are not in contact
    if (distSqr >= radius*radius)
    {
        manifold->contactsCount = 0;
        return;
    }

    float distance = sqrtf(distSqr);
    manifold->contactsCount = 1;

    if (distance == 0.0f)
    {
        manifold->penetration = bodyA->shape.radius;
        manifold->normal = CLITERAL(Vector2){ 1.0f, 0.0f };
        manifold->contacts[0] = bodyA->position;
    }
    else
    {
        manifold->penetration = radius - distance;
        manifold->normal = CLITERAL(Vector2){ normal.x/distance, normal.y/distance }; // Faster than using MathVector2Normalize() due to sqrt is already performed
        manifold->contacts[0] = CLITERAL(Vector2){ manifold->normal.x*bodyA->shape.radius + bodyA->position.x, manifold->normal.y*bodyA->shape.radius + bodyA->position.y };
    }

    // Update physics body grounded state if normal direction is down
    if (!bodyA->isGrounded) bodyA->isGrounded = (manifold->normal.y < 0);
}

// Solves collision between a circle to a polygon shape physics bodies
static void SolveCircleToPolygon(PhysicsManifold manifold)
{
    PhysicsBody bodyA = manifold->bodyA;
    PhysicsBody bodyB = manifold->bodyB;

    if ((bodyA == NULL) || (bodyB == NULL)) return;

    manifold->contactsCount = 0;

    // Transform circle center to polygon transform space
    Vector2 center = bodyA->position;
    center = MathMatVector2Product(MathMatTranspose(bodyB->shape.transform), MathVector2Subtract(center, bodyB->position));

    // Find edge with minimum penetration
    // It is the same concept as using support points in SolvePolygonToPolygon
    float separation = -PHYSAC_FLT_MAX;
    int faceNormal = 0;
    PhysicsVertexData vertexData = bodyB->shape.vertexData;

    for (unsigned int i = 0; i < vertexData.vertexCount; i++)
    {
        float currentSeparation = MathVector2DotProduct(vertexData.normals[i], MathVector2Subtract(center, vertexData.positions[i]));

        if (currentSeparation > bodyA->shape.radius) return;

        if (currentSeparation > separation)
        {
            separation = currentSeparation;
            faceNormal = i;
        }
    }

    // Grab face's vertices
    Vector2 v1 = vertexData.positions[faceNormal];
    int nextIndex = (((faceNormal + 1) < (int)vertexData.vertexCount) ? (faceNormal + 1) : 0);
    Vector2 v2 = vertexData.positions[nextIndex];

    // Check to see if center is within polygon
    if (separation < PHYSAC_EPSILON)
    {
        manifold->contactsCount = 1;
        Vector2 normal = MathMatVector2Product(bodyB->shape.transform, vertexData.normals[faceNormal]);
        manifold->normal = CLITERAL(Vector2){ -normal.x, -normal.y };
        manifold->contacts[0] = CLITERAL(Vector2){ manifold->normal.x*bodyA->shape.radius + bodyA->position.x, manifold->normal.y*bodyA->shape.radius + bodyA->position.y };
        manifold->penetration = bodyA->shape.radius;
        return;
    }

    // Determine which voronoi region of the edge center of circle lies within
    float dot1 = MathVector2DotProduct(MathVector2Subtract(center, v1), MathVector2Subtract(v2, v1));
    float dot2 = MathVector2DotProduct(MathVector2Subtract(center, v2), MathVector2Subtract(v1, v2));
    manifold->penetration = bodyA->shape.radius - separation;

    if (dot1 <= 0.0f) // Closest to v1
    {
        if (MathVector2SqrDistance(center, v1) > bodyA->shape.radius*bodyA->shape.radius) return;

        manifold->contactsCount = 1;
        Vector2 normal = MathVector2Subtract(v1, center);
        normal = MathMatVector2Product(bodyB->shape.transform, normal);
        MathVector2Normalize(&normal);
        manifold->normal = normal;
        v1 = MathMatVector2Product(bodyB->shape.transform, v1);
        v1 = MathVector2Add(v1, bodyB->position);
        manifold->contacts[0] = v1;
    }
    else if (dot2 <= 0.0f) // Closest to v2
    {
        if (MathVector2SqrDistance(center, v2) > bodyA->shape.radius*bodyA->shape.radius) return;

        manifold->contactsCount = 1;
        Vector2 normal = MathVector2Subtract(v2, center);
        v2 = MathMatVector2Product(bodyB->shape.transform, v2);
        v2 = MathVector2Add(v2, bodyB->position);
        manifold->contacts[0] = v2;
        normal = MathMatVector2Product(bodyB->shape.transform, normal);
        MathVector2Normalize(&normal);
        manifold->normal = normal;
    }
    else // Closest to face
    {
        Vector2 normal = vertexData.normals[faceNormal];

        if (MathVector2DotProduct(MathVector2Subtract(center, v1), normal) > bodyA->shape.radius) return;

        normal = MathMatVector2Product(bodyB->shape.transform, normal);
        manifold->normal = CLITERAL(Vector2){ -normal.x, -normal.y };
        manifold->contacts[0] = CLITERAL(Vector2){ manifold->normal.x*bodyA->shape.radius + bodyA->position.x, manifold->normal.y*bodyA->shape.radius + bodyA->position.y };
        manifold->contactsCount = 1;
    }
}

// Solves collision between a polygon to a circle shape physics bodies
static void SolvePolygonToCircle(PhysicsManifold manifold)
{
    PhysicsBody bodyA = manifold->bodyA;
    PhysicsBody bodyB = manifold->bodyB;

    if ((bodyA == NULL) || (bodyB == NULL)) return;

    manifold->bodyA = bodyB;
    manifold->bodyB = bodyA;
    SolveCircleToPolygon(manifold);

    manifold->normal.x *= -1.0f;
    manifold->normal.y *= -1.0f;
}

// Solves collision between two polygons shape physics bodies
static void SolvePolygonToPolygon(PhysicsManifold manifold)
{
    if ((manifold->bodyA == NULL) || (manifold->bodyB == NULL)) return;

    PhysicsShape bodyA = manifold->bodyA->shape;
    PhysicsShape bodyB = manifold->bodyB->shape;
    manifold->contactsCount = 0;

    // Check for separating axis with A shape's face planes
    int faceA = 0;
    float penetrationA = FindAxisLeastPenetration(&faceA, bodyA, bodyB);
    if (penetrationA >= 0.0f) return;

    // Check for separating axis with B shape's face planes
    int faceB = 0;
    float penetrationB = FindAxisLeastPenetration(&faceB, bodyB, bodyA);
    if (penetrationB >= 0.0f) return;

    int referenceIndex = 0;
    bool flip = false;  // Always point from A shape to B shape

    PhysicsShape refPoly; // Reference
    PhysicsShape incPoly; // Incident

    // Determine which shape contains reference face
    // Checking bias range for penetration
    if (penetrationA >= (penetrationB*0.95f + penetrationA*0.01f))
    {
        refPoly = bodyA;
        incPoly = bodyB;
        referenceIndex = faceA;
    }
    else
    {
        refPoly = bodyB;
        incPoly = bodyA;
        referenceIndex = faceB;
        flip = true;
    }

    // World space incident face
    Vector2 incidentFace[2];
    FindIncidentFace(&incidentFace[0], &incidentFace[1], refPoly, incPoly, referenceIndex);

    // Setup reference face vertices
    PhysicsVertexData refData = refPoly.vertexData;
    Vector2 v1 = refData.positions[referenceIndex];
    referenceIndex = (((referenceIndex + 1) < (int)refData.vertexCount) ? (referenceIndex + 1) : 0);
    Vector2 v2 = refData.positions[referenceIndex];

    // Transform vertices to world space
    v1 = MathMatVector2Product(refPoly.transform, v1);
    v1 = MathVector2Add(v1, refPoly.body->position);
    v2 = MathMatVector2Product(refPoly.transform, v2);
    v2 = MathVector2Add(v2, refPoly.body->position);

    // Calculate reference face side normal in world space
    Vector2 sidePlaneNormal = MathVector2Subtract(v2, v1);
    MathVector2Normalize(&sidePlaneNormal);

    // Orthogonalize
    Vector2 refFaceNormal = { sidePlaneNormal.y, -sidePlaneNormal.x };
    float refC = MathVector2DotProduct(refFaceNormal, v1);
    float negSide = MathVector2DotProduct(sidePlaneNormal, v1)*-1;
    float posSide = MathVector2DotProduct(sidePlaneNormal, v2);

    // MathVector2Clip incident face to reference face side planes (due to floating point error, possible to not have required points
    if (MathVector2Clip(CLITERAL(Vector2){ -sidePlaneNormal.x, -sidePlaneNormal.y }, &incidentFace[0], &incidentFace[1], negSide) < 2) return;
    if (MathVector2Clip(sidePlaneNormal, &incidentFace[0], &incidentFace[1], posSide) < 2) return;

    // Flip normal if required
    manifold->normal = (flip ? CLITERAL(Vector2){ -refFaceNormal.x, -refFaceNormal.y } : refFaceNormal);

    // Keep points behind reference face
    int currentPoint = 0; // MathVector2Clipped points behind reference face
    float separation = MathVector2DotProduct(refFaceNormal, incidentFace[0]) - refC;
    if (separation <= 0.0f)
    {
        manifold->contacts[currentPoint] = incidentFace[0];
        manifold->penetration = -separation;
        currentPoint++;
    }
    else manifold->penetration = 0.0f;

    separation = MathVector2DotProduct(refFaceNormal, incidentFace[1]) - refC;

    if (separation <= 0.0f)
    {
        manifold->contacts[currentPoint] = incidentFace[1];
        manifold->penetration += -separation;
        currentPoint++;

        // Calculate total penetration average
        manifold->penetration /= currentPoint;
    }

    manifold->contactsCount = currentPoint;
}

// Integrates physics forces into velocity
static void IntegratePhysicsForces(PhysicsBody body)
{
    if ((body == NULL) || (body->inverseMass == 0.0f) || !body->enabled) return;
    Vector2 forceDirection;
    if (body->useLocalGravity)
    {
        forceDirection = MathVector2Subtract(body->anchor, body->position);
        float distance = MathVector2SqrLen(forceDirection);
        float force = body->mass * body->anchorMass / distance;
        // printf("distance %f, force %f, mass %f, %f\n", distance, force, body->mass, body->anchorMass);
        MathVector2Normalize(&forceDirection);

        forceDirection.x *= body->anchorForce * force;
        forceDirection.y *= body->anchorForce * force;
        PhysicsAddForce(body, forceDirection);
    }
    body->velocity.x += (float)((body->force.x*body->inverseMass)*(deltaTime/2.0));
    body->velocity.y += (float)((body->force.y*body->inverseMass)*(deltaTime/2.0));
    if (body->useLocalGravity)
    {
        forceDirection.x *= -1;
        forceDirection.y *= -1;
        PhysicsAddForce(body, forceDirection);
    }
    if (body->useGravity)
    {
        body->velocity.x += (float)(gravityForce.x*(deltaTime/1000/2.0));
        body->velocity.y += (float)(gravityForce.y*(deltaTime/1000/2.0));
    }

    if (!body->freezeOrient) body->angularVelocity += (float)(body->torque*body->inverseInertia*(deltaTime/2.0));
}

// Initializes physics manifolds to solve collisions
static void InitializePhysicsManifolds(PhysicsManifold manifold)
{
    PhysicsBody bodyA = manifold->bodyA;
    PhysicsBody bodyB = manifold->bodyB;

    if ((bodyA == NULL) || (bodyB == NULL)) return;

    // Calculate average restitution, static and dynamic friction
    manifold->restitution = sqrtf(bodyA->restitution*bodyB->restitution);
    manifold->staticFriction = sqrtf(bodyA->staticFriction*bodyB->staticFriction);
    manifold->dynamicFriction = sqrtf(bodyA->dynamicFriction*bodyB->dynamicFriction);

    for (unsigned int i = 0; i < manifold->contactsCount; i++)
    {
        // Caculate radius from center of mass to contact
        Vector2 radiusA = MathVector2Subtract(manifold->contacts[i], bodyA->position);
        Vector2 radiusB = MathVector2Subtract(manifold->contacts[i], bodyB->position);

        Vector2 crossA = MathVector2Product(radiusA, bodyA->angularVelocity);
        Vector2 crossB = MathVector2Product(radiusB, bodyB->angularVelocity);

        Vector2 radiusV = { 0.0f, 0.0f };
        radiusV.x = bodyB->velocity.x + crossB.x - bodyA->velocity.x - crossA.x;
        radiusV.y = bodyB->velocity.y + crossB.y - bodyA->velocity.y - crossA.y;

        // Determine if we should perform a resting collision or not;
        // The idea is if the only thing moving this object is gravity, then the collision should be performed without any restitution
        if (MathVector2SqrLen(radiusV) < (MathVector2SqrLen(CLITERAL(Vector2){ (float)(gravityForce.x*deltaTime/1000), (float)(gravityForce.y*deltaTime/1000) }) + PHYSAC_EPSILON)) manifold->restitution = 0;
    }
}

// Integrates physics collisions impulses to solve collisions
static void IntegratePhysicsImpulses(PhysicsManifold manifold)
{
    PhysicsBody bodyA = manifold->bodyA;
    PhysicsBody bodyB = manifold->bodyB;

    if ((bodyA == NULL) || (bodyB == NULL)) return;

    // Early out and positional correct if both objects have infinite mass
    if (fabs(bodyA->inverseMass + bodyB->inverseMass) <= PHYSAC_EPSILON)
    {
        bodyA->velocity = PHYSAC_VECTOR_ZERO;
        bodyB->velocity = PHYSAC_VECTOR_ZERO;
        return;
    }

    for (unsigned int i = 0; i < manifold->contactsCount; i++)
    {
        // Calculate radius from center of mass to contact
        Vector2 radiusA = MathVector2Subtract(manifold->contacts[i], bodyA->position);
        Vector2 radiusB = MathVector2Subtract(manifold->contacts[i], bodyB->position);

        // Calculate relative velocity
        Vector2 radiusV = { 0.0f, 0.0f };
        radiusV.x = bodyB->velocity.x + MathVector2Product(radiusB, bodyB->angularVelocity).x - bodyA->velocity.x - MathVector2Product(radiusA, bodyA->angularVelocity).x;
        radiusV.y = bodyB->velocity.y + MathVector2Product(radiusB, bodyB->angularVelocity).y - bodyA->velocity.y - MathVector2Product(radiusA, bodyA->angularVelocity).y;

        // Relative velocity along the normal
        float contactVelocity = MathVector2DotProduct(radiusV, manifold->normal);

        // Do not resolve if velocities are separating
        if (contactVelocity > 0.0f) return;

        float raCrossN = MathVector2CrossProduct(radiusA, manifold->normal);
        float rbCrossN = MathVector2CrossProduct(radiusB, manifold->normal);

        float inverseMassSum = bodyA->inverseMass + bodyB->inverseMass + (raCrossN*raCrossN)*bodyA->inverseInertia + (rbCrossN*rbCrossN)*bodyB->inverseInertia;

        // Calculate impulse scalar value
        float impulse = -(1.0f + manifold->restitution)*contactVelocity;
        impulse /= inverseMassSum;
        impulse /= (float)manifold->contactsCount;

        // Apply impulse to each physics body
        Vector2 impulseV = { manifold->normal.x*impulse, manifold->normal.y*impulse };

        if (bodyA->enabled)
        {
            bodyA->velocity.x += bodyA->inverseMass*(-impulseV.x);
            bodyA->velocity.y += bodyA->inverseMass*(-impulseV.y);
            if (!bodyA->freezeOrient) bodyA->angularVelocity += bodyA->inverseInertia*MathVector2CrossProduct(radiusA, CLITERAL(Vector2){ -impulseV.x, -impulseV.y });
        }

        if (bodyB->enabled)
        {
            bodyB->velocity.x += bodyB->inverseMass*(impulseV.x);
            bodyB->velocity.y += bodyB->inverseMass*(impulseV.y);
            if (!bodyB->freezeOrient) bodyB->angularVelocity += bodyB->inverseInertia*MathVector2CrossProduct(radiusB, impulseV);
        }

        // Apply friction impulse to each physics body
        radiusV.x = bodyB->velocity.x + MathVector2Product(radiusB, bodyB->angularVelocity).x - bodyA->velocity.x - MathVector2Product(radiusA, bodyA->angularVelocity).x;
        radiusV.y = bodyB->velocity.y + MathVector2Product(radiusB, bodyB->angularVelocity).y - bodyA->velocity.y - MathVector2Product(radiusA, bodyA->angularVelocity).y;

        Vector2 tangent = { radiusV.x - (manifold->normal.x*MathVector2DotProduct(radiusV, manifold->normal)), radiusV.y - (manifold->normal.y*MathVector2DotProduct(radiusV, manifold->normal)) };
        MathVector2Normalize(&tangent);

        // Calculate impulse tangent magnitude
        float impulseTangent = -MathVector2DotProduct(radiusV, tangent);
        impulseTangent /= inverseMassSum;
        impulseTangent /= (float)manifold->contactsCount;

        float absImpulseTangent = (float)fabs(impulseTangent);

        // Don't apply tiny friction impulses
        if (absImpulseTangent <= PHYSAC_EPSILON) return;

        // Apply coulumb's law
        Vector2 tangentImpulse = { 0.0f, 0.0f };
        if (absImpulseTangent < impulse*manifold->staticFriction) tangentImpulse = CLITERAL(Vector2){ tangent.x*impulseTangent, tangent.y*impulseTangent };
        else tangentImpulse = CLITERAL(Vector2){ tangent.x*-impulse*manifold->dynamicFriction, tangent.y*-impulse*manifold->dynamicFriction };

        // Apply friction impulse
        if (bodyA->enabled)
        {
            bodyA->velocity.x += bodyA->inverseMass*(-tangentImpulse.x);
            bodyA->velocity.y += bodyA->inverseMass*(-tangentImpulse.y);

            if (!bodyA->freezeOrient) bodyA->angularVelocity += bodyA->inverseInertia*MathVector2CrossProduct(radiusA, CLITERAL(Vector2){ -tangentImpulse.x, -tangentImpulse.y });
        }

        if (bodyB->enabled)
        {
            bodyB->velocity.x += bodyB->inverseMass*(tangentImpulse.x);
            bodyB->velocity.y += bodyB->inverseMass*(tangentImpulse.y);

            if (!bodyB->freezeOrient) bodyB->angularVelocity += bodyB->inverseInertia*MathVector2CrossProduct(radiusB, tangentImpulse);
        }
    }
}

// Integrates physics velocity into position and forces
static void IntegratePhysicsVelocity(PhysicsBody body)
{
    if ((body == NULL) ||!body->enabled) return;

    body->position.x += (float)(body->velocity.x*deltaTime);
    body->position.y += (float)(body->velocity.y*deltaTime);

    if (!body->freezeOrient) body->orient += (float)(body->angularVelocity*deltaTime);
    body->shape.transform = MathMatFromRadians(body->orient);

    IntegratePhysicsForces(body);
}

// Corrects physics bodies positions based on manifolds collision information
static void CorrectPhysicsPositions(PhysicsManifold manifold)
{
    PhysicsBody bodyA = manifold->bodyA;
    PhysicsBody bodyB = manifold->bodyB;

    if ((bodyA == NULL) || (bodyB == NULL)) return;

    Vector2 correction = { 0.0f, 0.0f };
    correction.x = (PHYSAC_MAX(manifold->penetration - PHYSAC_PENETRATION_ALLOWANCE, 0.0f)/(bodyA->inverseMass + bodyB->inverseMass))*manifold->normal.x*PHYSAC_PENETRATION_CORRECTION;
    correction.y = (PHYSAC_MAX(manifold->penetration - PHYSAC_PENETRATION_ALLOWANCE, 0.0f)/(bodyA->inverseMass + bodyB->inverseMass))*manifold->normal.y*PHYSAC_PENETRATION_CORRECTION;

    if (bodyA->enabled)
    {
        bodyA->position.x -= correction.x*bodyA->inverseMass;
        bodyA->position.y -= correction.y*bodyA->inverseMass;
    }

    if (bodyB->enabled)
    {
        bodyB->position.x += correction.x*bodyB->inverseMass;
        bodyB->position.y += correction.y*bodyB->inverseMass;
    }
}

// Returns the extreme point along a direction within a polygon
static Vector2 GetSupport(PhysicsShape shape, Vector2 dir)
{
    float bestProjection = -PHYSAC_FLT_MAX;
    Vector2 bestVertex = { 0.0f, 0.0f };
    PhysicsVertexData data = shape.vertexData;

    for (unsigned int i = 0; i < data.vertexCount; i++)
    {
        Vector2 vertex = data.positions[i];
        float projection = MathVector2DotProduct(vertex, dir);

        if (projection > bestProjection)
        {
            bestVertex = vertex;
            bestProjection = projection;
        }
    }

    return bestVertex;
}

// Finds polygon shapes axis least penetration
static float FindAxisLeastPenetration(int *faceIndex, PhysicsShape shapeA, PhysicsShape shapeB)
{
    float bestDistance = -PHYSAC_FLT_MAX;
    int bestIndex = 0;

    PhysicsVertexData dataA = shapeA.vertexData;
    //PhysicsVertexData dataB = shapeB.vertexData;

    for (unsigned int i = 0; i < dataA.vertexCount; i++)
    {
        // Retrieve a face normal from A shape
        Vector2 normal = dataA.normals[i];
        Vector2 transNormal = MathMatVector2Product(shapeA.transform, normal);

        // Transform face normal into B shape's model space
        Matrix2x2 buT = MathMatTranspose(shapeB.transform);
        normal = MathMatVector2Product(buT, transNormal);

        // Retrieve support point from B shape along -n
        Vector2 support = GetSupport(shapeB, CLITERAL(Vector2){ -normal.x, -normal.y });

        // Retrieve vertex on face from A shape, transform into B shape's model space
        Vector2 vertex = dataA.positions[i];
        vertex = MathMatVector2Product(shapeA.transform, vertex);
        vertex = MathVector2Add(vertex, shapeA.body->position);
        vertex = MathVector2Subtract(vertex, shapeB.body->position);
        vertex = MathMatVector2Product(buT, vertex);

        // Compute penetration distance in B shape's model space
        float distance = MathVector2DotProduct(normal, MathVector2Subtract(support, vertex));

        // Store greatest distance
        if (distance > bestDistance)
        {
            bestDistance = distance;
            bestIndex = i;
        }
    }

    *faceIndex = bestIndex;
    return bestDistance;
}

// Finds two polygon shapes incident face
static void FindIncidentFace(Vector2 *v0, Vector2 *v1, PhysicsShape ref, PhysicsShape inc, int index)
{
    PhysicsVertexData refData = ref.vertexData;
    PhysicsVertexData incData = inc.vertexData;

    Vector2 referenceNormal = refData.normals[index];

    // Calculate normal in incident's frame of reference
    referenceNormal = MathMatVector2Product(ref.transform, referenceNormal); // To world space
    referenceNormal = MathMatVector2Product(MathMatTranspose(inc.transform), referenceNormal); // To incident's model space

    // Find most anti-normal face on polygon
    int incidentFace = 0;
    float minDot = PHYSAC_FLT_MAX;

    for (unsigned int i = 0; i < incData.vertexCount; i++)
    {
        float dot = MathVector2DotProduct(referenceNormal, incData.normals[i]);

        if (dot < minDot)
        {
            minDot = dot;
            incidentFace = i;
        }
    }

    // Assign face vertices for incident face
    *v0 = MathMatVector2Product(inc.transform, incData.positions[incidentFace]);
    *v0 = MathVector2Add(*v0, inc.body->position);
    incidentFace = (((incidentFace + 1) < (int)incData.vertexCount) ? (incidentFace + 1) : 0);
    *v1 = MathMatVector2Product(inc.transform, incData.positions[incidentFace]);
    *v1 = MathVector2Add(*v1, inc.body->position);
}

// Returns clipping value based on a normal and two faces
static int MathVector2Clip(Vector2 normal, Vector2 *faceA, Vector2 *faceB, float clip)
{
    int sp = 0;
    Vector2 out[2] = { *faceA, *faceB };

    // Retrieve distances from each endpoint to the line
    float distanceA = MathVector2DotProduct(normal, *faceA) - clip;
    float distanceB = MathVector2DotProduct(normal, *faceB) - clip;

    // If negative (behind plane)
    if (distanceA <= 0.0f) out[sp++] = *faceA;
    if (distanceB <= 0.0f) out[sp++] = *faceB;

    // If the points are on different sides of the plane
    if ((distanceA*distanceB) < 0.0f)
    {
        // Push intersection point
        float alpha = distanceA/(distanceA - distanceB);
        out[sp] = *faceA;
        Vector2 delta = MathVector2Subtract(*faceB, *faceA);
        delta.x *= alpha;
        delta.y *= alpha;
        out[sp] = MathVector2Add(out[sp], delta);
        sp++;
    }

    // Assign the new converted values
    *faceA = out[0];
    *faceB = out[1];

    return sp;
}

// Returns the barycenter of a triangle given by 3 points
static Vector2 MathTriangleBarycenter(Vector2 v1, Vector2 v2, Vector2 v3)
{
    Vector2 result = { 0.0f, 0.0f };

    result.x = (v1.x + v2.x + v3.x)/3;
    result.y = (v1.y + v2.y + v3.y)/3;

    return result;
}

#if !defined(PHYSAC_AVOID_TIMMING_SYSTEM)
// Initializes hi-resolution MONOTONIC timer
static void InitTimer(void)
{
#if defined(_WIN32)
    QueryPerformanceFrequency((unsigned long long int *) &frequency);
#endif

#if defined(__EMSCRIPTEN__) || defined(__linux__)
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) == 0) frequency = 1000000000;
#endif

#if defined(__APPLE__)
    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);
    frequency = (timebase.denom*1e9)/timebase.numer;
#endif

    baseClockTicks = (double)GetClockTicks();      // Get MONOTONIC clock time offset
    startTime = GetCurrentTime();                  // Get current time in milliseconds
}

// Get hi-res MONOTONIC time measure in clock ticks
static unsigned long long int GetClockTicks(void)
{
    unsigned long long int value = 0;

#if defined(_WIN32)
    QueryPerformanceCounter((unsigned long long int *) &value);
#endif

#if defined(__linux__)
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    value = (unsigned long long int)now.tv_sec*(unsigned long long int)1000000000 + (unsigned long long int)now.tv_nsec;
#endif

#if defined(__APPLE__)
    value = mach_absolute_time();
#endif

    return value;
}

// Get current time in milliseconds
static double GetCurrentTime(void)
{
    return (double)(GetClockTicks() - baseClockTicks)/frequency*1000;
}
#endif // !PHYSAC_AVOID_TIMMING_SYSTEM


// Returns the cross product of a vector and a value
static inline Vector2 MathVector2Product(Vector2 vector, float value)
{
    Vector2 result = { -value*vector.y, value*vector.x };
    return result;
}

// Returns the cross product of two vectors
static inline float MathVector2CrossProduct(Vector2 v1, Vector2 v2)
{
    return (v1.x*v2.y - v1.y*v2.x);
}

// Returns the len square root of a vector
static inline float MathVector2SqrLen(Vector2 vector)
{
    return (vector.x*vector.x + vector.y*vector.y);
}

// Returns the dot product of two vectors
static inline float MathVector2DotProduct(Vector2 v1, Vector2 v2)
{
    return (v1.x*v2.x + v1.y*v2.y);
}

// Returns the square root of distance between two vectors
static inline float MathVector2SqrDistance(Vector2 v1, Vector2 v2)
{
    Vector2 dir = MathVector2Subtract(v1, v2);
    return MathVector2DotProduct(dir, dir);
}

// Returns the normalized values of a vector
static void MathVector2Normalize(Vector2 *vector)
{
    float length, ilength;

    Vector2 aux = *vector;
    length = sqrtf(aux.x*aux.x + aux.y*aux.y);

    if (length == 0) length = 1.0f;

    ilength = 1.0f/length;

    vector->x *= ilength;
    vector->y *= ilength;
}

// Returns the sum of two given vectors
static inline Vector2 MathVector2Add(Vector2 v1, Vector2 v2)
{
    Vector2 result = { v1.x + v2.x, v1.y + v2.y };
    return result;
}

// Returns the subtract of two given vectors
static inline Vector2 MathVector2Subtract(Vector2 v1, Vector2 v2)
{
    Vector2 result = { v1.x - v2.x, v1.y - v2.y };
    return result;
}

// Creates a matrix 2x2 from a given radians value
static Matrix2x2 MathMatFromRadians(float radians)
{
    float cos = cosf(radians);
    float sin = sinf(radians);

    Matrix2x2 result = { cos, -sin, sin, cos };
    return result;
}

// Returns the transpose of a given matrix 2x2
static inline Matrix2x2 MathMatTranspose(Matrix2x2 matrix)
{
    Matrix2x2 result = { matrix.m00, matrix.m10, matrix.m01, matrix.m11 };
    return result;
}

// Multiplies a vector by a matrix 2x2
static inline Vector2 MathMatVector2Product(Matrix2x2 matrix, Vector2 vector)
{
    Vector2 result = { matrix.m00*vector.x + matrix.m01*vector.y, matrix.m10*vector.x + matrix.m11*vector.y };
    return result;
}
