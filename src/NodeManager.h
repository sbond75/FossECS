#pragma once

#include <Bengine/Bengine.h>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <Bengine/DebugRenderer.h>
#include <iostream>
#include "HashPair.h"
#include <unordered_map>
#include <functional>
#include <Bengine/Timing.h>

#include <ginac/ginac.h>

// https://stackoverflow.com/questions/321351/initializing-a-union-with-a-non-trivial-constructor
#include <new> // Required for placement 'new'.

#include <glm/gtx/rotate_vector.hpp>

typedef GiNaC::numeric real; // Real number type (instead of `double`, we have arbitrary precision using GMP (the GNU Multiple Precision library) ( https://www.ginac.de/tutorial/#Numbers )

enum Direction {
    NoDirection, Left, Right, Up, Down
};

static Direction invertDirection(Direction dir) {
    switch (dir) {
    case Left:
        return Right;
    case Right:
        return Left;
    case Up:
        return Down;
    case Down:
        return Up;
    case NoDirection:
        return NoDirection;
    }
}

struct Node;
struct NodeOperation {
    enum Type {
        AddNode
    };
    Type type;
    union {
        struct {
            Direction addNode_direction;
            Node* addNode_from;
            Node* addNode_to;
        };
    };
};

// https://stackoverflow.com/questions/13462001/ease-in-and-ease-out-animation-formula
// `t` goes from 0 to 1.
// Returns a value from 0 to 1.
static float BezierBlend(float t)
{
    return t * t * (3.0f - 2.0f * t);
}

// https://stackoverflow.com/questions/4353525/floating-point-linear-interpolation
// "To do a linear interpolation between two variables a and b given a fraction f":
static float lerp(float a, float b, float f)
{
    return a + f * (b - a);
}

// https://github.com/processing/processing/blob/be7e25187b289f9bfa622113c400e26dd76dc89b/core/src/processing/core/PApplet.java#L5061 , https://stackoverflow.com/questions/43683041/c-equivalent-of-processings-map-function
static float map(float value, float start1, float stop1, float start2, float stop2) {
    float outgoing =
        start2 + (stop2 - start2) * ((value - start1) / (stop1 - start1));
    return outgoing;
}

// This is sort of an "edge" from graph theory. But in circuits this is a "branch," or put simply, a "circuit element" like a resistor.
struct NodeBranch {
    NodeBranch() : type(None) {}
    ~NodeBranch() {
        switch (type) {
        case None:
            break;
        case Resistor:
            resistor_resistance.~symbol(); // Call destructor manually ( https://stackoverflow.com/questions/40106941/is-a-union-members-destructor-called )
            break;
        }
    }
    void reset() {
        std::cout << this << std::endl;
        this->~NodeBranch();
        type = None;
    }
    
    enum Type {
        None, // No branch present
        Resistor // A resistor on an electric circuit
    };
    Type type;
    
    union {
        struct {
            GiNaC::symbol resistor_resistance;
        };
    };
    GiNaC::symbol current;
};

struct NodeJunctionDetails {
    GiNaC::symbol nodeVoltage; // Keep?
};

struct Node {
    static constexpr float separationBetweenNodes = 100.0f; // Distance between each node.

    static Uint32 ticksWhenSelectionAnimationStartedToStop; // Used for animation in `draw()`.
    static Uint32 ticksWhenSelectionAnimationStartedToStart; // Used for animation in `draw()`.
    static float lerpAcc; // Accumulator for lerping (linear interpolation); used for animation in `draw()`.

    NodeBranch branchDetails; // Details about the branch ("edge" from graph theory), if any, from this node to `originNode`.
    NodeJunctionDetails juncDetails; // Details about the node as a junction in an electric circuit, aka a "node" in a circuit or graph theory. For example, this could be a node voltage?
    
    glm::ivec2 position2D; // This node's position as multiples of `separationBetweenNodes`.

    Direction cycleDirection = NoDirection; Node* cycleNode = nullptr; // For when a node forms a cycle, `cycleNode` is the node that is joined from `this` to the `cycleNode`.

    Direction originatingDirection = NoDirection; Node* originNode = nullptr; // The "origin" node is the node connected to us who spawned us. When about to create nodes in the same direction as this origin node, we should not spawn a node. Instead we go back to that node and make it the "selection" by setting its `active` node to nullptr.
    Node* up = nullptr, *down = nullptr, *left = nullptr, *right = nullptr, *active = nullptr;
    Direction activeNodeDirection = NoDirection;
    ~Node() {
        // Recursively delete all nodes (deleting nullptr is ok in most platforms...)
        delete up;
        up = nullptr;
        delete down;
        down = nullptr;
        delete left;
        left = nullptr;
        delete right;
        right = nullptr;
    }

