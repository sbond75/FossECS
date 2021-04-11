// Based on https://gamedevelopment.tutsplus.com/tutorials/how-to-create-a-custom-2d-physics-engine-the-basics-and-impulse-resolution--gamedev-6331

#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <vector>
#include <algorithm>
#include <cassert>
#include <Bengine/DebugRenderer.h>
#include "../MainGame.h"

typedef glm::vec2 Vec2;

float Length(const Vec2& v) {
  return v.length();
}
float LengthSquared(const Vec2& v) {
  return v.x*v.x + v.y*v.y;
}

// "An Axis Aligned Bounding Box (AABB) is a box that has its four axes aligned with the coordinate system in which it resides. This means it is a box that cannot rotate, and is always squared off at 90 degrees (usually aligned with the screen). In general it is referred to as a "bounding box" because AABBs are used to bound other more complex shapes."
// "The AABB of a complex shape can be used as a simple test to see if more complex shapes inside the AABBs can possibly be intersecting. However in the case of most games the AABB is used as a fundamental shape, and does not actually bound anything else. The structure of your AABB is important. There are a few different ways to represent an AABB, however this is my favorite:"
struct AABB
{
  Vec2 min;
  Vec2 max;
};

// "Here's a quick test taken from Real-Time Collision Detection [ https://realtimecollisiondetection.net/ ] by Christer Ericson, which makes use of the SAT [Separating Axis Theorem]:"
bool AABBvsAABB( AABB a, AABB b )
{
  // Exit with no intersection if found separated along an axis
  if(a.max.x < b.min.x or a.min.x > b.max.x) return false;
  if(a.max.y < b.min.y or a.min.y > b.max.y) return false;
 
  // No separating axis found, therefor there is at least one overlapping axis
  return true;
}






struct Transform {
  Transform(float x_, float y_) : x(x_), y(y_) { assert(position.x == x_ && position.y == y_); }
  union { // Union for convenience: can access Transform.position.x or just Transform.x
    Vec2 position;
    struct {
      float x;
      float y;
    };
  };
  
  // Future: rotation: will be added here
};
#define member_size(type, member) sizeof(((type *)0)->member) // https://stackoverflow.com/questions/3553296/sizeof-single-struct-member-in-c
static_assert(offsetof(Transform, x) /* - (offsetof(Transform, anyPreviousMemberBeforeTheXMemberGoesHere) + member_size(Transform, anyPreviousMemberBeforeTheXMemberGoesHere)) */ == offsetof(Vec2, x), "Location of the `x` member of `Transform` struct should be the same as the actual location in `Transform` struct but when accessed from the Vec2 `position`");
static_assert(offsetof(Transform, y) - offsetof(Transform, x) == offsetof(Vec2, y), "Same as above, but for y");


struct MassData
{
  MassData(float mass = 1.0f) {
    setMass(mass);
  }
  void setMass(float mass) {
    this->mass = mass;
    inv_mass = 1.0f / mass;
  }
  float mass;
  float inv_mass;
 
  // For rotations (not covered in this article)
  float inertia;
  float inverse_inertia;
};

struct Material
{
  Material(float density_ = 1.0f, float restitution_ = 1.0f) : density(density_), restitution(restitution_) {}
  float density;
  float restitution;
};

struct Shape {
  Shape() {}
  static Shape&& makeCircle(float radius) {
    Shape s;
    s.type = Circle;
    s.circle_radius = radius;
    return std::move(s);
  }
  static Shape&& makeBox(Vec2 dimensions) {
    Shape s;
    s.type = Box;
    s.box_dimensions = dimensions;
    return std::move(s);
  }
  
  enum Type {
    Box, Circle
  };
  
  Type type;
  union {
    struct { // Valid only if `type` == Box
      Vec2 box_dimensions; // Width and height
    };
    struct { // Valid only if `type` == Circle
      float circle_radius;
    };
  };
};
#define SQUARED(x) x*x
float Distance( Vec2 a, Vec2 b )
{
  return sqrt( SQUARED(a.x - b.x) + SQUARED(a.y - b.y) );
}
// "An important optimization to make here is get rid of any need to use the square root operator:"
bool CirclevsCircleOptimized(Transform a, Transform b, float radiusA, float radiusB)
{
  float r = radiusA + radiusB;
  r *= r;
  return r < SQUARED(a.x + b.x) + SQUARED(a.y + b.y);
}


