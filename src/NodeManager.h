#pragma once

#include <Bengine/Bengine.h>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <Bengine/DebugRenderer.h>

enum Direction {
    NoDirection, Left, Right, Up, Down
};

Direction invertDirection(Direction dir) {
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

    void move(Direction dir) {
        if (active) {
            active->move(dir); // Forward it to the active node.
            return;
        }

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
            return;
        }
        if (!*dest) {
            if (dir == originatingDirection && originNode) {
                // Then we need to re-use this node instead of spawning one:
                originNode->active = nullptr;
                return;
            }
            *dest = new Node();
            (*dest)->originNode = this;
            (*dest)->originatingDirection = invertDirection(dir);
        }
        activate(*dest);
    }

    void draw(Bengine::DebugRenderer& renderer, const glm::vec2& translation) {
        // Teal: {0,128,128,255}
        Bengine::ColorRGBA8 color = {0,200,200,100}; // Bright teal
        renderer.drawCircle(translation, color, 10);
        
        // Recursively draw the other nodes connected to this one:
        if (up) up->draw(renderer, {translation.x, translation.y + separationBetweenNodes});
        if (down) down->draw(renderer, {translation.x, translation.y - separationBetweenNodes});
        if (left) left->draw(renderer, {translation.x - separationBetweenNodes, translation.y});
        if (right) right->draw(renderer, {translation.x + separationBetweenNodes, translation.y});
    }
};

class NodeManager {
    Node root;

public:
    // Receives any just-pressed keys (not held).
    void receiveKeyPressed(SDL_Keycode key) {
        switch (key) {
        case SDLK_LEFT:
            root.move(Left);
            break;
        case SDLK_RIGHT:
            root.move(Right);
            break;
        case SDLK_UP:
            root.move(Up);
            break;
        case SDLK_DOWN:
            root.move(Down);
            break;
        default:
            break;
	}
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