    void activate(Node* node, Direction dirOfNode) {
        active = node;
        activeNodeDirection = dirOfNode;
    }

    Node** nodeForDirection(Direction dir) {
        Node** dest = nullptr;
        switch (dir) {
        case Left:
            dest = &left;
            break;
        case Right:
            dest = &right;
            break;
        case Up:
            dest = &up;
            break;
        case Down:
            dest = &down;
            break;
        case NoDirection:
            return nullptr;
        }
        return dest;
    }

    static glm::ivec2 ivec2ForDirection(Direction dir) {
        glm::ivec2 dest(0,0);
        switch (dir) {
        case Left:
            dest = {-1,0};
            break;
        case Right:
            dest = {1,0};
            break;
        case Up:
            dest = {0,1};
            break;
        case Down:
            dest = {0,-1};
            break;
        case NoDirection:
            break;
        }
        return dest;
    }

    // thisNode2DPosition is the offset of `this` node from the root node in terms of 2D grid coordinates. In the real editor, each of these integer positions (0, 1, 2, etc.) is separated by `separationBetweenNodes` pixels.
    void move(Direction dir, std::function<bool(glm::ivec2 fromNode2DPosition, Direction dir, Node* from)> shouldCreateNode, std::function<void(glm::ivec2 fromNode2DPosition, Direction dir, Node* from, Node* to)> onCreateNode, std::function<void(Node*)> onNavigateToNode, glm::ivec2 thisNode2DPosition={0,0}) {
        assert(position2D == thisNode2DPosition);
        
        if (active) {
            active->move(dir, shouldCreateNode, onCreateNode, onNavigateToNode, thisNode2DPosition + ivec2ForDirection(activeNodeDirection)); // Forward it to the active node.
            return;
        }

        if (dir == NoDirection) return;
        Node** dest = nodeForDirection(dir);
        if (!*dest) {
            if (!shouldCreateNode(thisNode2DPosition, dir, this)) {
                return;
            }
            if (dir == originatingDirection && originNode) {
                // Then we need to re-use this node instead of spawning one:
                originNode->active = nullptr;
                originNode->activeNodeDirection = NoDirection;
                onNavigateToNode(originNode);
                return;
            }
            *dest = new Node();
            (*dest)->originNode = this;
            (*dest)->originatingDirection = invertDirection(dir);
            (*dest)->position2D = thisNode2DPosition + ivec2ForDirection(dir);
            onNavigateToNode(*dest);

            onCreateNode(thisNode2DPosition, dir, this, *dest);
        }
        else {
            onNavigateToNode(*dest);
        }
        activate(*dest, dir);
    }

