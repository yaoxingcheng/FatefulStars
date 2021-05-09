/**********************************************************************************************
*
*   Physac v1.1 - 2D Physics library for videogames
*
*   DESCRIPTION:
*
*   Physac is a small 2D physics engine written in pure C. The engine uses a fixed time-step thread loop
*   to simluate physics. A physics step contains the following phases: get collision information,
*   apply dynamics, collision solving and position correction. It uses a very simple struct for physic
*   bodies with a position vector to be used in any 3D rendering API.
*
*   CONFIGURATION:
*
*   #define PHYSAC_IMPLEMENTATION
*       Generates the implementation of the library into the included file.
*       If not defined, the library is in header only mode and can be included in other headers
*       or source files without problems. But only ONE file should hold the implementation.
*
*   #define PHYSAC_STATIC (defined by default)
*       The generated implementation will stay private inside implementation file and all
*       internal symbols and functions will only be visible inside that file.
*
*   #define PHYSAC_DEBUG
*       Show debug traces log messages about physic bodies creation/destruction, physic system errors,
*       some calculations results and NULL reference exceptions
*
*   #define PHYSAC_DEFINE_VECTOR2_TYPE
*       Forces library to define struct Vector2 data type (float x; float y)
*
*   #define PHYSAC_AVOID_TIMMING_SYSTEM
*       Disables internal timming system, used by UpdatePhysics() to launch timmed physic steps,
*       it allows just running UpdatePhysics() automatically on a separate thread at a desired time step.
*       In case physics steps update needs to be controlled by user with a custom timming mechanism,
*       just define this flag and the internal timming mechanism will be avoided, in that case,
*       timming libraries are neither required by the module.
*
*   #define PHYSAC_MALLOC()
*   #define PHYSAC_CALLOC()
*   #define PHYSAC_FREE()
*       You can define your own malloc/free implementation replacing stdlib.h malloc()/free() functions.
*       Otherwise it will include stdlib.h and use the C standard library malloc()/free() function.
*
*   COMPILATION:
*
*   Use the following code to compile with GCC:
*       gcc -o $(NAME_PART).exe $(FILE_NAME) -s -static -lraylib -lopengl32 -lgdi32 -lwinmm -std=c99
*
*   VERSIONS HISTORY:
*       1.1 (20-Jan-2021) @raysan5: Library general revision 
*               Removed threading system (up to the user)
*               Support MSVC C++ compilation using CLITERAL()
*               Review DEBUG mechanism for TRACELOG() and all TRACELOG() messages
*               Review internal variables/functions naming for consistency
*               Allow option to avoid internal timming system, to allow app manage the steps
*       1.0 (12-Jun-2017) First release of the library
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2016-2021 Victor Fisac (@victorfisac) and Ramon Santamaria (@raysan5)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#ifndef PHYSAC_H
#define PHYSAC_H

#if defined(PHYSAC_STATIC)
    #define PHYSACDEF static            // Functions just visible to module including this file
#else
    #if defined(__cplusplus)
        #define PHYSACDEF extern "C"    // Functions visible from other files (no name mangling of functions in C++)
    #else
        #define PHYSACDEF extern        // Functions visible from other files
    #endif
#endif

// Allow custom memory allocators
#ifndef PHYSAC_MALLOC
    #define PHYSAC_MALLOC(size)         malloc(size)
#endif
#ifndef PHYSAC_CALLOC
    #define PHYSAC_CALLOC(size, n)      calloc(size, n)
#endif
#ifndef PHYSAC_FREE
    #define PHYSAC_FREE(ptr)            free(ptr)
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define PHYSAC_MAX_BODIES               64          // Maximum number of physic bodies supported
#define PHYSAC_MAX_MANIFOLDS            4096        // Maximum number of physic bodies interactions (64x64)
#define PHYSAC_MAX_VERTICES             24          // Maximum number of vertex for polygons shapes
#define PHYSAC_DEFAULT_CIRCLE_VERTICES  24          // Default number of vertices for circle shapes

#define PHYSAC_COLLISION_ITERATIONS     100
#define PHYSAC_PENETRATION_ALLOWANCE    0.05f
#define PHYSAC_PENETRATION_CORRECTION   0.4f

#define PHYSAC_PI                       3.14159265358979323846f
#define PHYSAC_DEG2RAD                  (PHYSAC_PI/180.0f)

//----------------------------------------------------------------------------------
// Data Types Structure Definition
//----------------------------------------------------------------------------------
#if defined(__STDC__) && __STDC_VERSION__ >= 199901L
    #include <stdbool.h>
#endif

typedef enum PhysicsShapeType { PHYSICS_CIRCLE = 0, PHYSICS_POLYGON } PhysicsShapeType;

// Previously defined to be used in PhysicsShape struct as circular dependencies
typedef struct PhysicsBodyData *PhysicsBody;

#if defined(PHYSAC_DEFINE_VECTOR2_TYPE)
// Vector2 type
typedef struct Vector2 {
    float x;
    float y;
} Vector2;
#endif

// Matrix2x2 type (used for polygon shape rotation matrix)
typedef struct Matrix2x2 {
    float m00;
    float m01;
    float m10;
    float m11;
} Matrix2x2;

typedef struct PhysicsVertexData {
    unsigned int vertexCount;                   // Vertex count (positions and normals)
    Vector2 positions[PHYSAC_MAX_VERTICES];     // Vertex positions vectors
    Vector2 normals[PHYSAC_MAX_VERTICES];       // Vertex normals vectors
} PhysicsVertexData;

typedef struct PhysicsShape {
    PhysicsShapeType type;                      // Shape type (circle or polygon)
    PhysicsBody body;                           // Shape physics body data pointer
    PhysicsVertexData vertexData;               // Shape vertices data (used for polygon shapes)
    float radius;                               // Shape radius (used for circle shapes)
    Matrix2x2 transform;                        // Vertices transform matrix 2x2
} PhysicsShape;

typedef struct PhysicsBodyData {
    unsigned int id;                            // Unique identifier
    bool enabled;                               // Enabled dynamics state (collisions are calculated anyway)
    Vector2 position;                           // Physics body shape pivot
    Vector2 velocity;                           // Current linear velocity applied to position
    Vector2 force;                              // Current linear force (reset to 0 every step)
    float angularVelocity;                      // Current angular velocity applied to orient
    float torque;                               // Current angular force (reset to 0 every step)
    float orient;                               // Rotation in radians
    float inertia;                              // Moment of inertia
    float inverseInertia;                       // Inverse value of inertia
    float mass;                                 // Physics body mass
    float inverseMass;                          // Inverse value of mass
    float staticFriction;                       // Friction when the body has not movement (0 to 1)
    float dynamicFriction;                      // Friction when the body has movement (0 to 1)
    float restitution;                          // Restitution coefficient of the body (0 to 1)
    bool useGravity;                            // Apply gravity force to dynamics
    bool isGrounded;                            // Physics grounded on other body state
    bool freezeOrient;                          // Physics rotation constraint
    PhysicsShape shape;                         // Physics body shape information (type, radius, vertices, transform)
    bool useLocalGravity;
    Vector2 anchor;
    float anchorForce;
    float anchorMass;
} PhysicsBodyData;

typedef struct PhysicsManifoldData {
    unsigned int id;                            // Unique identifier
    PhysicsBody bodyA;                          // Manifold first physics body reference
    PhysicsBody bodyB;                          // Manifold second physics body reference
    float penetration;                          // Depth of penetration from collision
    Vector2 normal;                             // Normal direction vector from 'a' to 'b'
    Vector2 contacts[2];                        // Points of contact during collision
    unsigned int contactsCount;                 // Current collision number of contacts
    float restitution;                          // Mixed restitution during collision
    float dynamicFriction;                      // Mixed dynamic friction during collision
    float staticFriction;                       // Mixed static friction during collision
} PhysicsManifoldData, *PhysicsManifold;

#if defined(__cplusplus)
extern "C" {                                    // Prevents name mangling of functions
#endif

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
// Physics system management
PHYSACDEF void InitPhysics(void);                                                                           // Initializes physics system
PHYSACDEF void UpdatePhysics(void);                                                                         // Update physics system
PHYSACDEF void ResetPhysics(void);                                                                          // Reset physics system (global variables)
PHYSACDEF void ClosePhysics(void);                                                                          // Close physics system and unload used memory
PHYSACDEF void SetPhysicsTimeStep(double delta);                                                            // Sets physics fixed time step in milliseconds. 1.666666 by default
PHYSACDEF void SetPhysicsGravity(float x, float y);                                                         // Sets physics global gravity force

// Physic body creation/destroy
PHYSACDEF PhysicsBody CreatePhysicsBodyCircle(Vector2 pos, float radius, float density);                    // Creates a new circle physics body with generic parameters
PHYSACDEF PhysicsBody CreatePhysicsBodyRectangle(Vector2 pos, float width, float height, float density);    // Creates a new rectangle physics body with generic parameters
PHYSACDEF PhysicsBody CreatePhysicsBodyPolygon(Vector2 pos, float radius, int sides, float density);        // Creates a new polygon physics body with generic parameters
PHYSACDEF void DestroyPhysicsBody(PhysicsBody body);                                                        // Destroy a physics body

// Physic body forces
PHYSACDEF void PhysicsAddForce(PhysicsBody body, Vector2 force);                                            // Adds a force to a physics body
PHYSACDEF void PhysicsAddTorque(PhysicsBody body, float amount);                                            // Adds an angular force to a physics body
PHYSACDEF void PhysicsShatter(PhysicsBody body, Vector2 position, float force);                             // Shatters a polygon shape physics body to little physics bodies with explosion force
PHYSACDEF void SetPhysicsBodyRotation(PhysicsBody body, float radians);                                     // Sets physics body shape transform based on radians parameter

// Query physics info
PHYSACDEF PhysicsBody GetPhysicsBody(int index);                                                            // Returns a physics body of the bodies pool at a specific index
PHYSACDEF int GetPhysicsBodiesCount(void);                                                                  // Returns the current amount of created physics bodies
PHYSACDEF int GetPhysicsShapeType(int index);                                                               // Returns the physics body shape type (PHYSICS_CIRCLE or PHYSICS_POLYGON)
PHYSACDEF int GetPhysicsShapeVerticesCount(int index);                                                      // Returns the amount of vertices of a physics body shape
PHYSACDEF Vector2 GetPhysicsShapeVertex(PhysicsBody body, int vertex);                                      // Returns transformed position of a body shape (body position + vertex transformed position)

#if defined(__cplusplus)
}
#endif

#endif // PHYSAC_H