#pragma once

#include <Bengine/Bengine.h>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <Bengine/DebugRenderer.h>
#include <iostream>
#include "HashPair.h"
#include <unordered_map>
#include <functional>

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
float BezierBlend(float t)
{
    return t * t * (3.0f - 2.0f * t);
}

struct Node {
    static constexpr float separationBetweenNodes = 100.0f; // Distance between each node.

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
    void draw(Bengine::DebugRenderer& renderer, float deltaTime, const glm::vec2& translation, Node* selectedNode) {
        bool forceNoActiveDrawing = false;
        _draw(renderer, deltaTime, translation, forceNoActiveDrawing, selectedNode);
    }
private:
    void _draw(Bengine::DebugRenderer& renderer, float deltaTime, const glm::vec2& translation, bool& forceNoActiveDrawing, Node* selectedNode) {
        // Teal: {0,128,128,255}
        Bengine::ColorRGBA8 color = {0,200,200,100}; // Bright teal
        Bengine::ColorRGBA8 red = {150,30,100,100};
        
        // Recursively draw the other nodes connected to this one:
        glm::vec2 dest;
        Node* destNode;
        if (up) {
            dest = {translation.x, translation.y + separationBetweenNodes};
            renderer.drawLine(translation, dest, color);
            up->_draw(renderer, deltaTime, dest, forceNoActiveDrawing, selectedNode);
        }
        if (down) {
            dest = {translation.x, translation.y - separationBetweenNodes};
            renderer.drawLine(translation, dest, color);
            down->_draw(renderer, deltaTime, dest, forceNoActiveDrawing, selectedNode);
        }
        if (left) {
            dest ={translation.x - separationBetweenNodes, translation.y};
            renderer.drawLine(translation, dest, color);
            left->_draw(renderer, deltaTime, dest, forceNoActiveDrawing, selectedNode);
        }
        if (right) {
            dest = {translation.x + separationBetweenNodes, translation.y};
            renderer.drawLine(translation, dest, color);
            right->_draw(renderer, deltaTime, dest, forceNoActiveDrawing, selectedNode);
        }
        
        // Draw ourselves as the selected node if we have no more `active` nodes to recurse to:
        if (selectedNode == this) {
            // Active node
            float sizeMultiplier = BezierBlend(sin(SDL_GetTicks()/1000.0f));
            renderer.drawCircle(translation, color, 20*sizeMultiplier*deltaTime);
        }
        else {
            // Regular node
            renderer.drawCircle(translation, color, 10);
        }

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
    
public:
    NodeManager() : selectedNode(&root) {}
    
    // Receives any just-pressed keys (not held).
    void receiveKeyPressed(SDL_Keycode key) {
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
        case SDLK_u:
            undo();
            break;
        default:
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
            *from = nullptr;

            // Remove the node from the map
            const glm::ivec2& toNode2DPosition = op.addNode_to->position2D;
            nodeAt2DPoint.erase({toNode2DPosition.x, toNode2DPosition.y});
            
            delete op.addNode_to;
        }
        undoStack.pop_back();
    }

    void draw(Bengine::DebugRenderer& renderer, float deltaTime, const glm::vec2& pos) {
        root.draw(renderer, deltaTime, pos, selectedNode);
    }
};

// data Direction = Left | Right | Up | Down
// Left keyboard
// Left -> Selects a node to the left of your selection

// < * >
// 