struct Body
{
  Body(Shape shape_, Material mat_ = Material(), Transform tx_ = Transform(0,0), MassData mass_data_= MassData()) :
    shape(shape_),
    tx(tx_),
    material(mat_),
    mass_data(mass_data_),
    velocity(),
    force(),
    gravityScale(1.0f),
    layers(INT_MAX)
  {
    
  }
  void ComputeAABB(AABB* out) {
    switch (shape.type) {
    case Shape::Box:
      out->min = tx.position - shape.box_dimensions / 2.0f;
      out->max = tx.position + shape.box_dimensions / 2.0f;
      break;
    case Shape::Circle:
      out->min = tx.position - shape.circle_radius;
      out->max = tx.position + shape.circle_radius;
      break;
    default:
      assert(false && "Unknown shape");
    }
  }
  Shape shape;
  Transform tx;
  Material material;
  MassData mass_data;
  Vec2 velocity;
  void move(float dt) {
    // Symplectic Euler
    velocity += (mass_data.inv_mass * force) * dt;
    tx.position += velocity * dt;
  }
  
  Vec2 force; // "This value starts at zero at the beginning of each physics update. Other influences in the physics engine (such as gravity) will add Vec2 vectors into this force data member. Just before integration all of this force will be used to calculate acceleration of the body, and be used during integration. After integration this force data member is zeroed out."
  float gravityScale;

  int layers; // A bitmask. "Layering refers to the act of having different objects never collide with one another." (Example: a bullet hits players on one team but not those on another.)
};

struct Pair
{
  Pair(Body *a, Body *b) { A = a; B = b; }
  Body *A;
  Body *B;
};

// "A broad phase should collect a bunch of possible collisions and store them all in Pair structures. These pairs can then be passed on to another portion of the engine (the narrow phase), and then resolved."


struct BroadPhase {
  std::vector<Pair> pairs;
  std::vector<Body> bodies;
  std::vector<Pair> uniquePairs;
  
  void GeneratePairs( void ); // Makes `uniquePairs`
};

