#pragma once

#include <Bengine/Bengine.h>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <Bengine/DebugRenderer.h>
#include <iostream>

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

struct Node {
    static constexpr float separationBetweenNodes = 100.0f; // Distance between each node.

    Direction originatingDirection = NoDirection; Node* originNode = nullptr; // The "origin" node is the node connected to us who spawned us. When about to create nodes in the same direction as this origin node, we should not spawn a node. Instead we go back to that node and make it the "selection" by setting its `active` node to nullptr.
    Node* up = nullptr, *down = nullptr, *left = nullptr, *right = nullptr, *active = nullptr;
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

    void activate(Node* node) {
        active = node;
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

    void move(Direction dir, std::vector<NodeOperation>& undoStack) {
        if (active) {
            active->move(dir, undoStack); // Forward it to the active node.
            return;
        }

        if (dir == NoDirection) return;
        Node** dest = nodeForDirection(dir);
        if (!*dest) {
            if (dir == originatingDirection && originNode) {
                // Then we need to re-use this node instead of spawning one:
                originNode->active = nullptr;
                return;
            }
            *dest = new Node();
            (*dest)->originNode = this;
            (*dest)->originatingDirection = invertDirection(dir);

            // Push to undo stack
            undoStack.emplace_back();
            undoStack.back().type = NodeOperation::Type::AddNode;
            undoStack.back().addNode_direction = dir;
            undoStack.back().addNode_from = this;
            undoStack.back().addNode_to = *dest;
        }
        activate(*dest);
    }

    void draw(Bengine::DebugRenderer& renderer, const glm::vec2& translation) {
        // Teal: {0,128,128,255}
        Bengine::ColorRGBA8 color = {0,200,200,100}; // Bright teal
        renderer.drawCircle(translation, color, 10);
        
        // Recursively draw the other nodes connected to this one:
        glm::vec2 dest;
        Node* destNode;
        if (up) {
            dest = {translation.x, translation.y + separationBetweenNodes};
            renderer.drawLine(translation, dest, color);
            up->draw(renderer, dest);
        }
        if (down) {
            dest = {translation.x, translation.y - separationBetweenNodes};
            renderer.drawLine(translation, dest, color);
            down->draw(renderer, dest);
        }
        if (left) {
            dest ={translation.x - separationBetweenNodes, translation.y};
            renderer.drawLine(translation, dest, color);
            left->draw(renderer, dest);
        }
        if (right) {
            dest = {translation.x + separationBetweenNodes, translation.y};
            renderer.drawLine(translation, dest, color);
            right->draw(renderer, dest);
        }
    }
};

class NodeManager {
    Node root;
    std::vector<NodeOperation> undoStack;
    std::vector<NodeOperation> redoStack;

public:
    // Receives any just-pressed keys (not held).
    void receiveKeyPressed(SDL_Keycode key) {
        switch (key) {
        case SDLK_LEFT:
            root.move(Left, undoStack);
            break;
        case SDLK_RIGHT:
            root.move(Right, undoStack);
            break;
        case SDLK_UP:
            root.move(Up, undoStack);
            break;
        case SDLK_DOWN:
            root.move(Down, undoStack);
            break;
        case SDLK_u:
            undo();
            break;
        default:
            break;
	}
    }
    
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
            *from = nullptr;
            delete op.addNode_to;
        }
        undoStack.pop_back();
    }

    void draw(Bengine::DebugRenderer& renderer, const glm::vec2& pos) {
        root.draw(renderer, pos);
    }
};

// data Direction = Left | Right | Up | Down
// Left keyboard
// Left -> Selects a node to the left of your selection

// < * >
// 