    // `forceNoActiveDrawing` is to make it so recursion stops drawing once we already found who is the first non-active node
public:
    // `selectedNode` is the highlighted node. It can be found by traversing the linked list formed by the `active` pointers in each `Node` (as done in `move` actually..), but this would be slow so we pass it here.
    void draw(Bengine::DebugRenderer& renderer, const Bengine::FpsLimiter& fpsInfo, const glm::vec2& translation, Node* selectedNode) {
        bool forceNoActiveDrawing = false;
        _draw(renderer, fpsInfo, translation, forceNoActiveDrawing, selectedNode);
    }
private:
    void _draw(Bengine::DebugRenderer& renderer, const Bengine::FpsLimiter& fpsInfo, const glm::vec2& translation, bool& forceNoActiveDrawing, Node* selectedNode) {
        // Teal: {0,128,128,255}
        Bengine::ColorRGBA8 color = {0,200,200,170}; // Bright teal
        Bengine::ColorRGBA8 colorBright = {0,255,170,250}; // Brighter teal-like color
        Bengine::ColorRGBA8 red = {150,30,100,200};
        
        // Draw the branch (could be just a line, or a resistor, etc.)
        std::function<void(const glm::vec2& pos1, const glm::vec2& pos2, const Bengine::ColorRGBA8& color)> drawBranch;
        switch (branchDetails.type) {
        case NodeBranch::None:
            drawBranch = [&renderer](const glm::vec2& pos1, const glm::vec2& pos2, const Bengine::ColorRGBA8& color) {
                renderer.drawLine(pos1, pos2, color);
            };
            break; // Use `drawBranch` defined above
        case NodeBranch::Resistor:
            // Draw resistor
            drawBranch = [&renderer](const glm::vec2& pos1, const glm::vec2& pos2, const Bengine::ColorRGBA8& color) {
                float d = glm::distance(pos1, pos2);
                glm::vec2 unit = glm::normalize(pos2 - pos1);
                
                glm::vec2 currentPos = pos1;
                glm::vec2 nextPos;
                float currentSign = 1.0f;
                float scale = 19.0f;
                //for (float i = 0; i < d; i += scale /*glm::distance(currentPos, nextPos)*/) {
                while (glm::distance(currentPos, pos1) < d) {
                    nextPos = glm::rotate(unit, 45.0f * currentSign) * scale;
                    renderer.drawLine(currentPos, currentPos + nextPos, color);
                    currentPos += nextPos;
                    currentSign = -currentSign;
                }
            };
            break;
        }
        
        // Recursively draw the other nodes connected to this one:
        glm::vec2 dest;
        if (up) {
            dest = {translation.x, translation.y + separationBetweenNodes};
            drawBranch(translation, dest, color);
            up->_draw(renderer, fpsInfo, dest, forceNoActiveDrawing, selectedNode);
        }
        if (down) {
            dest = {translation.x, translation.y - separationBetweenNodes};
            drawBranch(translation, dest, color);
            down->_draw(renderer, fpsInfo, dest, forceNoActiveDrawing, selectedNode);
        }
        if (left) {
            dest ={translation.x - separationBetweenNodes, translation.y};
            drawBranch(translation, dest, color);
            left->_draw(renderer, fpsInfo, dest, forceNoActiveDrawing, selectedNode);
        }
        if (right) {
            dest = {translation.x + separationBetweenNodes, translation.y};
            drawBranch(translation, dest, color);
            right->_draw(renderer, fpsInfo, dest, forceNoActiveDrawing, selectedNode);
        }
        
        // Draw ourselves as the selected node if we have no more `active` nodes to recurse to:
        if (selectedNode == this) {
            // Active node
            float sizeMultiplier = 1.0f;
#define computeSizeMultiplier(ticks) 0.5f + BezierBlend(fabsf(sin(ticks/1000.0f)))
            // Only show the node animation if we didn't make any nodes yet
            //std::cout << (left == right && right == up && up == down && down == nullptr) << std::endl;
            if (left == right && right == up && up == down && down == originNode && originNode == nullptr) { // If all these pointers are nullptr, then:
                ticksWhenSelectionAnimationStartedToStop = 0; // Reset timer for stopping since we are now starting and not stopping
                
                Uint32 end = SDL_GetTicks();
                
                // Initialize ticks for animation starting
                if (ticksWhenSelectionAnimationStartedToStart == 0) {
                    ticksWhenSelectionAnimationStartedToStart = SDL_GetTicks(); 
                }

                static bool ranOnce = false;
                Uint32 start = ticksWhenSelectionAnimationStartedToStart;
                Uint32 interval = 1000;
                if (end - start >= interval) {
                    // Start animating normally since we reached the destination time
                    sizeMultiplier = computeSizeMultiplier(end);
                    if (!ranOnce) {
                        std::cout << "Done with started to start animation: " << start + interval << " " << end << std::endl;
                        ranOnce = true;
                    }
                }
                else {
                    ranOnce = false;
                    
                    // Lerp into position, starting from where we left off (lerpAcc, which could be 1.0f if we completed the "starting to stop" animation, or any other value if not)
                    Uint32 goal = start + interval;
                    lerpAcc = computeSizeMultiplier(map(end, start, goal, 0, 1) + (1.0f- map(goal, start, goal, 0, 1))); /* Weighted average where the weights are based off the time to get from now(end) to the goal */
                    sizeMultiplier = lerpAcc;
                }
            }
            else if (ticksWhenSelectionAnimationStartedToStop == 0) {
                // Initialize `ticksWhenSelectionAnimationStartedToStop`:
                ticksWhenSelectionAnimationStartedToStop = SDL_GetTicks();
                lerpAcc = computeSizeMultiplier(SDL_GetTicks());

                // We started stopping, so reset the started starting animation:
                ticksWhenSelectionAnimationStartedToStart = 0;
            }

            if (ticksWhenSelectionAnimationStartedToStop != 0) {
                // Before we reach ticksWhenSelectionAnimationStartedToStop + 1000, we need to animate to full size.
                // UPDATE: the overflow actually isn't an issue as long as you always use end - start >= interval. Because negative numbers aren't happening with Uint32! That's why. Say `end` overflows (`end` is the current time): then it might be 10. 10 - 9999999 = negative number IF using signed ints, but we are using unsigned so it will be a large positive number and therefore will be greater than `interval` unless `interval` is extremely, extremely big. ( https://devblogs.microsoft.com/oldnewthing/20050531-22/?p=35493 )
                //if (ticksWhenSelectionAnimationStartedToStop + 1000 - SDL_GetTicks() <= 0) { // Side note: if SDL_GetTicks() "wraps" (overflows), then this animation plays almost forever...

                Uint32 end = SDL_GetTicks();
                Uint32 start = ticksWhenSelectionAnimationStartedToStop;
                Uint32 interval = 1000;
                if (end - start >= interval) { // https://www.reddit.com/r/gamedev/comments/245cx8/sdl_getticks_overflow_workaround/

                    // We reached the destination time, so stop animating
                    sizeMultiplier = 1.0f;
                }
                else {
                    // Animate to the destination time
                    lerpAcc = lerp(lerpAcc, 1.0f, 0.1);
                    sizeMultiplier = lerpAcc;
                }
                // [Nvm:] The only way to prevent overflow seems to be to use an accumulator:
                //fpsInfo.getFrameTime();
            }
            
            renderer.drawCircle(translation, colorBright, 20*sizeMultiplier*0.5f);
        }
        // Regular node drawing
        renderer.drawCircle(translation, color, 10);
        
        // Draw cycle node if any
        if (cycleNode) {
            dest = glm::vec2{translation.x, translation.y} + glm::vec2(ivec2ForDirection(cycleDirection))*separationBetweenNodes;
            renderer.drawLine(translation, dest, red);
        }
    }

public:
};