void UpdatePhysics( BroadPhase* bp, float dt );
void RenderGame(BroadPhase* bp, Bengine::DebugRenderer& _debugRenderer, Bengine::GLSLProgram& _colorProgram, Bengine::Camera2D& _camera, Bengine::Window& _window, float alpha);
void gameLoopPhysics(Bengine::DebugRenderer& renderer, Bengine::GLSLProgram& _colorProgram, Bengine::Window& _window, Bengine::Camera2D& _camera, Bengine::FpsLimiter& _fpsLimiter, Bengine::InputManager& _inputManager) {
#define GetCurrentTime() SDL_GetTicks() / 1000.0f // Convert to seconds

  BroadPhase bp;
  GameState _gameState = GameState::PLAY;

  // Spawn objects
  auto makeBox = [&](Vec2 pos) {    
    Body b(Shape::makeBox(Vec2(10,10)), Material(), Transform(pos.x,pos.y), MassData());
    bp.bodies.push_back(b);
  };
  
  const float fps = 100;
  const float dt = 1 / fps;
  float accumulator = 0;
 
  // In units of seconds
  float frameStart = GetCurrentTime( );
  float _fps;
  Body* target; // Movement target
 
  // main loop
  while(_gameState != GameState::EXIT) {
    const float currentTime = GetCurrentTime( );
    _fpsLimiter.begin();

    auto processInput = [&](){
      SDL_Event evnt;

      const float CAMERA_SPEED = 2.0f;
      const float SCALE_SPEED = 0.1f;

      //Will keep looping until there are no more events to process
      bool nodeManagerConsumedInput = false; // Assume false.
      while (SDL_PollEvent(&evnt)) {
        switch (evnt.type) {
	case SDL_QUIT:
	  _gameState = GameState::EXIT;
	  break;
	case SDL_MOUSEMOTION:
	  _inputManager.setMouseCoords((float)evnt.motion.x, (float)evnt.motion.y);
	  break;
	case SDL_KEYDOWN:
	  _inputManager.pressKey(evnt.key.keysym.sym);
                
	  // // Forward any just-pressed (and not held) inputs to the NodeManager:
	  // if (_inputManager.isKeyPressed(evnt.key.keysym.sym)) {
	  //     nodeManagerConsumedInput = _nodeManager.receiveKeyPressed(evnt.key);
	  //     if (nodeManagerConsumedInput) {
	  //         // Mark this as not pressed in the inputManager so that uses of the input manager after this enclosing while loop will not consider it.
	  //         _inputManager.releaseKey(evnt.key.keysym.sym);
	  //     }
	  // }
                
	  break;
	case SDL_KEYUP:
	  _inputManager.releaseKey(evnt.key.keysym.sym);
	  break;
	case SDL_MOUSEBUTTONDOWN:
	  _inputManager.pressKey(evnt.button.button);
	  break;
	case SDL_MOUSEBUTTONUP:
	  _inputManager.releaseKey(evnt.button.button);
	  break;
        }
      }

      if (_inputManager.isKeyDown(SDLK_w)) {
        _camera.setPosition(_camera.getPosition() + glm::vec2(0.0f, CAMERA_SPEED)/_camera.getScale());
      }
      if (_inputManager.isKeyDown(SDLK_s)) {
        _camera.setPosition(_camera.getPosition() + glm::vec2(0.0f, -CAMERA_SPEED)/_camera.getScale());
      }
      if (_inputManager.isKeyDown(SDLK_a)) {
        _camera.setPosition(_camera.getPosition() + glm::vec2(-CAMERA_SPEED, 0.0f)/_camera.getScale());
      }
      if (_inputManager.isKeyDown(SDLK_d)) {
        _camera.setPosition(_camera.getPosition() + glm::vec2(CAMERA_SPEED, 0.0f)/_camera.getScale());
      }
      if (_inputManager.isKeyDown(SDLK_q)) {
        _camera.setScale(_camera.getScale() + SCALE_SPEED*_camera.getScale());
      }
      if (_inputManager.isKeyDown(SDLK_e)) {
        _camera.setScale(_camera.getScale() - SCALE_SPEED*_camera.getScale());
      }

      if (_inputManager.isKeyDown(SDL_BUTTON_LEFT)) {
        glm::vec2 mouseCoords = _inputManager.getMouseCoords();
        mouseCoords = _camera.convertScreenToWorld(mouseCoords);
        
        // glm::vec2 playerPosition(0.0f);
        // glm::vec2 direction = mouseCoords - playerPosition;
        // direction = glm::normalize(direction);

	// Get object at mouse
	if (target == nullptr) {
	  for (Body& b : bp.bodies) {
	    AABB a;
	    b.ComputeAABB(&a);
	    if (AABBvsAABB(a, {mouseCoords, mouseCoords})) {
	      // Move this object
	      target = &b;
	    }
	  }
	}
      }
      else {
	target = nullptr; // Stop moving any object
      }
      
      if (_inputManager.isKeyDown(SDL_BUTTON_RIGHT)) {
	glm::vec2 mouseCoords = _inputManager.getMouseCoords();
        mouseCoords = _camera.convertScreenToWorld(mouseCoords);
	
        makeBox(mouseCoords);
      }

      // Check for mouse moving object
      if (target != nullptr) {
        glm::vec2 mouseCoords = _inputManager.getMouseCoords();
        mouseCoords = _camera.convertScreenToWorld(mouseCoords);
	
	target->tx.position = mouseCoords;
      }
    };
    processInput();
  
    // Store the time elapsed since the last frame began
    accumulator += currentTime - frameStart;
 
    // Record the starting of this frame
    frameStart = currentTime;

    // Avoid spiral of death and clamp dt, thus clamping
    // how many times the UpdatePhysics can be called in
    // a single game loop.
    if(accumulator > 0.2f)
      accumulator = 0.2f;
 
    while(accumulator > dt) {
      UpdatePhysics( &bp, dt );
      accumulator -= dt;
    }
 
    const float alpha = accumulator / dt;

    
    _camera.update();
    
    RenderGame( &bp, renderer, _colorProgram, _camera, _window, alpha );
    
    _fps = _fpsLimiter.end();
    
    //print only once every 60 frames
    static int frameCounter = 0;
    frameCounter++;
    if (frameCounter == 60) {
      std::cout << _fps << std::endl;
      frameCounter = 0;
    }
  }
}

