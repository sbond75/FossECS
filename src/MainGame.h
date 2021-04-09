#pragma once

#include <SDL2/SDL.h>
#include <GL/glew.h>

#include <Bengine/Bengine.h>
#include <Bengine/GLSLProgram.h>
#include <Bengine/GLTexture.h>
#include <Bengine/Sprite.h>
#include <Bengine/Window.h>
#include <Bengine/InputManager.h>
#include <Bengine/Timing.h>

#include <Bengine/SpriteBatch.h>

#include <Bengine/Camera2D.h>

#include "Bullet.h"

#include <vector>

#include <Bengine/DebugRenderer.h>
//#include "NodeManager.h"
#include <Bengine/SpriteFont.h>

enum class GameState {PLAY, EXIT};

//Our example game class, just for testing purposes right now
class MainGame
{
public:
    MainGame();
    ~MainGame();

    void run();

private:
    void initSystems();
    void initShaders();
    void gameLoop();
    void processInput();
    void drawGame();

    Bengine::Window _window;
    int _screenWidth;
    int _screenHeight;
    GameState _gameState;

    Bengine::GLSLProgram _colorProgram;
    Bengine::Camera2D _camera;

    Bengine::SpriteBatch _spriteBatch;

    Bengine::InputManager _inputManager;
    Bengine::FpsLimiter _fpsLimiter;

    std::vector<Bullet> _bullets;

    Bengine::DebugRenderer _debugRenderer;
    //NodeManager _nodeManager;
    Bengine::SpriteFont _spriteFont;
    
    float _maxFPS;
    float _fps;
    float _time;
};