class NodeManager {
    Node root;
    std::vector<NodeOperation> undoStack;
    std::vector<NodeOperation> redoStack;
    std::unordered_map<std::pair<int, int>, Node*, hash_tuple::pair_hash> nodeAt2DPoint;
    Node* selectedNode;
    struct InputState {
        enum {
            Normal, // Standard, top-level state
            TypingName,
            TypingValue
        } state = Normal;
        
        union {
            struct {
                Node* typing_target; // Target object for typing a name or value to go to.
            };
        };

        // Temporary storage for as the user types in the names and values of something.
        std::string tempName;
        std::string tempValue;
    };
    InputState inputState; // Like a finite state machine, this is the state of the NodeManager's interpretation of the current keyboard input being supplied. For example, if you just pressed "R" to make a resistor out of a node and its origin node, this `inputState` will ensure that pressing letters on the keyboard will now correspond to entering in the resistor's name or value.
    
public:
    NodeManager() : selectedNode(&root) {}
    
    // Receives any just-pressed keys (not held).
    void receiveKeyPressed(SDL_KeyboardEvent keyEvent) {
        auto shouldCreateNode = [this](glm::ivec2 fromNode2DPosition, Direction dir, Node* from) -> bool {      
            // Get the direction of the new node relative to the "from node" (`fromNode2DPosition`):
            glm::ivec2 toNode2DPosition = fromNode2DPosition + Node::ivec2ForDirection(dir);
            
            // Set the corresponding spot in the map, checking if a spot exists there already and isn't where we just came from like by moving back along where we came, and if so then we create a link for a cycle:
            if (from->originatingDirection == dir) return true;
            auto it = nodeAt2DPoint.find({toNode2DPosition.x, toNode2DPosition.y});
            if (it == nodeAt2DPoint.end()) {
                // Not found, so insert it with no cycle
                return true;
            }
            else {
                // Found a cycle: "connect" to an existing node
                from->cycleNode = it->second;
                from->cycleDirection = dir;
                return false;
            }
        };
        auto onCreateNode = [this](glm::ivec2 fromNode2DPosition, Direction dir, Node* from, Node* to) {
            // Push to undo stack
            undoStack.emplace_back();
            undoStack.back().type = NodeOperation::Type::AddNode;
            undoStack.back().addNode_direction = dir;
            undoStack.back().addNode_from = from;
            undoStack.back().addNode_to = to;

            // Get the direction of the new node relative to the "from node" (`fromNode2DPosition`):
            glm::ivec2 toNode2DPosition = fromNode2DPosition + Node::ivec2ForDirection(dir);
            
            // Insert into the map
            nodeAt2DPoint[{toNode2DPosition.x, toNode2DPosition.y}] = to;
        };
        auto onNavigateToNode = [this](Node* node) {
            selectedNode = node;
        };

        SDL_Keycode key = keyEvent.keysym.sym;
    scan:
        switch (inputState.state) {
        case InputState::Normal:
            switch (key) {
            case SDLK_LEFT:
                root.move(Left, shouldCreateNode, onCreateNode, onNavigateToNode);
                break;
            case SDLK_RIGHT:
                root.move(Right, shouldCreateNode, onCreateNode, onNavigateToNode);
                break;
            case SDLK_UP:
                root.move(Up, shouldCreateNode, onCreateNode, onNavigateToNode);
                break;
            case SDLK_DOWN:
                root.move(Down, shouldCreateNode, onCreateNode, onNavigateToNode);
                break;
            case SDLK_u: // Undo
                undo();
                break;
            case SDLK_r: // Spawn resistor going from selected node to its origin node
                // Workflow: press r, then type a value for the resistor's resistance (or if you press _ then type a name too at any point along the way -- i.e. at any time, _ will switch to NAME entering instead of value entering), then arrow keys to continue on spawning nodes.
                Node* o;
                if ((o = selectedNode->originNode)) {
                    o->branchDetails.reset(); // Call dtor
                    o->branchDetails.type = NodeBranch::Resistor;
                    new (&o->branchDetails.resistor_resistance) GiNaC::symbol("r"); // We need placement new because otherwise we will call the dtor on uninitialized memory as part of assignment (to destruct the previous object before assigning to it).
                    inputState.state = InputState::TypingValue;
                    inputState.typing_target = o;
                }
            case SDLK_c: // Set current value or variable
            default:
                break;
            }
            break;
        case InputState::TypingValue:
            // Numbers will keep us in this state, and letters or underscores switch to TypingName.
            if (key >= SDLK_0 && key <= SDLK_9) {
                // Type in the value
                inputState.tempValue += key;
            }
            else if ((key >= SDLK_a && key <= SDLK_z) || key == SDLK_UNDERSCORE) {
                inputState.state = InputState::TypingName;
            }
            else {
                inputState.state = InputState::Normal;
                goto scan; // Process this event again under a different state
            }
            break;
        case InputState::TypingName:
            // Letters and numbers will keep us in this state.
            std::cout << "ASD:" << inputState.tempName << std::endl;
            if ((key >= SDLK_a && key <= SDLK_z) || (key >= SDLK_0 && key <= SDLK_9)) {
                // Letter or number
                inputState.tempName += key;
            }
            else if (key == SDLK_UNDERSCORE) {
                inputState.state = InputState::TypingValue;
            }
            else {
                inputState.state = InputState::Normal;
                goto scan; // Process this event again under a different state
            }
            break;
        }
    }