// "The last topic to cover in this article is simple manifold generation. A manifold in mathematical terms is something along the lines of "a collection of points that represents an area in space". However, when I refer to the term manifold I am referring to a small object that contains information about a collision between two objects."
struct Manifold
{
  Body *A;
  Body *B;
  // "During collision detection, both penetration and the collision normal should be computed. In order to find this info the original collision detection algorithms from the top of this article must be extended."
  float penetration;
  Vec2 normal;
};

float DotProduct(const Vec2& a, const Vec2& b) {
  return glm::dot(a, b);
}

// For when two objects collide but they will separate next frame anyway: "Two objects collide, but velocity will separate them next frame. Do not resolve this type of collision."
// "If objects are moving away from one another we want to do nothing. This will prevent objects that shouldn't actually be considered colliding from resolving away from one another. This is important for creating a simulation that follows human intuition on what should happen during object interaction."
void ResolveCollision_other(Manifold* m) {
  Body* A = m->A;
  Body* B = m->B;
  
  // Calculate relative velocity
  Vec2 rv = B->velocity - A->velocity;
  
  // Calculate relative velocity in terms of the normal direction
  float velAlongNormal = DotProduct( rv, m->normal );
  
  // Do not resolve if velocities are separating
  if(velAlongNormal > 0)
    return;
 
  // Calculate restitution
  float e = std::min( A->material.restitution, B->material.restitution);
 
  // Calculate impulse scalar
  float j = -(1 + e) * velAlongNormal;
  j /= A->mass_data.inv_mass + B->mass_data.inv_mass;
  
  float mass_sum = A->mass_data.mass + B->mass_data.mass;
  float ratio = A->mass_data.mass / mass_sum;
  Vec2 impulse = j * m->normal;
  A->velocity -= ratio * impulse;
 
  ratio = B->mass_data.mass / mass_sum;
  B->velocity += ratio * impulse;
}

bool CirclevsCircle( Manifold *m )
{
  // Setup a couple pointers to each object
  Body *A = m->A;
  Body *B = m->B;
 
  // Vector from A to B
  Vec2 n = B->tx.position - A->tx.position;
 
  float r = A->shape.circle_radius + B->shape.circle_radius;
  r *= r;
 
  if(LengthSquared( n ) > r)
    return false;
 
  // Circles have collided, now compute manifold
  float d = Length( n ); // perform actual sqrt
 
  // If distance between circles is not zero
  if(d != 0)
  {
    // Distance is difference between radius and distance
    m->penetration = r - d;
 
    // Utilize our d since we performed sqrt on it already within Length( )
    // Points from A to B, and is a unit vector
    m->normal = n / d;
    return true;
  }
 
  // Circles are on same position
  else
  {
    // Choose random (but consistent) values
    m->penetration = A->shape.circle_radius;
    m->normal = Vec2( 1, 0 );
    return true;
  }
}

bool AABBvsAABB( Manifold *m )
{
  // Setup a couple pointers to each object
  Body *A = m->A;
  Body *B = m->B;
  
  // Vector from A to B
  Vec2 n = B->tx.position - A->tx.position;
  
  AABB abox; A->ComputeAABB(&abox);
  AABB bbox; A->ComputeAABB(&bbox);
  
  // Calculate half extents along x axis for each object
  float a_extent = (abox.max.x - abox.min.x) / 2;
  float b_extent = (bbox.max.x - bbox.min.x) / 2;
  
  // Calculate overlap on x axis
  float x_overlap = a_extent + b_extent - abs( n.x );
  
  // SAT test on x axis
  if(x_overlap > 0)
  {
    // Calculate half extents along x axis for each object
    float a_extent = (abox.max.y - abox.min.y) / 2;
    float b_extent = (bbox.max.y - bbox.min.y) / 2;
  
    // Calculate overlap on y axis
    float y_overlap = a_extent + b_extent - abs( n.y );
  
    // SAT test on y axis
    if(y_overlap > 0)
    {
      // Find out which axis is axis of least penetration
      if(x_overlap > y_overlap)
      {
        // Point towards B knowing that n points from A to B
        if(n.x < 0)
          m->normal = Vec2( -1, 0 );
        else
          m->normal = Vec2( 0, 0 );
	m->penetration = x_overlap;
        return true;
      }
      else
      {
        // Point toward B knowing that n points from A to B
        if(n.y < 0)
          m->normal = Vec2( 0, -1 );
        else
          m->normal = Vec2( 0, 1 );
        m->penetration = y_overlap;
        return true;
      }
    }
  }

  return false;
}

bool AABBvsCircle( Manifold *m )
{
  // Setup a couple pointers to each object
  Body *A = m->A;
  Body *B = m->B;
 
  // Vector from A to B
  Vec2 n = B->tx.position - A->tx.position;
 
  // Closest point on A to center of B
  Vec2 closest = n;
 
  AABB abox; A->ComputeAABB(&abox);
  AABB bbox; A->ComputeAABB(&bbox);
  
  // Calculate half extents along each axis
  float x_extent = (abox.max.x - abox.min.x) / 2;
  float y_extent = (abox.max.y - abox.min.y) / 2;
 
  // Clamp point to edges of the AABB
  closest.x = glm::clamp( closest.x, -x_extent, x_extent );
  closest.y = glm::clamp( closest.y, -y_extent, y_extent );
 
  bool inside = false;
 
  // Circle is inside the AABB, so we need to clamp the circle's center
  // to the closest edge
  if(n == closest)
  {
    inside = true;
 
    // Find closest axis
    if(abs( n.x ) > abs( n.y ))
    {
      // Clamp to closest extent
      if(closest.x > 0)
        closest.x = x_extent;
      else
        closest.x = -x_extent;
    }
 
    // y axis is shorter
    else
    {
      // Clamp to closest extent
      if(closest.y > 0)
        closest.y = y_extent;
      else
        closest.y = -y_extent;
    }
  }
 
  Vec2 normal = n - closest;
  float d = LengthSquared( normal );
  float r = B->shape.circle_radius;
 
  // Early out of the radius is shorter than distance to closest point and
  // Circle not inside the AABB
  if(d > r * r && !inside)
    return false;
 
  // Avoided sqrt until we needed
  d = sqrt( d );
 
  // Collision normal needs to be flipped to point outside if circle was
  // inside the AABB
  if(inside)
  {
    m->normal = -n;
    m->penetration = r - d;
  }
  else
  {
    m->normal = n;
    m->penetration = r - d;
  }
 
  return true;
}

void PositionalCorrectionWithSlop( Manifold* m )
{
  Body* A = m->A;
  Body* B = m->B;
  const float percent = 0.2; // usually 20% to 80%
  const float k_slop = 0.01; // usually 0.01 to 0.1
  // "This allows objects to penetrate ever so slightly without the position correction kicking in."
  // "During collision detection, both penetration and the collision normal should be computed. In order to find this info the original collision detection algorithms from the top of this article must be extended."
  Vec2 correction = (std::max( m->penetration - k_slop, 0.0f ) / (A->mass_data.inv_mass + B->mass_data.inv_mass)) * percent * m->normal;
  A->tx.position -= A->mass_data.inv_mass * correction;
  B->tx.position += B->mass_data.inv_mass * correction;
}
void UpdatePhysics( BroadPhase* bp, float dt ) {
  bp->GeneratePairs();

  // Could be parallelized with the above!
  for (Body& b : bp->bodies) {
    // Apply movement
    b.move(dt);
  }
  
  // Go through the generated pairs and process collisions
  Manifold m{nullptr, nullptr, 0.0f, Vec2(0,0)};
  for (Pair& p : bp->uniquePairs) {
    m.A = p.A;
    m.B = p.B;

    // Compute collision info
    switch (m.A->shape.type) {
    case Shape::Box:
      switch (m.B->shape.type) {
      case Shape::Box:
	AABBvsAABB(&m);
	break;
      case Shape::Circle:
	AABBvsCircle(&m);
	break;
      }
      break;
    case Shape::Circle:
      switch (m.B->shape.type) {
      case Shape::Box:
	std::swap(m.A, m.B); // Make A be a box, B be a circle, according to the Manifold.
	AABBvsCircle(&m);
	std::swap(m.A, m.B); // Undo that change from above
	break;
      case Shape::Circle:
	CirclevsCircle(&m);
	break;
      }
      break;
    }
    
    // Resolve collisions that are not directly at each other
    ResolveCollision_other(&m);
    
    // Linear projection reduces the penetration of two objects by a small percentage, and this is performed after the impulse is applied.
    PositionalCorrectionWithSlop(&m);
  }
}