    // Next: add cycles where you move up to another branch using arrow keys and it creates a cycle which is indicated by a red line
    // Next: add indicator (like a blue expanding and shrinking orb that does this repeatedly) for your current position/`active` node
    // Next: arrow keys to show where you can go and what button you can press at any moment, right next to your cursor: https://www.google.com/search?q=arrow+keys+symbol&sxsrf=ALeKk00MxqraaGBrIsWDLnsAMV-hSJ-7pg:1616131487763&source=lnms&tbm=isch&sa=X&ved=2ahUKEwjyzvP5zrvvAhV2Ap0JHVjlBcQQ_AUoAXoECBQQAw&biw=1280&bih=676
    // Next: R to add a resistor, formed between your `origin` node and the current `active` node. Make a node type for resistor.
    
    void undo() {
        // [FIXED_TODO]: immediate: THIS causes segfaults if you undo sometimes...  probably dangling ptrs?
        if (undoStack.empty()) return;
        const NodeOperation& op = undoStack.back();
        switch (op.type) {
        case NodeOperation::AddNode:
            // Perform the inverse operation by removing the last `_to` node:
            Node** from = op.addNode_from->nodeForDirection(op.addNode_direction);
            assert(*from == op.addNode_to);
            if (*from != op.addNode_to) {
                std::cout << "Assertion failed (manual check)" << std::endl;
                exit(1);
            }
            op.addNode_from->active = nullptr;
            op.addNode_from->activeNodeDirection = NoDirection;
            if (op.addNode_to == selectedNode) // Move our selected node back before the `_to` node so that we don't reference deleted memory once we `delete op.addNode_to;` below:
                selectedNode = op.addNode_from;
            *from = nullptr;

            // Remove the node from the map
            const glm::ivec2& toNode2DPosition = op.addNode_to->position2D;
            nodeAt2DPoint.erase({toNode2DPosition.x, toNode2DPosition.y});
            
            delete op.addNode_to;
        }
        undoStack.pop_back();
    }

    void draw(Bengine::DebugRenderer& renderer, const Bengine::FpsLimiter& fpsInfo, const glm::vec2& pos) {
        root.draw(renderer, fpsInfo, pos, selectedNode);
    }
};

// data Direction = Left | Right | Up | Down
// Left keyboard
// Left -> Selects a node to the left of your selection

// < * >
// 