void RenderGame(BroadPhase* bp, Bengine::DebugRenderer& _debugRenderer, Bengine::GLSLProgram& _colorProgram, Bengine::Camera2D& _camera, Bengine::Window& _window, float alpha) {
  // for shape : game.shapes {
  //   // calculate an interpolated transform for rendering
  //   Transform i = shape.previous * alpha + shape.current * (1.0f - alpha);
  //   shape.previous = shape.current;
  //   shape.Render( i );
  // }

    //Set the base depth to 1.0
    glClearDepth(1.0);
    //Clear the color and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //Enable the shader
    _colorProgram.use();

    //We are using texture unit 0
    glActiveTexture(GL_TEXTURE0);
    //Get the uniform location
    GLint textureLocation = _colorProgram.getUniformLocation("mySampler");
    //Tell the shader that the texture is in texture unit 0
    glUniform1i(textureLocation, 0);

    //Set the camera matrix
    GLint pLocation = _colorProgram.getUniformLocation("P");
    glm::mat4 cameraMatrix = _camera.getCameraMatrix();

    glUniformMatrix4fv(pLocation, 1, GL_FALSE, &(cameraMatrix[0][0]));
    
  AABB a;
  for (Body& body : bp->bodies) {
    switch (body.shape.type) {
    case Shape::Box:
      body.ComputeAABB(&a);
      _debugRenderer.drawBox({a.min.x, a.min.y, a.max.x - a.min.x, a.max.y - a.min.y}, Bengine::ColorRGBA8(255,255,255,255), 0);
      break;
    case Shape::Circle:
      _debugRenderer.drawCircle(body.tx.position, Bengine::ColorRGBA8(255,255,255,255), body.shape.circle_radius);
      break;
    }
  }
  
    _debugRenderer.end();
    _debugRenderer.render(cameraMatrix, 2);
    
    //unbind the texture
    glBindTexture(GL_TEXTURE_2D, 0);

    //disable the shader
    _colorProgram.unuse();

    //Swap our buffer and draw everything to the screen!
    _window.swapBuffer();
}




// MARK: Broad phase //

bool SortPairs( Pair lhs, Pair rhs )
{
  if(lhs.A < rhs.A)
    return true;
 
  if(lhs.A == rhs.A)
    return lhs.B < rhs.B;
 
  return false;
}
// Example O(n^2) broad phase:
// Generates the pair list.
// All previous pairs are cleared when this function is called.
void BroadPhase::GeneratePairs( void )
{
  pairs.clear( );
 
  // Cache space for AABBs to be used in computation
  // of each shape's bounding box
  AABB A_aabb;
  AABB B_aabb;
 
  for(auto i = bodies.begin( ); i != bodies.end( ); ++i)
  {
    for(auto j = bodies.begin( ); j != bodies.end( ); ++j)
    {
      Body *A = &*i;
      Body *B = &*j;
 
      // Skip check with self
      if(A == B)
        continue;
      
      // Only matching layers will be considered
      if(!(A->layers & B->layers))
        continue;
 
      A->ComputeAABB( &A_aabb );
      B->ComputeAABB( &B_aabb );
 
      if(AABBvsAABB( A_aabb, B_aabb ))
        pairs.emplace_back( A, B );
    }
  }

  // Need to remove duplicates //
  
  // Sort pairs to expose duplicates
  std::sort( pairs.begin(), pairs.end( ), SortPairs );
 
  // Queue manifolds for solving
  {
    int i = 0;
    while(i < pairs.size( ))
      {
	Pair* pair = &*(pairs.begin( ) + i);
	uniquePairs.push_back( *pair );
 
	++i;
 
	// Skip duplicate pairs by iterating i until we find a unique pair
	while(i < pairs.size( ))
	  {
	    Pair *potential_dup = &*(pairs.begin() + i);
	    if(pair->A != potential_dup->B || pair->B != potential_dup->A)
	      break;
	    ++i;
	  }
      }
  }
}
